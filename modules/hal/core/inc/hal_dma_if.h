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

#ifndef HAL_DMA_IF_H
#define HAL_DMA_IF_H

#include "hal_dma.h"

typedef void dma_trans_t(void *data, unsigned int chan,
                         void *src, void *dst, unsigned int n,
                         dma_attr_t flags);
typedef void dma_set_chan_t(void *data, unsigned int chan, unsigned int devid);
typedef void dma_en_t(void *data, unsigned int chan, int en);
typedef void dma_start_t(void *data, unsigned int chan);
typedef void dma_irq_ack_t(void *data, unsigned int chan);

typedef struct {
  dma_trans_t *trans;
  dma_set_chan_t *set_chan;
  dma_en_t *en;
  dma_start_t *start;
  dma_irq_ack_t *irq_ack;
} dma_controller_t;

typedef struct {
  dma_controller_t *cont;
  void *data;
} dma_cont_data_t;

extern dma_cont_data_t dma_cont_data[];
extern unsigned int dma_cont_data_len;

int dma_controller_reg(unsigned int num, dma_controller_t *cont, void *data);

#endif
