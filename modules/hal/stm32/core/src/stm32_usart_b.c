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

#include <stdlib.h>

#include "common.h"
#include "debug_ser.h"
#include "hal_int.h"
#include "hal_uart.h"
#include "io.h"
#include "stm32_hal.h"
#include "stm32_regs.h"
#include "xassert.h"
#include "fast_log.h"
#if BMOS
#include "bmos_op_msg.h"
#include "bmos_msg_queue.h"
#endif

typedef struct {
  unsigned int cr1;
  unsigned int cr2;
  unsigned int cr3;
  unsigned int brr;
  unsigned int gtpr;
  unsigned int rtor;
  unsigned int rqr;
  unsigned int isr;
  unsigned int icr;
  unsigned int rdr;
  unsigned int tdr;
  unsigned int presc;
} stm32_usart_b_t;

#define UART_ISR_RXNE BIT(5)
#define UART_ISR_TXE BIT(7)
#define UART_ISR_TC BIT(6)
#define UART_ISR_ORE BIT(3)

#define USART_CR1_FIFOEN BIT(29)
#define USART_CR1_TXEIE BIT(7)
#define USART_CR1_RXNEIE BIT(5)
#define USART_CR1_TE BIT(3)
#define USART_CR1_RE BIT(2)
#define USART_CR1_UE BIT(0)

#define CR1_DEFAULT (USART_CR1_RXNEIE | USART_CR1_TE | \
                     USART_CR1_RE | USART_CR1_UE)

volatile stm32_usart_b_t *duart;

static int usart_tx_done(volatile stm32_usart_b_t *usart)
{
  return usart->isr & UART_ISR_TC;
}

static void usart_putc(volatile stm32_usart_b_t *usart, int ch)
{
  while (!(usart->isr & UART_ISR_TXE))
    ;

  usart->tdr = ch & 0xff;
}

static int usart_xgetc(volatile stm32_usart_b_t *usart)
{
  unsigned int isr = usart->isr;

  if (isr & UART_ISR_ORE)
    usart->icr |= UART_ISR_ORE;

  if (isr & UART_ISR_RXNE)
    return (int)(usart->rdr & 0xff);

  return -1;
}

static void usart_set_baud(volatile stm32_usart_b_t *usart,
                           unsigned int baud, unsigned int clock,
                           unsigned int flags)
{
  unsigned int divider;

  divider = (clock + (baud / 2)) / baud;

  if (flags & 1)
    divider <<= 8;

  usart->brr = divider;
}

void debug_uart_init(void *base, unsigned int baud,
                     unsigned int clock, unsigned int flags)
{
  volatile stm32_usart_b_t *usart = (volatile stm32_usart_b_t *)base;

  duart = usart;

  usart_set_baud(usart, baud, clock, flags);

  duart->cr1 = CR1_DEFAULT;
}

void debug_putc(int ch)
{
  usart_putc(duart, ch);
}

int debug_getc(void)
{
  return usart_xgetc(duart);
}

int debug_ser_tx_done(void)
{
  return usart_tx_done(duart);
}

#if BMOS
static void usart_tx(uart_t *u)
{
  volatile stm32_usart_b_t *usart = u->base;
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
      usart->cr1 &= ~USART_CR1_TXEIE;
      return;
    }
    len = m->len;
    data = BMOS_OP_MSG_GET_DATA(m);
  }

  usart->tdr = *data;

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

static void usart_isr(void *data)
{
  uart_t *u = (uart_t *)data;
  volatile stm32_usart_b_t *usart = u->base;
  int c;
  bmos_op_msg_t *m;
  unsigned char *msgdata;
  unsigned int isr;

  FAST_LOG('u', "usart isr\n", 0, 0);

  isr = usart->isr;

  if (isr & UART_ISR_ORE) {
    usart->icr |= UART_ISR_ORE;
    u->stats.hw_overrun++;
  }

  if ((usart->cr1 & USART_CR1_TXEIE) && (isr & UART_ISR_TXE)) {
    FAST_LOG('u', "usart tx\n", 0, 0);
    usart_tx(u);
  }

  if (isr & UART_ISR_RXNE) {
    FAST_LOG('u', "usart rx\n", 0, 0);
    for (;;) {
      c = usart_xgetc(usart);
      if (c < 0)
        break;

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
}

static void _put(void *p)
{
  uart_t *u = (uart_t *)p;
  volatile stm32_usart_b_t *usart = u->base;

  usart->cr1 |= USART_CR1_TXEIE;
}

bmos_queue_t *uart_open(uart_t *u, unsigned int baud, bmos_queue_t *rxq,
                        unsigned int op)
{
  volatile stm32_usart_b_t *usart = u->base;
  const char *pool_name = "upool", *tx_queue_name = "utx";

  usart_set_baud(usart, baud, u->clock, u->flags);

  usart->cr1 = CR1_DEFAULT;

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

  irq_register(u->name, usart_isr, u, u->irq);

  return u->txq;
}
#endif
