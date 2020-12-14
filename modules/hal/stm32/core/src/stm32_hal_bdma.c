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

#include "stm32_hal_bdma.h"

typedef struct {
  unsigned int ccr;
  unsigned int cndtr;
  unsigned int cpar;
  unsigned int cmar[2];
} stm32_bdma_chan_t;

typedef struct {
  unsigned int isr;
  unsigned int ifcr;
  stm32_bdma_chan_t chan[7];
  unsigned int pad0[5];
  unsigned int cselr;
} stm32_bdma_t;

void stm32_bdma_set_chan(void *base, unsigned int chan, unsigned int devid)
{
  volatile stm32_bdma_t *d = base;

  reg_set_field(&d->cselr, 4, chan << 2, devid);
}

void stm32_bdma_en(void *base, unsigned int chan, int en)
{
  volatile stm32_bdma_t *d = base;
  volatile stm32_bdma_chan_t *c = &d->chan[chan];

  if (en)
    c->ccr |= CCR_EN;
  else
    c->ccr &= ~CCR_EN;
}

void stm32_bdma_sw_trig(void *base, unsigned int chan)
{
  volatile stm32_bdma_t *d = base;
  volatile stm32_bdma_chan_t *c = &d->chan[chan];

  c->ccr &= ~(CCR_MEM2MEM | CCR_EN);
  c->ccr |= (CCR_MEM2MEM | CCR_EN);
}

void stm32_bdma_irq_ack(void *base, unsigned int chan, unsigned int flags)
{
  volatile stm32_bdma_t *d = base;

  reg_set_field(&d->ifcr, 4, chan << 2, flags);
}

void stm32_bdma_trans(void *base, unsigned int chan,
                      void *src, void *dst, unsigned int n,
                      unsigned int flags)
{
  volatile stm32_bdma_t *d = base;
  volatile stm32_bdma_chan_t *c = &d->chan[chan];

  c->ccr &= ~CCR_EN;

  c->cndtr = n;
  c->cpar = (unsigned int)src;
  c->cmar[0] = (unsigned int)dst;

  c->ccr = flags;
}

void stm32_bdma_memcpy(void *base, void *src, void *dst, unsigned int n)
{
  stm32_bdma_trans(base, 0, src, dst, n,
                   CCR_PL(0) | CCR_MSIZ(0) | CCR_PSIZ(0) | CCR_MINC | CCR_PINC);
  stm32_bdma_sw_trig(base, 0);
}

void stm32_bdma_chan_dump(void *base, unsigned int chan)
{
  volatile stm32_bdma_t *d = base;
  volatile stm32_bdma_chan_t *c = &d->chan[chan];

  xprintf("S: %08x D: %08x C: %d\n", c->cpar, c->cmar[0], c->cndtr);
}

#define GPIO_ADDR_SET_CLEAR(port) (0x48000000 + 0x400 * (port) + 0x18)
#define GPIO_ADDR GPIO_ADDR_SET_CLEAR(0)

int cmd_bdma(int argc, char *argv[])
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

    stm32_bdma_set_chan(BDMA1_BASE, chan, devid);
    break;
  case 'm':
    if (argc < 5)
      return -1;
    src = strtoul(argv[2], 0, 16);
    dst = strtoul(argv[3], 0, 16);
    n = strtoul(argv[4], 0, 0);

    xprintf("copy src: %08x dst: %08x cnt: %d\n", src, dst, n);
    stm32_bdma_memcpy(BDMA1_BASE, (void *)src, (void *)dst, n);
    break;
  case 't':
    if (argc < 3)
      return -1;
    stm32_bdma_sw_trig(BDMA1_BASE, atoi(argv[2]));
    break;
  case 'd':
    stm32_bdma_chan_dump(BDMA1_BASE, 1);
    stm32_bdma_chan_dump(BDMA1_BASE, 2);
    stm32_bdma_chan_dump(BDMA1_BASE, 5);
    break;
  }

  return 0;
}

SHELL_CMD(bdma, cmd_bdma);
