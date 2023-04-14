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

#ifndef HAL_DMA_H
#define HAL_DMA_H

#include "common.h"

typedef struct {
  unsigned int ssiz : 2;
  unsigned int dsiz : 2;
  unsigned int dir : 1;
  unsigned int prio : 4;
  unsigned int sinc : 1;
  unsigned int dinc : 1;
  unsigned int irq : 1;
  unsigned int circ : 1;
  unsigned int irq_half : 1;
} dma_attr_t;

#define DMA_SIZ_1 0
#define DMA_SIZ_2 1
#define DMA_SIZ_4 2

#define DMA_DIR_TO 0
#define DMA_DIR_FROM 1

void dma_trans(unsigned int num, unsigned int chan,
               void *src, void *dst, unsigned int n, dma_attr_t flags);
void dma_set_chan(unsigned int num, unsigned int chan, unsigned int devid);
void dma_en(unsigned int num, unsigned int chan, int en);
void dma_start(unsigned int num, unsigned int chan);
/* returns status flags */
#define DMA_IRQ_STATUS_FULL BIT(0)
#define DMA_IRQ_STATUS_HALF BIT(1)
unsigned int dma_irq_ack(unsigned int num, unsigned int chan);

#endif
