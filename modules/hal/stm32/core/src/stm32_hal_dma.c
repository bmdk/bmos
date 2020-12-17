/* Copyright (c) 2019 Brian Thomas Murphy
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

#include "hal_common.h"
#include "shell.h"
#include "io.h"

#include "stm32_hal_dma.h"

typedef struct {
  unsigned int cr;
  unsigned int ndtr;
  unsigned int par;
  unsigned int mar[2];
  unsigned int fcr;
} stm32_dma_chan_t;

typedef struct {
  unsigned int isr[2];
  unsigned int ifcr[2];
  stm32_dma_chan_t chan[DMA_CHANNELS];
} stm32_dma_t;

#ifdef STM32_H7XX
static volatile stm32_dma_t *dma[] = { (void *)0x40020000,
                                       (void *)0x40020400 };
#else
#error Define dmamux for this platform
#endif

static volatile stm32_dma_t *num2addr(unsigned int num)
{
  if (num >= ARRSIZ(dma))
    num = 0;
  return dma[num];
}

void stm32_dma_set_chan(unsigned int num, unsigned int chan, unsigned int devid)
{
  volatile stm32_dma_t *d = num2addr(num);
  volatile stm32_dma_chan_t *c = &d->chan[chan];

  reg_set_field(&c->cr, 3, 25, devid);
}

void stm32_dma_en(unsigned int num, unsigned int chan, int en)
{
  volatile stm32_dma_t *d = num2addr(num);
  volatile stm32_dma_chan_t *c = &d->chan[chan];

  if (en)
    c->cr |= DMA_CR_EN;
  else
    c->cr &= ~DMA_CR_EN;
}

void stm32_dma_sw_trig(unsigned int num, unsigned int chan)
{
  volatile stm32_dma_t *d = num2addr(num);
  volatile stm32_dma_chan_t *c = &d->chan[chan];

  c->cr &= ~(DMA_CR_DIR_M2M | DMA_CR_EN);
  c->cr |= (DMA_CR_DIR_M2M | DMA_CR_EN);
}

void stm32_dma_trans(unsigned int num, unsigned int chan,
                     void *src, void *dst, unsigned int n,
                     unsigned int flags)
{
  volatile stm32_dma_t *d = num2addr(num);
  volatile stm32_dma_chan_t *c = &d->chan[chan];

  c->cr &= ~DMA_CR_EN;

  c->ndtr = n;
  c->par = (unsigned int)src;
  c->mar[0] = (unsigned int)dst;

  c->cr = flags;
}

void stm32_dma_memcpy(unsigned int num, void *src, void *dst, unsigned int n)
{
  stm32_dma_trans(num, 0, src, dst, n,
                  DMA_CR_PL(0) | DMA_CR_MSIZ(0) | DMA_CR_PSIZ(0) | \
                  DMA_CR_MINC | DMA_CR_PINC);
  stm32_dma_sw_trig(num, 0);
}

void stm32_dma_chan_dump(unsigned int num)
{
  volatile stm32_dma_t *d = num2addr(num);
  unsigned int i;

  for (i = 0; i < DMA_CHANNELS; i++) {
    volatile stm32_dma_chan_t *c = &d->chan[i];
    xprintf("%d P: %08x M: %08x, %08x N: %d\n", i,
            c->par, c->mar[0], c->mar[1], c->ndtr);
  }
}

int cmd_dma(int argc, char *argv[])
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

    stm32_dma_set_chan(0, chan, devid);
    break;
  case 'm':
    if (argc < 5)
      return -1;
    src = strtoul(argv[2], 0, 16);
    dst = strtoul(argv[3], 0, 16);
    n = strtoul(argv[4], 0, 0);

    xprintf("copy src: %08x dst: %08x cnt: %d\n", src, dst, n);
    stm32_dma_memcpy(0, (void *)src, (void *)dst, n);
    break;
  case 't':
    if (argc < 3)
      return -1;
    stm32_dma_sw_trig(0, atoi(argv[2]));
    break;
  case 'd':
    stm32_dma_chan_dump(0);
    break;
  }

  return 0;
}

SHELL_CMD(dma, cmd_dma);
