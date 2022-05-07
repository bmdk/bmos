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

#include "bmos_msg_queue.h"
#include "bmos_op_msg.h"
#include "bmos_sem.h"
#include "bmos_syspool.h"
#include "bmos_task.h"
#include "hal_int.h"
#include "io.h"
#include "shell.h"
#include "hal_board.h"
#include "tusb.h"
#include "xslog.h"

#define MAX_RETRIES 3

unsigned int SystemCoreClock;

static bmos_sem_t *usb_wakeup;
static shell_t cdc_sh;
static bmos_queue_t *cdc_tx;
static unsigned int dropped = 0;

static void usb_int(void *data)
{
  tud_int_handler(0);
}

void usb_init(void);

static void usb_task(void *arg)
{
  SystemCoreClock = hal_cpu_clock;
  usb_init();
  tusb_init();

  for (;;)
    tud_task();
}

static void cdc_task(void *arg)
{
  task_set_tls(TLS_IND_STDOUT, cdc_tx);

  shell_init(&cdc_sh, "> ");

  for (;;) {
    (void)sem_wait_ms(usb_wakeup, 1000);

    if ( tud_cdc_available() ) {
      char buf[64];
      unsigned int count, i;

      count = tud_cdc_read(buf, sizeof(buf));

      for (i = 0; i < count; i++)
        shell_input(&cdc_sh, buf[i]);
    }
  }
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
}

void tud_cdc_rx_cb(uint8_t itf)
{
  sem_post(usb_wakeup);
}

static void cdc_shell_put(void *arg)
{
  bmos_op_msg_t *m;
  int len;
  unsigned char *data;
  unsigned int count;

  for (;;) {
    m = op_msg_get(cdc_tx);
    if (!m)
      break;

    len = m->len;

    data = BMOS_OP_MSG_GET_DATA(m);

    count = 0;

    for (;;) {
      int l;

      l = (int)tud_cdc_write(data, len);
      tud_cdc_write_flush();

      len -= l;
      if (len <= 0)
        break;
      data += l;

      /* ok as long as progress is made */
      if (l != 0)
        count = 0;
      else if (++count > MAX_RETRIES) {
        dropped++;
        break;
      }

      task_delay(1);
    }

    op_msg_return(m);
  }
}

void tusb_cdc_init()
{
  irq_register("usb", usb_int, 0, 67);
  task_init(usb_task, 0, "usb", 2, 0, 512);
  task_init(cdc_task, 0, "usb_cdc", 1, 0, 1024);
  usb_wakeup = sem_create("usb_wakeup", 0);
  cdc_tx = queue_create("cdc_tx", QUEUE_TYPE_DRIVER);
  (void)queue_set_put_f(cdc_tx, cdc_shell_put, 0);
}

static int cmd_cdc_stats(int argc, char *argv[])
{
  xprintf("dropped cdc packets %d\n", dropped);

  return 0;
}

SHELL_CMD(cdc_stats, cmd_cdc_stats);
