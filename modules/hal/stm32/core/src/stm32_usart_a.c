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

#include "common.h"
#include "debug_ser.h"
#include "hal_int.h"
#include "hal_uart.h"
#include "stm32_hal.h"
#include "xassert.h"
#if BMOS
#include "bmos_op_msg.h"
#include "bmos_msg_queue.h"
#endif

#define MSGDATA_POW2 4
#define MSGDATA_LEN (1 << MSGDATA_POW2)

typedef struct {
  unsigned int sr;
  unsigned int dr;
  unsigned int brr;
  unsigned int cr1;
  unsigned int cr2;
  unsigned int cr3;
  unsigned int gtpr;
} stm32_usart_a_t;

#define UART_SR_ORE BIT(3)
#define UART_SR_IDLE BIT(4)
#define UART_SR_RXNE BIT(5)
#define UART_SR_TC BIT(6)
#define UART_SR_TXE BIT(7)

#define USART_CR1_RE BIT(2)     /* RX enable */
#define USART_CR1_TE BIT(3)     /* TX enable */
#define USART_CR1_IDLEIE BIT(4) /* Idle int enable */
#define USART_CR1_RXNEIE BIT(5) /* RX int enable */
#define USART_CR1_TXEIE BIT(7)  /* TX int enable */
#define USART_CR1_UE BIT(13)    /* usart enable */

#define USART_CR3_HDSEL BIT(3)  /* Single wire half duplex */

static volatile stm32_usart_a_t *duart;

static int usart_tx_done(volatile stm32_usart_a_t *usart)
{
  return usart->sr & UART_SR_TC;
}

static void usart_putc(volatile stm32_usart_a_t *usart, int ch)
{
  while (!(usart->sr & UART_SR_TXE))
    ;

  usart->dr = ch & 0xff;
}

static int usart_xgetc(volatile stm32_usart_a_t *usart)
{
  if (usart->sr & UART_SR_RXNE)
    return (int)(usart->dr & 0xff);

  return -1;
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

void debug_uart_init(void *base, unsigned int baud, unsigned int clock,
                     unsigned int flags)
{
  duart = (volatile stm32_usart_a_t *)base;

  duart->brr = (clock + baud / 2) / baud;
  duart->cr1 = USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE;
}

#if BMOS
static void usart_set_baud(volatile stm32_usart_a_t *usart,
                           unsigned int baud, unsigned int clock,
                           unsigned int flags)
{
  unsigned int divider;

  divider = (clock + (baud / 2)) / baud;

  usart->brr = divider;
}

static void usart_tx(uart_t *u)
{
  volatile stm32_usart_a_t *usart = u->base;
  bmos_op_msg_t *m;
  unsigned int len;
  unsigned char *data;

  m = u->msg;

  if (m) {
    len = u->data_len;
    data = u->data;
  } else {
    m = op_msg_get(u->txq);
    if (!m) {
      usart->cr1 &= ~USART_CR1_TXEIE;
      return;
    }
    len = m->len;
    data = BMOS_OP_MSG_GET_DATA(m);
  }

  usart->dr = *data;

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

static void sendcb(uart_t *u)
{
  unsigned char *msgdata;
  int len;
  bmos_op_msg_t *m;

  m = op_msg_get(u->pool);
  if (!m)
    u->stats.overrun++;
  else {
    msgdata = BMOS_OP_MSG_GET_DATA(m);
    len = circ_buf_read(&u->cb, msgdata, MSGDATA_LEN);
    if (len < MSGDATA_LEN)
      len += circ_buf_read(&u->cb, msgdata + len, MSGDATA_LEN);
    op_msg_put(u->rxq, m, u->op, len);
  }
}

static void usart_isr(void *data)
{
  uart_t *u = (uart_t *)data;
  volatile stm32_usart_a_t *usart = u->base;
  unsigned int isr;

  isr = usart->sr;

  if (isr & UART_SR_ORE)
    u->stats.hw_overrun++;

  if ((usart->cr1 & USART_CR1_TXEIE) && (isr & UART_SR_TXE))
    usart_tx(u);

  if (isr & UART_SR_RXNE) {
    unsigned char c;
    int len;

    c = usart->dr & 0xff;

    len = circ_buf_write(&u->cb, &c, 1);
    if (len == 0) {
      sendcb(u);
      (void)circ_buf_write(&u->cb, &c, 1);
    }
  }

  if (isr & UART_SR_IDLE) {
    (void)usart->dr;
    if (circ_buf_used(&u->cb) > 0)
      sendcb(u);
  }
}

static void _put(void *p)
{
  uart_t *u = (uart_t *)p;
  volatile stm32_usart_a_t *usart = u->base;

  usart->cr1 |= USART_CR1_TXEIE;
}

bmos_queue_t *uart_open(uart_t *u, unsigned int baud, bmos_queue_t *rxq,
                        unsigned int op)
{
  volatile stm32_usart_a_t *usart = u->base;

  usart_set_baud(usart, baud, u->clock, u->flags);

  if (u->flags & STM32_UART_SINGLE_WIRE)
    usart->cr3 |= USART_CR3_HDSEL;

  usart->cr1 = USART_CR1_UE | USART_CR1_RXNEIE | \
               USART_CR1_IDLEIE | USART_CR1_TE | USART_CR1_RE;

  circ_buf_init(&u->cb, MSGDATA_POW2);

  u->pool = op_msg_pool_create("uart pool", QUEUE_TYPE_DRIVER, 4, MSGDATA_LEN);
  XASSERT(u->pool);

  u->txq = queue_create("uart tx", QUEUE_TYPE_DRIVER);
  XASSERT(u->txq);

  (void)queue_set_put_f(u->txq, _put, (void *)u);

  u->rxq = rxq;
  u->op = (unsigned short)op;

  irq_register(u->name, usart_isr, u, u->irq);

  return u->txq;
}
#endif
