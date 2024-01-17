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

#include <string.h>

#include "common.h"
#include "debug_ser.h"
#include "fast_log.h"
#include "hal_can.h"
#include "hal_int.h"
#include "io.h"
#include "shell.h"
#include "stm32_hal.h"
#include "xassert.h"
#if BMOS
#include "bmos_op_msg.h"
#include "bmos_msg_queue.h"
#endif
#include "shell.h"

typedef struct {
  reg32_t mcr;
  reg32_t msr;
  reg32_t tsr;
  reg32_t rfr[2];
  reg32_t ier;
  reg32_t esr;
  reg32_t btr;
  reg32_t pad0[88];

  struct {
    reg32_t i;
    reg32_t dt;
    reg32_t d[2];
  } t[3];

  struct {
    reg32_t i;
    reg32_t dt;
    reg32_t d[2];
  } r[2];

  reg32_t pad1[12];

  reg32_t fmr;
  reg32_t fm1r;
  reg32_t pad2;
  reg32_t fs1r;
  reg32_t pad3;
  reg32_t ffa1r;
  reg32_t pad4;
  reg32_t fa1r;

  reg32_t pad5[8];

  struct {
    reg32_t id;
    reg32_t mask;
  } f[28];

} stm32_can_t;

#define CAN_MCR_INRQ BIT(0)
#define CAN_MCR_SLEEP BIT(1)
#define CAN_MCR_TXFP BIT(2)
#define CAN_MCR_RFLM BIT(3)
#define CAN_MCR_NART BIT(4)
#define CAN_MCR_AWUM BIT(5)
#define CAN_MCR_ABOM BIT(6)
#define CAN_MCR_TTCM BIT(7)
#define CAN_MCR_DBF BIT(16)

#define CAN_MSR_INAK BIT(0)
#define CAN_MSR_SLAK BIT(1)
#define CAN_MSR_ERRI BIT(2)

#define CAN_IER_TMEIE BIT(0)
#define CAN_IER_FMPIE0 BIT(1)
#define CAN_IER_FFIE0 BIT(2)
#define CAN_IER_FOVIE0 BIT(3)
#define CAN_IER_ERRIE BIT(15)

#define CAN_TSR_TME0 BIT(26)
#define CAN_TSR_RQCP0 BIT(0)

#define CAN_RFR_FOVR BIT(4)
#define CAN_RFR_RFOM BIT(5)

static int can_send(stm32_can_t *can, can_t *pkt)
{
  unsigned int val[2];

  if (can->t[0].i & 1)
    return -1;

  memcpy(val, pkt->data, pkt->len);

  can->t[0].d[0] = val[0];
  can->t[0].d[1] = val[1];

  can->t[0].dt = pkt->len;
  can->t[0].i = ((pkt->id & 0x3ff) << 21) | 1;

  return 0;
}

static void _can_filter_add_mask(stm32_can_t *can,
                                 unsigned int index, unsigned int id,
                                 unsigned int mask)
{
  can->fs1r |= BIT(index);
  can->fa1r |= BIT(index);
  can->f[index].id = id << 21;
  can->f[index].mask = mask << 21;
}

static void _can_filter_add(stm32_can_t *can,
                            unsigned int index, unsigned int id)
{
  _can_filter_add_mask(can, index, id, 0x7ff);
}

static void _can_filter_add_promisc(stm32_can_t *can)
{
  _can_filter_add_mask(can, 0, 0, 0);
}

static inline unsigned int val_param(unsigned int param, unsigned int mask)
{
  if (param == 0)
    return 0;
  return (param - 1) & mask;
}

static void can_init(candev_t *c, const unsigned int *id,
                     unsigned int id_len)
{
  stm32_can_t *can = c->base;
  can_params_t *p = &c->params;
  unsigned int i;

  can->mcr &= ~(CAN_MCR_SLEEP | CAN_MCR_DBF);

  can->mcr |= CAN_MCR_INRQ;

  while ((can->msr & CAN_MCR_INRQ) == 0)
    ;

  can->btr = (val_param(p->sjw, 0x3) << 24) | (val_param(p->ts2, 0x7) << 20) |
             (val_param(p->ts1, 0xf) << 16) | val_param(p->prediv, 0x3ff);

  can->fmr = 1;
  can->fm1r = 0;
  can->fs1r = 0;
  can->ffa1r = 0; /* FIFO 0 */

  if (id_len == 0)
    _can_filter_add_promisc(can);
  else
    for (i = 0; i < id_len; i++)
      _can_filter_add(can, i, id[i]);

  can->fmr = 0;

  can->mcr |= CAN_MCR_RFLM | CAN_MCR_TXFP | CAN_MCR_ABOM;
  can->mcr &= ~CAN_MCR_INRQ;

  can->ier = (CAN_IER_FMPIE0 | CAN_IER_FFIE0 | CAN_IER_FOVIE0 | CAN_IER_TMEIE);

  if (c->err_irq > 0)
    can->ier |= CAN_IER_ERRIE;
#if 0
  can->tsr = 0xffffffff;
#endif
}

#if BMOS
static void _tx(candev_t *c)
{
  stm32_can_t *can = c->base;
  bmos_op_msg_t *m;
  unsigned int len;

  if (!(can->tsr & CAN_TSR_TME0)) {
    can->ier |= CAN_IER_TMEIE;
    return;
  }

  m = op_msg_get(c->txq);
  if (!m) {
    can->ier &= ~CAN_IER_TMEIE;
    return;
  }

  len = m->len;
  if (len == sizeof(can_t)) {
    can_t *pkt = (can_t *)BMOS_OP_MSG_GET_DATA(m);

    can_send(can, pkt);
  }

  op_msg_return(m);
}

void can_isr(void *arg)
{
  candev_t *c = arg;
  stm32_can_t *can = c->base;
  bmos_op_msg_t *m;
  can_t *cdata;
  int count;
  unsigned int rfr = can->rfr[0];

  if (rfr & CAN_RFR_FOVR) {
    /* ack overflow interrupt */
    can->rfr[0] = CAN_RFR_FOVR;
    c->stats.hw_overrun++;
  }

  count = rfr & 0x3;
  if (count) {
    unsigned char *d = (unsigned char *)&can->r[0].d[0];

    m = op_msg_get(c->pool);
    if (m) {
      cdata = BMOS_OP_MSG_GET_DATA(m);

      cdata->id = (can->r[0].i >> 21) & 0x7ff;
      cdata->len = can->r[0].dt & 0xf;
      for (int i = 0; i < cdata->len; i++)
        cdata->data[i] = d[i];

      op_msg_put(c->rxq, m, c->op, sizeof(can_t));
    } else
      c->stats.overrun++;

    can->rfr[0] = CAN_RFR_RFOM;
  }
}

void can_tx_isr(void *arg)
{
  candev_t *c = arg;

  _tx(c);
}

void can_err_isr(void *arg)
{
  candev_t *c = arg;
  stm32_can_t *can = c->base;
  unsigned int esr = can->esr;

  FAST_LOG('c', "can error %x", esr, 0);

  /* ack error interrupt */
  can->msr = CAN_MSR_ERRI;
}

static void _put(void *p)
{
  unsigned int saved;
  candev_t *c = p;

  saved = interrupt_disable();

  _tx(c);

  interrupt_enable(saved);
}


bmos_queue_t *can_open(candev_t *c, const unsigned int *id,
                       unsigned int id_len, bmos_queue_t *rxq,
                       unsigned int op)
{
  const char *pool_name = "cpool", *tx_queue_name = "ctx";
  unsigned int rx_queue_len = 4;

  if (c->pool_name)
    pool_name = c->pool_name;
  if (c->rx_queue_len > 0)
    rx_queue_len = c->rx_queue_len;
  c->pool = op_msg_pool_create(pool_name, QUEUE_TYPE_DRIVER,
                               rx_queue_len, sizeof(can_t));
  XASSERT(c->pool);

  if (c->tx_queue_name)
    tx_queue_name = c->tx_queue_name;
  c->txq = queue_create(tx_queue_name, QUEUE_TYPE_DRIVER);
  XASSERT(c->txq);

  (void)queue_set_put_f(c->txq, _put, (void *)c);

  c->rxq = rxq;
  c->op = (unsigned short)op;

  can_init(c, id, id_len);

  if (c->err_irq > 0)
    irq_register(c->name, can_err_isr, c, c->err_irq);

  irq_register(c->name, can_tx_isr, c, c->tx_irq);
  irq_register(c->name, can_isr, c, c->irq);

  return c->txq;
}
#endif
