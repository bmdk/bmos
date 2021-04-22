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

#include "common.h"
#include "debug_ser.h"
#include "hal_int.h"
#include "hal_uart.h"
#include "io.h"
#include "xassert.h"
#include "fast_log.h"

#include "hardware/uart.h"

#if BMOS
#include "bmos_op_msg.h"
#include "bmos_msg_queue.h"
#endif

#define UART_NUM(n) uart_num((int)n)

static uart_inst_t *uart_num(unsigned int num)
{
  if (num == 1)
    return uart1;
  return uart0;
}

#if BMOS
static void usart_tx(uart_t *u)
{
  uart_inst_t *uart = UART_NUM(u->base);
  bmos_op_msg_t *m;
  unsigned int len;
  unsigned char *data;

  m = u->msg;

  if (m) {
    len = u->data_len;
    data = u->data;
  } else {
    FAST_LOG('u', "get %s\n", queue_get_name(u->txq), 0);
    m = op_msg_get(u->txq);
    if (!m) {
      uart_set_irq_enables(uart, true, false);
      return;
    }
    len = m->len;
    data = BMOS_OP_MSG_GET_DATA(m);
  }

  uart_putc_raw(uart, *data);

  len--;
  data++;

  u->data++;
  u->data_len--;

  if (len == 0) {
    op_msg_return(m);
    u->msg = 0;
  } else {
    u->msg = m;
    u->data_len = len;
    u->data = data;
  }
}

static void uart_isr(void *data)
{
  uart_t *u = (uart_t *)data;
  uart_inst_t *uart = UART_NUM(u->base);
  int c;
  bmos_op_msg_t *m;
  unsigned char *msgdata;

  FAST_LOG('u', "usart isr\n", 0, 0);

  if (uart_is_writable(uart)) {
    FAST_LOG('u', "usart tx\n", 0, 0);
    usart_tx(u);
  }

  while (uart_is_readable(uart)) {
    FAST_LOG('u', "usart rx\n", 0, 0);
    c = uart_getc(uart);
    m = op_msg_get(u->pool);
    FAST_LOG('u', "get %s %p\n", queue_get_name(u->pool), m);
    if (!m) {
      FAST_LOG('u', "rx overrun\n", 0, 0);
      u->stats.overrun++;
      continue;
    }

    msgdata = BMOS_OP_MSG_GET_DATA(m);

    *msgdata = (unsigned char)c;

    FAST_LOG('u', "put %s %p\n", queue_get_name(u->rxq), m);
    op_msg_put(u->rxq, m, u->op, 1);
  }
}

static void _put(void *p)
{
  uart_t *u = (uart_t *)p;
  uart_inst_t *uart = UART_NUM(u->base);
  unsigned int saved;

  uart_set_irq_enables(uart, true, true);

  saved = interrupt_disable();
  usart_tx(u);
  interrupt_enable(saved);
}

bmos_queue_t *uart_open(uart_t *u, unsigned int baud, bmos_queue_t *rxq,
                        unsigned int op)
{
  uart_inst_t *uart = UART_NUM(u->base);
  const char *pool_name = "upool", *tx_queue_name = "utx";

  uart_init(uart, baud);

  uart_set_irq_enables(uart, true, false);
  uart_set_fifo_enabled(uart, false);

  if (u->pool_name)
    pool_name = u->pool_name;
  u->pool = op_msg_pool_create(pool_name, QUEUE_TYPE_DRIVER, 4, 4);
  XASSERT(u->pool);

  if (u->tx_queue_name)
    tx_queue_name = u->tx_queue_name;
  u->txq = queue_create(tx_queue_name, QUEUE_TYPE_DRIVER);
  XASSERT(u->txq);

  (void)queue_set_put_f(u->txq, _put, (void *)u);

  u->rxq = rxq;
  u->op = (unsigned short)op;

  irq_register(u->name, uart_isr, u, u->irq);

  return u->txq;
}
#endif
