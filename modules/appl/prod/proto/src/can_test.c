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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmos_msg_queue.h"
#include "bmos_op_msg.h"
#include "bmos_sem.h"
#include "bmos_syspool.h"
#include "bmos_task.h"

#include "hal_can.h"
#include "hal_int.h"
#include "io.h"
#include "shell.h"
#include "xslog.h"
#include "xlib.h"
#include "common.h"

#if STM32_F1XX || STM32_F4XX || STM32_G4XX || AT32_F4XX
#define CAN1_BASE 0x40006400
#define CAN2_BASE 0x40006800
#define CAN3_BASE 0x40006C00
#elif STM32_L4XX
#define CAN1_BASE 0x40006400
#elif STM32_UXXX
#define CAN1_BASE 0x4000A400
#elif STM32_H7XX
#define CAN1_BASE 0x4000A000
#define CAN2_BASE 0x4000A400
#define CAN3_BASE 0x4000D400
#else
#error Define CAN base addresses
#endif

#define OP_CAN1_DATA 1

typedef struct {
  bmos_queue_t *rxq;
  bmos_queue_t *txq;
  bmos_queue_t *tx_pool;
} can_task_data_t;

static can_task_data_t can_task_data;

static const unsigned int can_id_list[] = {
#if 0
  0xf2, 0xf3
#endif
};

#if AT32_F4XX
/* APB1 clock 120 MHz */
static candev_t can0 = {
  .name     = "can1",
  .base     = (void *)CAN1_BASE,
  .irq      = 20,
  /* 1Mbit */
  .params   = {
    .prediv = 12,
    .ts1    = 5,
    .ts2    = 4,
    .sjw    = 1
  },
  .tx_irq   = 19
};
#elif STM32_F1XX
/* APB1 clock 36 MHz */
static candev_t can0 = {
  .name     = "can1",
  .base     = (void *)CAN1_BASE,
  .irq      = 20,
  /* 1Mbit */
  .params   = {
    .prediv = 4,
    .ts1    = 4,
    .ts2    = 4,
    .sjw    = 1
  },
  .tx_irq   = 19
};
#elif STM32_G4XX
/* HSE at 24MHz clock */
static candev_t can0 = {
  .name     = "can0",
  .base     = (void *)CAN1_BASE,
  .irq      = 21,
  .params   = {
    .prediv = 1,
    .ts1    = 12,
    .ts2    = 11,
    .sjw    = 3
  }
};
#elif STM32_UXXX
/* PLL1Q at 160 MHz clock */
static candev_t can0 = {
  .name     = "can0",
  .base     = (void *)CAN1_BASE,
  .irq      = 39,
  .params   = {
    .prediv = 8,
    .ts1    = 10,
    .ts2    = 9,
    .sjw    = 3
  }
};
#else
/* HSE at 25MHz clock */
static candev_t can0 = {
  .name     = "can0",
  .base     = (void *)CAN1_BASE,
  .irq      = 19,
  .params   = {
    .prediv = 1,
    .ts1    = 12,
    .ts2    = 12,
    .sjw    = 3
  }
};
#endif

static void send_can(unsigned int id, void *data, unsigned int len)
{
  bmos_op_msg_t *m;
  can_t *pkt;

  m = op_msg_wait(can_task_data.tx_pool);

  pkt = (can_t *)BMOS_OP_MSG_GET_DATA(m);

  pkt->id = id;
  pkt->len = len;

  memcpy(pkt->data, data, len);

  op_msg_put(can_task_data.txq, m, 0, sizeof(can_t));
}

void task_can()
{
  char buf[36];

  can_task_data.rxq = queue_create("can0rx", QUEUE_TYPE_TASK);
  can_task_data.txq = can_open(&can0, can_id_list, ARRSIZ(can_id_list),
                               can_task_data.rxq, OP_CAN1_DATA);
  can_task_data.tx_pool = op_msg_pool_create("can1tx", QUEUE_TYPE_TASK,
                                             4, sizeof(can_t));

  for (;;) {
    bmos_op_msg_t *m;

    m = op_msg_wait(can_task_data.rxq);

    if (m->op == OP_CAN1_DATA && m->len == sizeof(can_t)) {
      int i, n, r = sizeof(buf) - 1;
      char *bufp;
      can_t *pkt = BMOS_OP_MSG_GET_DATA(m);

      bufp = buf;
      n = snprintf(bufp, r, "can rx: id %03x(%d) ", pkt->id, pkt->len);
      bufp += n;
      r -= n;

      for (i = 0; i < pkt->len; i++) {
        n = snprintf(bufp, r, "%02x", pkt->data[i]);
        bufp += n;
        r -= n;
      }
      xslog(LOG_INFO, buf);
    }

    op_msg_return(m);
  }
}

int cmd_can(int argc, char *argv[])
{
  unsigned int id, i, len;
  unsigned char data[8];
  char *b;

  if (argc < 3)
    return -1;

  id = strtoul(argv[1], 0, 16);
  len = strlen(argv[2]);

  if (len % 2 == 1)
    return -1;

  len /= 2;

  if (len > 8)
    return -1;

  b = argv[2];

  for (i = 0; i < len; i++) {
    unsigned int val;
    int err;

    err = scanx(b, 2, &val);
    if (err < 2)
      return -1;
    data[i] = val;
    b += 2;
  }

  send_can(id, data, len);

  return 0;
}

SHELL_CMD(can, cmd_can);
