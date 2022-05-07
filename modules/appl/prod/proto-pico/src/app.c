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

#include <stdio.h>

#include "tusb.h"

#include "cortexm.h"
#include "hal_board.h"
#include "hal_int.h"
#include "hal_int_cpu.h"
#include "hal_time.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "debug_ser.h"
#include "xtime.h"

#include "bmos_task.h"
#include "bmos_queue.h"
#include "bmos_op_msg.h"
#include "bmos_syspool.h"

#include "rp2040_hal_gpio.h"
#include "rp2040_hal_resets.h"

#include <pico/mutex.h>

void panic(const char *fmt, ...)
{
  for (;;)
    ;
}

void critical_section_init(critical_section_t *critsec)
{
}

void mutex_init(mutex_t *mtx)
{
}

void mutex_exit(mutex_t *mtx)
{
}

bool mutex_enter_timeout_ms(mutex_t *mtx, uint32_t timeout_ms)
{
  return true;
}

bool running_on_fpga()
{
  return false;
}

bmos_queue_t *syspool;

static int led_state = 0;
static xtime_ms_t last_blink = 0;
#define LONG_WAIT 6000
#define SHORT_WAIT 100
static xtime_diff_ms_t wait = LONG_WAIT;

#define LED_PIN 25

static void led_set(int num, int v)
{
  gpio_set(LED_PIN, v);
}

void blink()
{
  xtime_ms_t now;

  now = xtime_ms();
  if (xtime_diff_ms(now, last_blink) >= wait) {
    led_state ^= 1;
    led_set(0, led_state);
    last_blink = now;
    if (wait == LONG_WAIT)
      wait = SHORT_WAIT;
    else
      wait = LONG_WAIT;
  }
}

void systick_hook(void)
{
  blink();
}

hal_time_us_t hal_time_us()
{
  return time_us_32();
}

#if 0
static void polled_shell(void)
{
  shell_t sh;

  shell_init(&sh, "> ");

  for (;;) {
    if (uart_is_readable(DEBUG_UART)) {
      int c = uart_getc(DEBUG_UART);
      shell_input(&sh, c);
    } else
      task_delay(10);
  }
}
#endif

static void debug_uart_pins_init()
{
  gpio_init_attr(0, GPIO_ATTR_PICO(GPIO_FUNC_UART));
  gpio_init_attr(1, GPIO_ATTR_PICO(GPIO_FUNC_UART));
}

uart_t debug_uart = { "debugser", (void *)0x40034000, 125000000, 20 };

#define SHELL_SRC_COUNT 1
#define OP_UART1_DATA 0
#define OP_UART2_DATA 1

typedef struct {
  unsigned short dest;
  bmos_queue_t *rxq;
  bmos_queue_t *txq[SHELL_SRC_COUNT];
  unsigned short txop[SHELL_SRC_COUNT];
} shell_info_t;

static shell_info_t shell_info;

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

#define CONFIG_PICO_USB_ENABLE 1

#if CONFIG_PICO_USB_ENABLE
static shell_t cdc_sh;
static bmos_queue_t *cdc_tx;

static void usb_task(void *arg)
{
  tusb_init();

  task_set_tls(TLS_IND_STDOUT, cdc_tx);

  shell_init(&cdc_sh, "> ");

  for (;;) {
    tud_task();

    if ( tud_cdc_available() ) {
      char buf[64];
      unsigned int count, i;

      count = tud_cdc_read(buf, sizeof(buf));

      for (i = 0; i < count; i++)
        shell_input(&cdc_sh, buf[i]);
    }

    task_delay(1);
  }
}

static void cdc_shell_put(void *arg)
{
  bmos_op_msg_t *m;
  int len;
  unsigned char *data;

  for (;;) {
    m = op_msg_get(cdc_tx);
    if (!m)
      break;

    len = m->len;

    data = BMOS_OP_MSG_GET_DATA(m);

    for (;;) {
      int l;

      l = (int)tud_cdc_write(data, len);
      tud_cdc_write_flush();

      len -= l;
#if 0
      if (len <= 0)
        break;

      data += l;

      task_delay(1);
#else
      break;
#endif
    }

    op_msg_return(m);
  }
}
#endif

unsigned int hal_cpu_clock = 125000000;

extern void clocks_init();

#define UART0_BASE 0x40034000
#define UART1_BASE 0x40038000

int main()
{
  INTERRUPT_OFF();
  rp2040_reset_clr(RESETS_RESET_PADS_BANK0);
  rp2040_reset_clr(RESETS_RESET_IO_BANK0);

  gpio_init(LED_PIN, GPIO_OUTPUT);
  gpio_set(LED_PIN, 1);

  clocks_init();

  hal_cpu_init();

  rp2040_reset_clr(RESETS_RESET_UART0);
  debug_uart_pins_init();
  debug_uart_init((void *)UART0_BASE, 115200, hal_cpu_clock, 0);
  debug_puts("\nAPPL\n\n");

  task_init(shell_task, NULL, "shell", 2, 0, 4096);

#if CONFIG_PICO_USB_ENABLE
  cdc_tx = queue_create("cdc_tx", QUEUE_TYPE_DRIVER);
  (void)queue_set_put_f(cdc_tx, cdc_shell_put, 0);
  task_init(usb_task, NULL, "usbshell", 2, 0, 4096);
#endif

  shell_info.rxq = queue_create("sh1rx", QUEUE_TYPE_TASK);
  shell_info.txq[0] = uart_open(&debug_uart, 115200, shell_info.rxq,
                                OP_UART1_DATA);
  shell_info.txop[0] = 0;

  io_set_output(shell_info.txq[0], 0);

  syspool = op_msg_pool_create("sys", QUEUE_TYPE_TASK, SYSPOOL_COUNT,
                               SYSPOOL_SIZE);

  systick_init();

  task_start();

  for (;;)
    ;
}
