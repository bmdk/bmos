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
#include <string.h>

#include "circ_buf.h"
#include "common.h"
#include "debug_ser.h"
#include "fast_log.h"
#include "hal_common.h"
#include "hal_int.h"
#include "hal_uart.h"
#include "io.h"
#include "stm32_hal.h"
#include "xassert.h"
#if BMOS
#include "bmos_op_msg.h"
#include "bmos_msg_queue.h"
#endif

typedef struct {
  reg32_t cr1;
  reg32_t cr2;
  reg32_t cr3;
  reg32_t brr;
  reg32_t gtpr;
  reg32_t rtor;
  reg32_t rqr;
  reg32_t isr;
  reg32_t icr;
  reg32_t rdr;
  reg32_t tdr;
  reg32_t presc;
} stm32_usart_b_t;

#define UART_ISR_TXFT BIT(27)
#define UART_ISR_RXFT BIT(26)
#define UART_ISR_RXFF BIT(24)
#define UART_ISR_TXFE BIT(23)
#define UART_ISR_RTOF BIT(11)
#define UART_ISR_TXFNF BIT(7)
#define UART_ISR_TXE BIT(7)
#define UART_ISR_TC BIT(6)
#define UART_ISR_RXNE BIT(5)
#define UART_ISR_IDLE BIT(4)
#define UART_ISR_ORE BIT(3)

#define USART_CR1_RXFFIE BIT(31)
#define USART_CR1_TXFEIE BIT(30)
#define USART_CR1_FIFOEN BIT(29)
#define USART_CR1_RTOIE BIT(26)
#define USART_CR1_TXEIE BIT(7)
#define USART_CR1_RXNEIE BIT(5)
#define USART_CR1_IDLEIE BIT(4)
#define USART_CR1_TE BIT(3)
#define USART_CR1_RE BIT(2)
#define USART_CR1_UE BIT(0)

#define USART_CR2_RTOEN BIT(23)

#define USART_CR3_RXFTIE BIT(28)
#define USART_CR3_RXFTCFG_OFS 25

#define USART_FIFO_THRES_1_8 0
#define USART_FIFO_THRES_1_4 1
#define USART_FIFO_THRES_1_2 2
#define USART_FIFO_THRES_3_4 3
#define USART_FIFO_THRES_7_8 4
#define USART_FIFO_THRES_FUL 5

#define CR1_DEFAULT (USART_CR1_TE | USART_CR1_RE | USART_CR1_UE)

#define MSGDATA_POW2 4
#define MSGDATA_LEN (1 << MSGDATA_POW2)

static stm32_usart_b_t *duart;

static int usart_tx_done(stm32_usart_b_t *usart)
{
  return usart->isr & UART_ISR_TC;
}

static void usart_putc(stm32_usart_b_t *usart, int ch)
{
  while (!(usart->isr & UART_ISR_TXE))
    ;

  usart->tdr = ch & 0xff;
}

static int usart_xgetc(stm32_usart_b_t *usart)
{
  unsigned int isr = usart->isr;

  if (isr & UART_ISR_ORE) {
    usart->icr |= UART_ISR_ORE;
    FAST_LOG('u', "rx overrun hw\n", 0, 0);
  }

  if (isr & UART_ISR_RXNE)
    return (int)(usart->rdr & 0xff);

  return -1;
}

static void usart_set_baud(stm32_usart_b_t *usart,
                           unsigned int baud, unsigned int clock,
                           unsigned int flags)
{
  unsigned int divider;

  divider = (clock + (baud / 2)) / baud;

  if (flags & STM32_UART_LP)
    divider <<= 8;

  usart->brr = divider;
}

void debug_uart_init(void *base, unsigned int baud,
                     unsigned int clock, unsigned int flags)
{
  stm32_usart_b_t *usart = (stm32_usart_b_t *)base;

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
static void usart_tx(uart_t *u, unsigned int isr)
{
  stm32_usart_b_t *usart = u->base;
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
      if (u->flags & STM32_UART_FIFO)
        usart->cr1 &= ~USART_CR1_TXFEIE;
      else
        usart->cr1 &= ~USART_CR1_TXEIE;
      return;
    }
    len = m->len;
    data = BMOS_OP_MSG_GET_DATA(m);
  }

  if (u->flags & STM32_UART_FIFO) {
    unsigned int count = 0;
    while ((isr & UART_ISR_TXFNF) && (len > 0)) {
      usart->tdr = *data;

      len--;
      data++;
      count++;
      isr = usart->isr;
    }
  } else {
    usart->tdr = *data;

    len--;
    data++;
  }

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
  if (!m) {
    FAST_LOG('u', "rx overrun\n", 0, 0);
    u->stats.overrun++;
  } else {
    msgdata = BMOS_OP_MSG_GET_DATA(m);
    len = circ_buf_read(&u->cb, msgdata, MSGDATA_LEN);
    if (len < MSGDATA_LEN)
      len += circ_buf_read(&u->cb, msgdata + len, MSGDATA_LEN);
    op_msg_put(u->rxq, m, u->op, len);
  }
}

static void rx_fifo(uart_t *u, unsigned int isr)
{
  stm32_usart_b_t *usart = u->base;
  int c;
  bmos_op_msg_t *m;
  unsigned char *msgdata;
  unsigned int count = 0;

  m = op_msg_get(u->pool);
  if (!m) {
    u->stats.overrun++;

    while (isr & UART_ISR_RXNE) {
      c = usart->rdr & 0xff;
      count++;
      isr = usart->isr;
    }

    FAST_LOG('u', "rx overrun: discarded %d\n", count, 0);

    return;
  } else
    msgdata = BMOS_OP_MSG_GET_DATA(m);

  while ((isr & UART_ISR_RXNE) && (count < MSGDATA_LEN)) {
    c = usart->rdr & 0xff;

    *msgdata++ = (unsigned char)c;
    count++;

    isr = usart->isr;
  }

  if (count == 0)
    op_msg_return(m);
  else
    op_msg_put(u->rxq, m, u->op, count);
}

static void usart_isr(void *data)
{
  uart_t *u = (uart_t *)data;
  stm32_usart_b_t *usart = u->base;
  unsigned int isr, cr1;

  isr = usart->isr;
  cr1 = usart->cr1;

  if (isr & UART_ISR_ORE) {
    usart->icr |= UART_ISR_ORE;
    u->stats.hw_overrun++;
    FAST_LOG('u', "rx overrun hw A\n", 0, 0);
  }

  if (isr & UART_ISR_IDLE) {
    usart->icr = UART_ISR_IDLE;

    if (u->flags & STM32_UART_FIFO)
      rx_fifo(u, isr);
    else if (circ_buf_used(&u->cb) > 0)
      sendcb(u);
  }

  if (isr & UART_ISR_RXFT)
    rx_fifo(u, isr);
  else if ((isr & UART_ISR_RXNE) && (cr1 & USART_CR1_RXNEIE)) {
    unsigned char c;
    int len;

    c = usart->rdr & 0xff;

    len = circ_buf_write(&u->cb, &c, 1);
    if (len == 0) {
      sendcb(u);
      (void)circ_buf_write(&u->cb, &c, 1);
    }
  }

  if ( ((cr1 & USART_CR1_TXFEIE) && (isr & UART_ISR_TXFE))
       || ((cr1 & USART_CR1_TXEIE) && (isr & UART_ISR_TXE)) )
    usart_tx(u, isr);
}

static void _put(void *p)
{
  uart_t *u = (uart_t *)p;
  stm32_usart_b_t *usart = u->base;

  if (u->flags & STM32_UART_FIFO)
    usart->cr1 |= USART_CR1_TXFEIE;
  else
    usart->cr1 |= USART_CR1_TXEIE;
}

static void _put_pool(void *p)
{
  unsigned int saved;
  uart_t *u = (uart_t *)p;

  saved = interrupt_disable();

  if (circ_buf_used(&u->cb) > 0)
    sendcb(u);

  interrupt_enable(saved);
}

bmos_queue_t *uart_open(uart_t *u, unsigned int baud, bmos_queue_t *rxq,
                        unsigned int op)
{
  stm32_usart_b_t *usart = u->base;
  const char *pool_name = "upool", *tx_queue_name = "utx";
  unsigned int cr1;

  usart_set_baud(usart, baud, u->clock, u->flags);

  usart->cr1 = 0;
  cr1 = CR1_DEFAULT;

  cr1 |= USART_CR1_IDLEIE;
  usart->cr2 = 0;

  if (u->flags & STM32_UART_FIFO) {
    cr1 |= USART_CR1_FIFOEN;
    reg_set_field(&usart->cr3, 3, USART_CR3_RXFTCFG_OFS, USART_FIFO_THRES_7_8);
    usart->cr3 |= USART_CR3_RXFTIE;
  } else {
    cr1 |= USART_CR1_RXNEIE;
    circ_buf_init(&u->cb, MSGDATA_POW2);
  }
  usart->cr1 = cr1;

  if (u->pool_name)
    pool_name = u->pool_name;
  u->pool = op_msg_pool_create(pool_name, QUEUE_TYPE_DRIVER, 4, MSGDATA_LEN);
  XASSERT(u->pool);

  if (u->tx_queue_name)
    tx_queue_name = u->tx_queue_name;
  u->txq = queue_create(tx_queue_name, QUEUE_TYPE_DRIVER);
  XASSERT(u->txq);

  (void)queue_set_put_f(u->txq, _put, (void *)u);
  (void)queue_set_put_f(u->pool, _put_pool, (void *)u);

  u->rxq = rxq;
  u->op = (unsigned short)op;

  irq_register(u->name, usart_isr, u, u->irq);

  return u->txq;
}
#endif
