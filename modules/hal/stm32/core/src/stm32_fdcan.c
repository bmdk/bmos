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

#include <string.h>
#include <stdio.h>

#include "common.h"
#include "fast_log.h"
#include "hal_can.h"
#include "hal_int.h"
#include "io.h"
#include "shell.h"
#include "xslog.h"
#include "stm32_hal.h"
#include "xassert.h"
#if BMOS
#include "bmos_op_msg.h"
#include "bmos_msg_queue.h"
#endif

typedef struct {
  unsigned int crel;
  unsigned int endn;
  unsigned int pad1;
  unsigned int dbtp;
  unsigned int test;
  unsigned int rwd;
  unsigned int cccr;
  unsigned int nbtp;
  unsigned int tscc;
  unsigned int tscv;
  unsigned int tocc;
  unsigned int tocv;
  unsigned int pad2[4];
  unsigned int ecr;
  unsigned int psr;
  unsigned int tdcr;
  unsigned int pad3;
  unsigned int ir;
  unsigned int ie;
  unsigned int ils;
  unsigned int ile;
  unsigned int pad4[8];
  unsigned int gfc;
#if STM32_H7XX
  unsigned int sidfc;
  unsigned int xidfc;
  unsigned int pad5;
#endif
  unsigned int xidam;
  unsigned int hpms;
#if STM32_H7XX
  unsigned int ndnat[2];
  unsigned int rxf0c;
#else
  unsigned int pad5;
#endif
  unsigned int rxf0s;
  unsigned int rxf0a;
#if STM32_H7XX
  unsigned int rxbc;
  unsigned int rxf1c;
#endif
  unsigned int rxf1s;
  unsigned int rxf1a;

#if STM32_H7XX
  unsigned int rxesc;
#else
  unsigned int pad6[8];
#endif

  unsigned int txbc;
  unsigned int txfqs;
#if STM32_H7XX
  unsigned int txesc;
#endif
  unsigned int txbrp;
  unsigned int txbar;
  unsigned int txbcr;
  unsigned int txbto;
  unsigned int txbcf;
  unsigned int txbtie;
  unsigned int txbcie;
#if STM32_H7XX
  unsigned int pad6[2];
  unsigned int txefc;
#endif
  unsigned int txefs;
  unsigned int txefa;

#if !STM32_H7XX
  unsigned int pad7[5];

  unsigned int ckdiv;
#endif
} stm32_fdcan_t;

typedef struct {
  unsigned int id;
  unsigned int flags;
  union {
    unsigned char c[64];
    unsigned int i[8];
  } data;
} fdcan_buf_t;

#ifdef STM32_G4XX
#define FDCAN1_BASE 0x40006400
#define FDCAN2_BASE 0x40006800
#define FDCAN3_BASE 0x40006C00

#define FDCAN_MES_BASE 0x4000A400
#else
#define FDCAN1_BASE 0x4000A000
#define FDCAN2_BASE 0x4000A400
#define FDCAN3_BASE 0x4000D400

#define FDCAN_MES_BASE 0x4000AC00
#endif

#define FDCAN_DBTP(tdc, dbrp, dtseg1, dtseg2, dsjw) \
  ((((tdc) & 1) << 23) | (((dbrp) & 0x1f) << 16) | \
   (((dtseg1) & 0x1f) << 8) | (((dtseg2) & 0xf) << 4) | ((dsjw) & 0xf))

#define FDCAN_NBTP(nbrp, ntseg1, ntseg2, nsjw) \
  ( ((((unsigned int)(nsjw) - 1) & 0x7f) << 25) | \
    ((((unsigned int)(nbrp) - 1) & 0x1ff) << 16) | \
    ((((unsigned int)(ntseg1) - 1) & 0xff) << 8) | \
    (((unsigned int)(ntseg2) - 1) & 0x7f) )

#define FDCAN_CCCR_INIT BIT(0)
#define FDCAN_CCCR_CCE BIT(1)
#define FDCAN_CCCR_MON BIT(5)
#define FDCAN_CCCR_TEST BIT(7)
#define FDCAN_CCCR_FDOE BIT(8)
#define FDCAN_CCCR_BRSE BIT(9)

#define FDCAN_TCBC_TFQM BIT(24)

#define FDCAN_IR_RF0N BIT(0)
#define FDCAN_IR_TC BIT(7)

#define FDCAN_GFC(lse, lss, anfs, anfe) ((((lse) & 0xff) << 24) | \
                                         (((lss) & 0xff) << 16) | \
                                         (((anfs) & 0x3) << 4) | \
                                         (((anfe) & 0x3) << 2))

#define FDCAN_TXBUF_ID_ESI BIT(31)
#define FDCAN_TXBUF_ID_XTD BIT(30)
#define FDCAN_TXBUF_ID_RTR BIT(29)
#define FDCAN_TXBUF_ID_ID(id) (((id) & 0x7ff) << 18)
#define FDCAN_TXBUF_ID_IDEXT(id) ((id) & 0x1ffffff)

#define FDCAN_TXBUF_FLAGS_MM(v) (((v) & 0xff) << 24)
#define FDCAN_TXBUF_FLAGS_EFC BIT(23)
#define FDCAN_TXBUF_FLAGS_FDF BIT(21)
#define FDCAN_TXBUF_FLAGS_BRS BIT(20)
#define FDCAN_TXBUF_FLAGS_DLC(v) (((v) & 0xf) << 16)

#define MESRAM_FILTER_OFS 0
#define MESRAM_FILTER_EXT_OFS 0x070
#define MESRAM_RXFIFO0_OFS 0x0b0
#define MESRAM_RXFIFO1_OFS 0x188
#define MESRAM_TXEVENT_OFS 0x260
#define MESRAM_TXBUF_OFS 0x278

#define FDCAN_TXFQS_TFQF BIT(21)
#define FDCAN_TXFQS_MASK 0x3

void fdcan_rx();

#define FDCAN_FILT_SFT_RANGE 0
#define FDCAN_FILT_SFT_DUAL 1
#define FDCAN_FILT_SFT_CLASSIC 2
#define FDCAN_FILT_SFT_DISABLED 3

#define FDCAN_FILT_SFEC_DISABLED 0
#define FDCAN_FILT_SFEC_FIFO0 1
#define FDCAN_FILT_SFEC_FIFO1 2
#define FDCAN_FILT_SFEC_REJECT 3
#define FDCAN_FILT_SFEC_PRIO 4
#define FDCAN_FILT_SFEC_PRIOFIFO0 5
#define FDCAN_FILT_SFEC_PRIOFIFO1 6

#define FDCAN_FILTER(sft, sfec, sfid1, sfid2) \
  ( (((sft) & 0x3) << 30) | (((sfec) & 0x7) << 27) | \
    (((sfid1) & 0x7ff) << 16) | ((sfid2) & 0x7ff))

static int fdcan_send(volatile stm32_fdcan_t *fdcan, can_t *pkt)
{
  fdcan_buf_t *tx = (void *)(FDCAN_MES_BASE + MESRAM_TXBUF_OFS);
  unsigned int txfqs, idx, val;

  txfqs = fdcan->txfqs;
  if (txfqs & FDCAN_TXFQS_TFQF)
    return -1;

  idx = (txfqs >> 16) & FDCAN_TXFQS_MASK;

  memcpy(&val, &pkt->data[0], 4);
  tx[idx].data.i[0] = val;
  memcpy(&val, &pkt->data[4], 4);
  tx[idx].data.i[1] = val;

  tx[idx].id = FDCAN_TXBUF_ID_ID(pkt->id);
  tx[idx].flags = FDCAN_TXBUF_FLAGS_DLC(pkt->len);

  fdcan->txbar = BIT(idx);

  return 0;
}

#if BMOS
static void _tx(candev_t *c)
{
  volatile stm32_fdcan_t *fdcan = c->base;
  bmos_op_msg_t *m;
  unsigned int len, txfqs;

  txfqs = fdcan->txfqs;
  if (txfqs & FDCAN_TXFQS_TFQF)
    return;

  m = op_msg_get(c->txq);
  if (!m) {
    fdcan->ie &= ~FDCAN_IR_TC;
    return;
  }

  len = m->len;
  if (len == sizeof(can_t)) {
    can_t *pkt = (can_t *)BMOS_OP_MSG_GET_DATA(m);

    fdcan_send(fdcan, pkt);
  }

  op_msg_return(m);
}

void irq_fdcan(void *arg)
{
  candev_t *c = arg;
  volatile stm32_fdcan_t *fdcan = c->base;
  bmos_op_msg_t *m;
  can_t *cdata;
  unsigned int ir = fdcan->ir;

  if (ir & FDCAN_IR_TC)
    _tx(c);

  if (ir & FDCAN_IR_RF0N) {
    unsigned int rxs, flags, id, idx, cnt;
    fdcan_buf_t *rx = (void *)(FDCAN_MES_BASE + MESRAM_RXFIFO0_OFS);

    for (;;) {
      rxs = fdcan->rxf0s;
      idx = (rxs >> 8) & 0x3;
      cnt = (rxs) & 0xf;

      if (cnt == 0)
        break;

      m = op_msg_get(c->pool);
      if (m) {
        cdata = BMOS_OP_MSG_GET_DATA(m);

        id = rx[idx].id;
        cdata->id = (id >> 18) & 0x7ff;
        flags = rx[idx].flags;
        cdata->len = (flags >> 16) & 0xf;

        memcpy(cdata->data, &rx[idx].data.c[0], cdata->len);
        op_msg_put(c->rxq, m, c->op, sizeof(can_t));
      } else
        c->stats.overrun++;

      fdcan->rxf0a = idx;
    }
  }

  fdcan->ir = ir;
}

static void _put(void *p)
{
  unsigned int saved;
  candev_t *c = p;

  saved = interrupt_disable();

  _tx(c);

  interrupt_enable(saved);
}

static void fdcan_filter(const unsigned int *id, unsigned int id_len)
{
  unsigned int *filt = (void *)(FDCAN_MES_BASE + MESRAM_FILTER_OFS);
  unsigned int i;

  for (i = 0; i < id_len; i++)
    filt[i] = FDCAN_FILTER(FDCAN_FILT_SFT_CLASSIC,
                           FDCAN_FILT_SFEC_FIFO0, id[i], 0x7ff);
}

static void fdcan_init(candev_t *c, const unsigned int *id, unsigned int id_len)
{
  volatile stm32_fdcan_t *fdcan = c->base;
  can_params_t *p = &c->params;

  fdcan->cccr = (FDCAN_CCCR_CCE | FDCAN_CCCR_INIT);

  fdcan->txbc &= ~FDCAN_TCBC_TFQM; /* FIFO mode */

  fdcan->nbtp = FDCAN_NBTP(p->prediv, p->ts1, p->ts2, p->sjw);

#if STM32_H7XX
  fdcan->sidfc = (id_len << 16) | ((MESRAM_FILTER_OFS) & 0xffff);
  fdcan->rxf0c = (3 << 16) |  ((MESRAM_RXFIFO0_OFS) & 0xffff);
  fdcan->txbc = (3 << 24) | ((MESRAM_TXBUF_OFS) & 0xffff);
  fdcan->rxesc = (7 << 8) | (7 << 4) | (7 << 0);
  fdcan->txesc = (7 << 0);
#endif

#if 0
  fdcan->dbtp = FDCAN_DBTP(tdp, nbrp, ntseg1, ntseg2, nsjw);
#endif

  /* enable rx fifo 0 interrupt */
  fdcan->ie = FDCAN_IR_RF0N;
  /* all interrupts on interrupt 0 */
  fdcan->ils = 0;
  /* enable interrupt 0 */
  fdcan->ile = BIT(0);

  if (id_len > 0) {
    fdcan_filter(id, id_len);
    fdcan->gfc = FDCAN_GFC(0, id_len, 2, 2);
  } else
    fdcan->gfc = FDCAN_GFC(0, 0, 0, 0);

  fdcan->cccr &= ~(FDCAN_CCCR_CCE | FDCAN_CCCR_INIT);
}

bmos_queue_t *can_open(candev_t *c, const unsigned int *id,
                       unsigned int id_len,
                       bmos_queue_t *rxq, unsigned int op)
{
  const char *pool_name = "fdcanpool", *tx_queue_name = "fdcantx";

  if (c->pool_name)
    pool_name = c->pool_name;
  c->pool = op_msg_pool_create(pool_name, QUEUE_TYPE_DRIVER, 4, sizeof(can_t));
  XASSERT(c->pool);

  if (c->tx_queue_name)
    tx_queue_name = c->tx_queue_name;
  c->txq = queue_create(tx_queue_name, QUEUE_TYPE_DRIVER);
  XASSERT(c->txq);

  (void)queue_set_put_f(c->txq, _put, (void *)c);

  c->rxq = rxq;
  c->op = (unsigned short)op;

  fdcan_init(c, id, id_len);

  irq_register(c->name, irq_fdcan, c, c->irq);

  return c->txq;
}
#endif
