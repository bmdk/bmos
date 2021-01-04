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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmos_msg_queue.h"
#include "bmos_op_msg.h"
#include "bmos_sem.h"
#include "bmos_syspool.h"
#include "bmos_task.h"
#include "common.h"
#include "debug_ser.h"
#include "fast_log.h"
#include "hal_board.h"
#include "hal_gpio.h"
#include "hal_int.h"
#include "hal_time.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "stm32_exti.h"
#include "stm32_hal.h"
#include "stm32_regs.h"
#include "xslog.h"
#include "xtime.h"

static int led_state = 0;
static xtime_ms_t last_blink = 0;

void blink()
{
  xtime_ms_t now;

  now = xtime_ms();
  if (xtime_diff_ms(now, last_blink) >= 1000) {
    led_state ^= 1;
    led_set(0, led_state);
    last_blink = now;
  }
}

#if STM32_H7XX || STM32_F767 || STM32_L4XX
#define BUTTON_EXTI 13
#else
#define BUTTON_EXTI 11
#endif

void button_int(void *data)
{
  stm32_exti_irq_ack(BUTTON_EXTI);
  debug_puts("X\n");
}

static void polled_shell(void)
{
  shell_t sh;

  shell_init(&sh, "> ");

  for (;;) {
    int c = debug_getc();
    if (c >= 0)
      shell_input(&sh, c);
  }
}

void systick_init();

#if STM32_H7XX
#define SHELL_SRC_COUNT 2
#else
#define SHELL_SRC_COUNT 1
#endif

typedef struct {
  unsigned short dest;
  bmos_queue_t *rxq;
  bmos_queue_t *txq[SHELL_SRC_COUNT];
  unsigned short txop[SHELL_SRC_COUNT];
} shell_info_t;

static shell_info_t shell_info;

#define OP_UART1_DATA 0
#define OP_UART2_DATA 1

static int _xgetc(int timeout)
{
  unsigned char *data;
  static bmos_op_msg_t *m = NULL;
  static unsigned int pos = 0;
  int c;

  if (!m) {
    m = op_msg_wait_ms(shell_info.rxq, timeout);

    if (!m)
      return -1;

    if (m->op >= SHELL_SRC_COUNT || m->len == 0) {
      op_msg_return(m);
      return -1;
    }

    if (m->op != shell_info.dest && shell_info.txq[m->op] != 0) {
      io_set_output(shell_info.txq[m->op], shell_info.txop[m->op]);
      shell_info.dest = m->op;
    }

    pos = 0;
  }

  data = BMOS_OP_MSG_GET_DATA(m);

  c = data[pos];

  pos++;
  if (pos >= m->len) {
    op_msg_return(m);
    pos = 0;
    m = NULL;
  }

  return c;
}

static void shell_task(void *arg)
{
  shell_t sh;

  shell_init(&sh, "> ");

  for (;;) {
    int c = _xgetc(-1);

    shell_input(&sh, c);
  }
}

static void blink_task(void *arg)
{
  for (;;) {
    task_delay(1000);
    led_set(1, 1);
    task_delay(1000);
    led_set(1, 0);
  }
}

void systick_hook(void)
{
  blink();
}

bmos_queue_t *syspool;

extern uart_t debug_uart_2;

#if STM32_H735DK
void set_lcd(unsigned int start, unsigned int width, unsigned int height);

void lcd_task(void *arg)
{
  unsigned int i = 0;

  for (;;) {
    set_lcd(i++, 480, 272);
    task_delay(10);
  }
}
#endif

void task_net();
void task_led();

int main()
{
  interrupt_disable();
  hal_cpu_init();
  hal_board_init();
  hal_time_init();
  fast_log_init("TIS");

  led_set(0, 1);

  debug_puts("\nAPPL\n\n");

  xslog_init();

  xslog(LOG_INFO, "starting");

#if STM32_H7XX || STM32_L4XX || STM32_F767
  irq_register("ext", button_int, 0, 40);
#endif

  task_init(blink_task, NULL, "blink", 2, 0, 1024);

  shell_info.rxq = queue_create("sh1rx", QUEUE_TYPE_TASK);
  shell_info.txq[0] = uart_open(&debug_uart, 115200, shell_info.rxq,
                                OP_UART1_DATA);
  shell_info.txop[0] = 0;
#if STM32_H7XX
  shell_info.txq[1] = uart_open(&debug_uart_2, 115200, shell_info.rxq,
                                OP_UART2_DATA);
#endif
  shell_info.txop[1] = 0;
  shell_info.dest = OP_UART1_DATA;

  task_init(shell_task, &shell_info, "shell", 2, 0, 4096);

  io_set_output(shell_info.txq[0], 0);

#if STM32_H735DK
  task_init(lcd_task, NULL, "lcd", 2, 0, 512);
#endif

#if CONFIG_LWIP
  task_init(task_net, NULL, "net", 4, 0, 8192);
#endif

#if WS2811
  task_init(task_led, NULL, "lcd", 4, 0, 1024);
#endif

  syspool = op_msg_pool_create("sys", QUEUE_TYPE_TASK, 10, 64);

  systick_init();

  task_start();

  xputs("OOPS\n");

  for (;;)
    ;

  polled_shell();

  return 0;
}
