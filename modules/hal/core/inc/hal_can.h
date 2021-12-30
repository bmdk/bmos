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

#ifndef HAL_CAN_H
#define HAL_CAN_H

#if BMOS
#include "bmos_queue.h"
#include "bmos_op_msg.h"
#endif

typedef struct {
  unsigned short prediv;
  unsigned short ts1;
  unsigned short ts2;
  unsigned short sjw;
} can_params_t;

typedef struct {
  const char *name;
  void *base;
  unsigned char irq;
  can_params_t params;
  unsigned char tx_irq;
  unsigned int flags;
  const char *pool_name;
  const char *tx_queue_name;
  struct {
    unsigned int overrun;
    unsigned int hw_overrun;
  } stats;
#if BMOS
  bmos_queue_t *txq;
  bmos_queue_t *rxq;
  bmos_queue_t *pool;
  bmos_op_msg_t *msg;
  unsigned char *data;
  unsigned short data_len;
  unsigned short op;
#endif
} candev_t;

typedef struct {
  unsigned int id;
  unsigned char data[8];
  unsigned char len;
} can_t;

#if BMOS
bmos_queue_t *can_open(candev_t *c, const unsigned int *id,
                       unsigned int id_len,
                       bmos_queue_t *rxq, unsigned int op);
#endif

#endif
