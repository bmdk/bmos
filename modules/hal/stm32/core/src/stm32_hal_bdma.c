/* Copyright (c) 2019-2022 Brian Thomas Murphy
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

#include "hal_dma_if.h"

#if STM32_G0XX || STM32_G4XX
#include "stm32_hal_dmamux.h"
#endif

#define CCR_CT BIT(16)
#define CCR_DBM BIT(15)
#define CCR_MEM2MEM BIT(14)
#define CCR_PL(_l_) (((_l_) & 0x3) << 12) /* prio */
#define CCR_MSIZ(_s_) (((_s_) & 0x3) << 10)
#define CCR_PSIZ(_s_) (((_s_) & 0x3) << 8)
#define CCR_MINC BIT(7)
#define CCR_PINC BIT(6)
#define CCR_CIRC BIT(5)
#define CCR_DIR BIT(4) /* 0 - from peripheral */
#define CCR_TEIE BIT(3)
#define CCR_HEIE BIT(2)
#define CCR_TCIE BIT(1)
#define CCR_EN BIT(0)

#define IER_TEIF BIT(3)
#define IER_HTIF BIT(2)
#define IER_TCIF BIT(1)
#define IER_GIF BIT(0)

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

static void stm32_bdma_set_chan(void *base, unsigned int chan,
                                unsigned int devid)
{
#if STM32_G0XX || STM32_G4XX
  if (base == (void *)0x40020400)
    chan += 8;

  stm32_dmamux_req(chan, devid);
#elif !STM32_F1XX
  volatile stm32_bdma_t *d = base;

  reg_set_field(&d->cselr, 4, chan << 2, devid);
#endif
}

static void stm32_bdma_en(void *base, unsigned int chan, int en)
{
  volatile stm32_bdma_t *d = base;
  volatile stm32_bdma_chan_t *c = &d->chan[chan];

  if (en)
    c->ccr |= CCR_EN;
  else
    c->ccr &= ~CCR_EN;
}

static void stm32_bdma_start(void *base, unsigned int chan)
{
  volatile stm32_bdma_t *d = base;
  volatile stm32_bdma_chan_t *c = &d->chan[chan];

  c->ccr &= ~(CCR_MEM2MEM | CCR_EN);
  c->ccr |= (CCR_MEM2MEM | CCR_EN);
}

static void stm32_bdma_irq_ack(void *base, unsigned int chan)
{
  volatile stm32_bdma_t *d = base;

  reg_set_field(&d->ifcr, 4, chan << 2, IER_TCIF);
}

static void stm32_bdma_trans(void *base, unsigned int chan,
                             void *src, void *dst, unsigned int n,
                             dma_attr_t attr)
{
  volatile stm32_bdma_t *d = base;
  volatile stm32_bdma_chan_t *c = &d->chan[chan];
  unsigned int flags;

  c->ccr &= ~CCR_EN;
  c->cndtr = n;

  flags = CCR_PL(attr.prio);

  if (attr.dir == DMA_DIR_TO) {
    c->cpar = (unsigned int)dst;
    c->cmar[0] = (unsigned int)src;

    flags |= CCR_MSIZ(attr.ssiz) | CCR_PSIZ(attr.dsiz);;

    if (attr.sinc)
      flags |= CCR_MINC;

    if (attr.dinc)
      flags |= CCR_PINC;

    flags |= CCR_DIR;
  } else {
    c->cpar = (unsigned int)src;
    c->cmar[0] = (unsigned int)dst;

    flags |= CCR_MSIZ(attr.dsiz) | CCR_PSIZ(attr.ssiz);;

    if (attr.sinc)
      flags |= CCR_PINC;

    if (attr.dinc)
      flags |= CCR_MINC;
  }

  if (attr.irq)
    flags |= CCR_TCIE;

  c->ccr = flags;
}

#if 0
void stm32_bdma_chan_dump(void *base, unsigned int chan)
{
  volatile stm32_bdma_t *d = base;
  volatile stm32_bdma_chan_t *c = &d->chan[chan];

  xprintf("S: %08x D: %08x C: %d\n", c->cpar, c->cmar[0], c->cndtr);
}
#endif

dma_controller_t stm32_bdma_controller = {
  stm32_bdma_trans,
  stm32_bdma_set_chan,
  stm32_bdma_en,
  stm32_bdma_start,
  stm32_bdma_irq_ack
};
