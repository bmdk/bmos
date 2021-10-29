/* Copyright (c) 2019-2021 Brian Thomas Murphy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>

#include "hal_dma_if.h"
#include "hal_common.h"
#include "shell.h"
#include "io.h"

#define DMA_CHANNELS 16

#define GPDMA_TCF BIT(8)
#define GPDMA_HTF BIT(9)
#define GPDMA_DTEF BIT(10)
#define GPDMA_ULEF BIT(11)
#define GPDMA_USEF BIT(12)
#define GPDMA_SUSPF BIT(13)
#define GPDMA_TOF BIT(14)

typedef struct {
  reg32_t lbar;
  unsigned int pad0[2];
  reg32_t fcr;
  reg32_t sr;
  reg32_t cr;
  unsigned int pad1[10];
  reg32_t tr1;
  reg32_t tr2;
  reg32_t br1;
  reg32_t sar;
  reg32_t dar;
  reg32_t tr3;
  reg32_t br2;
  unsigned int pad2[8];
  reg32_t llr;
} stm32_gpdma_chan_t;

typedef struct {
  reg32_t seccfgr;
  reg32_t privcfgr;
  reg32_t rcfglockr;
  reg32_t misr;
  reg32_t smisr;
  unsigned int pad0[15];
  stm32_gpdma_chan_t chan[16];
} stm32_gpdma_t;

#define GPDMA_CR_EN BIT(0)
#define GPDMA_CR_RESET BIT(1)
#define GPDMA_CR_SUSP BIT(2)
#define GPDMA_CR_TCIE BIT(8)
#define GPDMA_CR_HTIE BIT(9)
#define GPDMA_CR_DTEIE BIT(10)
#define GPDMA_CR_ULEIE BIT(11)
#define GPDMA_CR_USEIE BIT(12)
#define GPDMA_CR_SUSPIE BIT(13)
#define GPDMA_CR_TOIE BIT(14)
#define GPDMA_CR_LSM BIT(16)
#define GPDMA_CR_LAP BIT(17)
#define GPDMA_CR_PRIO(_n_) (((_n_) & 0x3) << 22)

#define GPDMA_TR1_SDW(_n_) (((_n_) & 0x3) << 0)
#define GPDMA_TR1_SINC BIT(3)
#define GPDMA_TR1_SBL(_n_) (((_n_) & 0x1f) << 4)
#define GPDMA_TR1_PAM(_n_) (((_n_) & 0x3) << 11)
#define GPDMA_TR1_SBX BIT(13)
#define GPDMA_TR1_SAP BIT(14)
#define GPDMA_TR1_SSEC BIT(15)
#define GPDMA_TR1_DDW(_n_) (((_n_) & 0x3) << 16)
#define GPDMA_TR1_DINC BIT(19)
#define GPDMA_TR1_DBL(_n_) (((_n_) & 0x1f) << 20)
#define GPDMA_TR1_DBX BIT(26)
#define GPDMA_TR1_DHX BIT(27)
#define GPDMA_TR1_DAP BIT(30)
#define GPDMA_TR1_DSEC BIT(31)

#define GPDMA_TR2_SWREQ BIT(9)

void stm32_gpdma_set_chan(void *addr, unsigned int chan,
                          unsigned int devid)
{
  stm32_gpdma_t *d = addr;
  stm32_gpdma_chan_t *c = &d->chan[chan];

  reg_set_field(&c->tr2, 7, 0, devid);
}

void stm32_gpdma_irq_ack(void *addr, unsigned int chan)
{
  stm32_gpdma_t *d = addr;
  stm32_gpdma_chan_t *c = &d->chan[chan];

  c->fcr = GPDMA_TCF;
}

void stm32_gpdma_en(void *addr, unsigned int chan, int en)
{
  stm32_gpdma_t *d = addr;
  stm32_gpdma_chan_t *c = &d->chan[chan];

  if (en)
    c->cr |= GPDMA_CR_EN;
  else
    c->cr &= ~GPDMA_CR_EN;
}

void stm32_gpdma_start(void *addr, unsigned int chan)
{
  stm32_gpdma_t *d = addr;
  stm32_gpdma_chan_t *c = &d->chan[chan];

  stm32_gpdma_en(addr, chan, 0);
  c->tr2 |= GPDMA_TR2_SWREQ;
  stm32_gpdma_en(addr, chan, 1);
}

void stm32_gpdma_trans(void *addr, unsigned int chan,
                       void *src, void *dst, unsigned int n,
                       dma_attr_t attr)
{
  stm32_gpdma_t *d = addr;
  stm32_gpdma_chan_t *c = &d->chan[chan];
  unsigned int flags;

  if (chan > 15)
    return;

  c->cr &= ~GPDMA_CR_EN;

  c->sar = (unsigned int)src;
  c->dar = (unsigned int)dst;
  c->br1 = n;

  flags = GPDMA_TR1_DDW(attr.dsiz) | GPDMA_TR1_SDW(attr.ssiz);

  if (attr.sinc)
    flags |= GPDMA_TR1_SINC;

  if (attr.dinc)
    flags |= GPDMA_TR1_DINC;

  if (attr.irq)
    flags |= GPDMA_TCF;

  c->tr1 = flags;
}

#if 0
void stm32_gpdma_memcpy(void *addr, void *src, void *dst, unsigned int n)
{
  stm32_gpdma_trans(num, 0, src, dst, n,
                    GPDMA_TR1_DDW(0) | GPDMA_TR1_SDW(0) | \
                    GPDMA_TR1_SINC | GPDMA_TR1_DINC);
  stm32_gpdma_sw_trig(num, 0);
}

void stm32_gpdma_chan_dump(void *addr)
{
  stm32_gpdma_t *d = addr;
  unsigned int i;

  for (i = 0; i < DMA_CHANNELS; i++) {
    stm32_gpdma_chan_t *c = &d->chan[i];
    xprintf("%d ST: %08x C: %08x S: %08x D: %08x N: %d\n", i,
            c->sr, c->cr, c->sar, c->dar, c->tr1 & 0xffff);
  }
}

#define DMANUM 0

int cmd_gpdma(int argc, char *argv[])
{
  unsigned int chan, devid;
  unsigned int src, dst, n;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'c':
    if (argc < 4)
      return -1;
    chan = atoi(argv[2]);
    devid = atoi(argv[3]);

    stm32_gpdma_set_chan(DMANUM, chan, devid);
    break;
  case 'm':
    if (argc < 5)
      return -1;
    src = strtoul(argv[2], 0, 16);
    dst = strtoul(argv[3], 0, 16);
    n = strtoul(argv[4], 0, 0);

    xprintf("copy src: %08x dst: %08x cnt: %d\n", src, dst, n);
    stm32_gpdma_memcpy(DMANUM, (void *)src, (void *)dst, n);
    break;
  case 'p':
    if (argc < 5)
      return -1;
    src = strtoul(argv[2], 0, 16);
    dst = strtoul(argv[3], 0, 16);
    n = strtoul(argv[4], 0, 0);
    stm32_gpdma_trans(DMANUM, 0, (void *)src, (void *)dst, n,
                      GPDMA_TR1_DDW(0) | GPDMA_TR1_SDW(0));
    break;
  case 't':
    if (argc < 3)
      return -1;
    stm32_gpdma_sw_trig(DMANUM, atoi(argv[2]));
    break;
  case 'e':
    if (argc < 3)
      return -1;
    stm32_gpdma_en(DMANUM, atoi(argv[2]), 1);
    break;
  case 's':
    if (argc < 3)
      return -1;
    stm32_gpdma_en(DMANUM, atoi(argv[2]), 0);
    break;
  case 'd':
    stm32_gpdma_chan_dump(DMANUM);
    break;
  }

  return 0;
}

SHELL_CMD(gpdma, cmd_gpdma);
#endif


dma_controller_t stm32_gpdma_controller = {
  stm32_gpdma_trans,
  stm32_gpdma_set_chan,
  stm32_gpdma_en,
  stm32_gpdma_start,
  stm32_gpdma_irq_ack
};
