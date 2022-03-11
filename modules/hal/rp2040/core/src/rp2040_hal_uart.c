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

#include "common.h"
#include "debug_ser.h"
#include "hal_int.h"
#include "hal_uart.h"
#include "io.h"
#include "xassert.h"
#include "fast_log.h"

#if BMOS
#include "bmos_op_msg.h"
#include "bmos_msg_queue.h"
#endif

typedef struct {
  reg32_t dr;
  reg32_t rsr;
  reg32_t pad0[4];
  reg32_t fr;
  reg32_t pad1;
  reg32_t ilpr;
  reg32_t ibrd;
  reg32_t fbrd;
  reg32_t lcr_h;
  reg32_t cr;
  reg32_t ifls;
  reg32_t imsc;
  reg32_t ris;
  reg32_t mis;
  reg32_t icr;
  reg32_t dmacr;
} rp2040_uart_t;

#define UART0_BASE 0x40034000
#define UART1_BASE 0x40038000

#define UART_FR_TXFE BIT(7)
#define UART_FR_RXFF BIT(6)
#define UART_FR_TXFF BIT(5)
#define UART_FR_RXFE BIT(4)
#define UART_FR_BUSY BIT(3)
#define UART_FR_DCD BIT(2)
#define UART_FR_DSR BIT(1)
#define UART_FR_CTS BIT(0)

#define UART_IMSC_TXIM BIT(5)
#define UART_IMSC_RXIM BIT(4)

rp2040_uart_t *duart;

void debug_putc(int ch)
{
  while (duart->fr & UART_FR_TXFF)
    ;
  duart->dr = ch & 0xff;
}

int debug_ser_tx_done()
{
  return !(duart->fr & UART_FR_BUSY);
}

static void x_uart_putc(rp2040_uart_t *uart, int ch)
{
  while (uart->fr & UART_FR_TXFF)
    ;

  uart->dr = ch & 0xff;
}

static int x_uart_getc(rp2040_uart_t *uart)
{
  return uart->dr & 0xff;
}

static int x_uart_readable(rp2040_uart_t *uart)
{
  return !(uart->fr & UART_FR_RXFE);
}

static int x_uart_writable(rp2040_uart_t *uart)
{
  return !(uart->fr & UART_FR_TXFF);
}

static void x_uart_enable_rx(rp2040_uart_t *uart, int en)
{
  if (en)
    uart->imsc |= UART_IMSC_RXIM;
  else
    uart->imsc &= ~UART_IMSC_RXIM;
}

static void x_uart_enable_tx(rp2040_uart_t *uart, int en)
{
  if (en)
    uart->imsc |= UART_IMSC_TXIM;
  else
    uart->imsc &= ~UART_IMSC_TXIM;
}

#if BMOS
static void usart_tx(uart_t *u)
{
  bmos_op_msg_t *m;
  unsigned int len;
  unsigned char *data;
  rp2040_uart_t *uart = (rp2040_uart_t *)u->base;

  m = u->msg;

  if (m) {
    len = u->data_len;
    data = u->data;
  } else {
    FAST_LOG('u', "get %s\n", queue_get_name(u->txq), 0);
    m = op_msg_get(u->txq);
    if (!m) {
      x_uart_enable_tx(uart, 0);
      return;
    }
    len = m->len;
    data = BMOS_OP_MSG_GET_DATA(m);
  }

  x_uart_putc(uart, *data);

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
  rp2040_uart_t *uart = (rp2040_uart_t *)u->base;
  int c;
  bmos_op_msg_t *m;
  unsigned char *msgdata;

  FAST_LOG('u', "usart isr\n", 0, 0);

  if (x_uart_writable(uart)) {
    FAST_LOG('u', "usart tx\n", 0, 0);
    usart_tx(u);
  }

  while (x_uart_readable(uart)) {
    FAST_LOG('u', "usart rx\n", 0, 0);
    c = x_uart_getc(uart);
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
  unsigned int saved;
  rp2040_uart_t *uart = (rp2040_uart_t *)u->base;

  saved = interrupt_disable();
  x_uart_enable_tx(uart, 1);

  usart_tx(u);
  interrupt_enable(saved);
}

#define UART_CR_RXE BIT(9)
#define UART_CR_TXE BIT(8)
#define UART_CR_EN BIT(0)

#define UART_LCRH_WLEN(v) (((v) & 0x3) << 5)
#define UART_LCRH_WLEN_8 UART_LCRH_WLEN(3)
#define UART_LCRH_FEN BIT(4)
#define UART_LCRH_STP2 BIT(3)
#define UART_LCRH_EPS BIT(2)
#define UART_LCRH_PEN BIT(1)
#define UART_LCRH_BRK BIT(0)

static void uart_init(rp2040_uart_t *uart, unsigned int baud,
                      unsigned int clock, unsigned int flags)
{
  unsigned int div = (8 * clock / baud);
  unsigned int ibrd = div >> 7;
  unsigned int fbrd = ((div & 0x7f) + 1) / 2;

  if (ibrd == 0) {
    ibrd = 1;
    fbrd = 0;
  } else if (ibrd >= 65535) {
    ibrd = 65535;
    fbrd = 0;
  }

  uart->lcr_h = UART_LCRH_WLEN_8;
  uart->ibrd = ibrd;
  uart->fbrd = fbrd;
  uart->cr = UART_CR_RXE | UART_CR_TXE | UART_CR_EN;
  uart->lcr_h |= 0;
}

void debug_uart_init(void *base, unsigned int baud,
                     unsigned int clock, unsigned int flags)
{
  duart = (rp2040_uart_t *)base;

  uart_init(duart, baud, clock, flags);
}

bmos_queue_t *uart_open(uart_t *u, unsigned int baud, bmos_queue_t *rxq,
                        unsigned int op)
{
  const char *pool_name = "upool", *tx_queue_name = "utx";
  rp2040_uart_t *uart = (rp2040_uart_t *)u->base;

  uart_init(uart, baud, u->clock, 0);
  x_uart_enable_rx(uart, 1);

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
