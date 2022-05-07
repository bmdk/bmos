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

#define DMA_CHANNELS 8

#define DMA_CR_CHSEL(_l_) (((_l_) & 0x7) << 25)
#define DMA_CR_MBURST(_l_) (((_l_) & 0x3) << 23)
#define DMA_CR_MBURST_SINGLE CR_MBURST(0)
#define DMA_CR_MBURST_INCR4 CR_MBURST(1)
#define DMA_CR_MBURST_INCR8 CR_MBURST(2)
#define DMA_CR_MBURST_INCR16 CR_MBURST(3)
#define DMA_CR_PBURST(_l_) (((_l_) & 0x3) << 21)
#define DMA_CR_PBURST_SINGLE CR_PBURST(0)
#define DMA_CR_PBURST_INCR4 CR_PBURST(1)
#define DMA_CR_PBURST_INCR8 CR_PBURST(2)
#define DMA_CR_PBURST_INCR16 CR_PBURST(3)
#define DMA_CR_TRBUFF BIT(20)
#define DMA_CR_CT BIT(19)
#define DMA_CR_DBM BIT(18)
#define DMA_CR_PL(_l_) (((_l_) & 0x3) << 16) /* prio */
#define DMA_CR_PINCOS BIT(15)
#define DMA_CR_MSIZ(_s_) (((_s_) & 0x3) << 13)
#define DMA_CR_PSIZ(_s_) (((_s_) & 0x3) << 11)
#define DMA_CR_MINC BIT(10)
#define DMA_CR_PINC BIT(9)
#define DMA_CR_CIRC BIT(8)
#define DMA_CR_DIR(_s_) (((_s_) & 0x3) << 6)
#define DMA_CR_DIR_P2M DMA_CR_DIR(0)
#define DMA_CR_DIR_M2P DMA_CR_DIR(1)
#define DMA_CR_DIR_M2M DMA_CR_DIR(2)
#define DMA_CR_PFCTRL BIT(5)
#define DMA_CR_TCIE BIT(4)
#define DMA_CR_HTIE BIT(3)
#define DMA_CR_TEIE BIT(2)
#define DMA_CR_DMEIE BIT(1)
#define DMA_CR_EN BIT(0)

#define DMA_FCR_DMDIS BIT(2)

#define DMA_IER_TCIF BIT(5)
#define DMA_IER_HTIF BIT(4)
#define DMA_IER_TEIF BIT(3)
#define DMA_IER_DMEI BIT(2)
#define DMA_IER_FEIF BIT(0)

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

static void stm32_dma_set_chan(void *addr, unsigned int chan,
                               unsigned int devid)
{
  volatile stm32_dma_t *d = addr;
  volatile stm32_dma_chan_t *c = &d->chan[chan];

  reg_set_field(&c->cr, 3, 25, devid);
}

static void stm32_dma_irq_ack(void *addr, unsigned int chan)
{
  volatile stm32_dma_t *d = addr;
  unsigned int i = 0, ofs = 0;

  if (chan >= 4) {
    i = 1;
    chan -= 4;
  }

  if (chan >= 2) {
    ofs = 16;
    chan -= 2;
  }

  reg_set_field(&d->ifcr[i], 6, ofs + 6 * chan, DMA_IER_TCIF);
}

static void stm32_dma_en(void *addr, unsigned int chan, int en)
{
  volatile stm32_dma_t *d = addr;
  volatile stm32_dma_chan_t *c = &d->chan[chan];

  if (en)
    c->cr |= DMA_CR_EN;
  else
    c->cr &= ~DMA_CR_EN;
}

static void stm32_dma_start(void *addr, unsigned int chan)
{
  volatile stm32_dma_t *d = addr;
  volatile stm32_dma_chan_t *c = &d->chan[chan];

  c->cr &= ~(DMA_CR_DIR_M2M | DMA_CR_EN);
  c->fcr = DMA_FCR_DMDIS;
  c->cr |= (DMA_CR_DIR_M2M | DMA_CR_EN);
}

static void stm32_dma_trans(void *addr, unsigned int chan,
                            void *src, void *dst, unsigned int n,
                            dma_attr_t attr)
{
  volatile stm32_dma_t *d = addr;
  volatile stm32_dma_chan_t *c = &d->chan[chan];
  unsigned int ifcr_reg = 0, ofs = 0, flags;

  if (chan > 8)
    return;

  c->cr &= ~DMA_CR_EN;

  if (chan >= 4) {
    ifcr_reg = 1;
    chan -= 4;
  }

  if (chan >= 2) {
    ofs = 16;
    chan -= 2;
  }

  ofs += chan * 6;

  d->ifcr[ifcr_reg] = (BIT(6) - 1) << ofs;

  flags = DMA_CR_PL(attr.prio) | DMA_CR_MSIZ(attr.ssiz) | \
          DMA_CR_PSIZ(attr.dsiz);

  if (attr.dir == DMA_DIR_TO) {
    c->par = (unsigned int)dst;
    c->mar[0] = (unsigned int)src;
    flags |= DMA_CR_DIR_M2P;

    if (attr.sinc)
      flags |= DMA_CR_MINC;

    if (attr.dinc)
      flags |= DMA_CR_PINC;
  } else {
    c->par = (unsigned int)src;
    c->mar[0] = (unsigned int)dst;
    flags |= DMA_CR_DIR_P2M;

    if (attr.sinc)
      flags |= DMA_CR_PINC;

    if (attr.dinc)
      flags |= DMA_CR_MINC;
  }

  if (attr.irq)
    flags |= DMA_CR_TCIE;

  c->fcr &= ~DMA_FCR_DMDIS; /* disable fifo */
  c->ndtr = n;

  reg_set_field(&c->cr, 25, 0, flags);
}

#if 0
void stm32_dma_chan_dump(void *addr)
{
  volatile stm32_dma_t *d = addr;
  unsigned int i;

  for (i = 0; i < DMA_CHANNELS; i++) {
    volatile stm32_dma_chan_t *c = &d->chan[i];
    xprintf("%d F: %08x P: %08x M: %08x, %08x N: %d\n", i,
            c->cr, c->par, c->mar[0], c->mar[1], c->ndtr);
  }
}
#endif

dma_controller_t stm32_dma_controller = {
  stm32_dma_trans,
  stm32_dma_set_chan,
  stm32_dma_en,
  stm32_dma_start,
  stm32_dma_irq_ack
};
