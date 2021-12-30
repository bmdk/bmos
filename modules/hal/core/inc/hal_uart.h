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

#ifndef HAL_UART_H
#define HAL_UART_H

#if BMOS
#include "bmos_queue.h"
#include "bmos_op_msg.h"
#endif

#include "circ_buf.h"

typedef struct {
  const char *name;
  void *base;
  unsigned int clock;
  unsigned char irq;
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
  circ_buf_t cb;
} uart_t;

#if BMOS
bmos_queue_t *uart_open(uart_t *u, unsigned int baud, bmos_queue_t *rxq,
                        unsigned int op);

void uart_set_baud(uart_t *u, unsigned int baud);
#endif

extern uart_t debug_uart;

#endif
