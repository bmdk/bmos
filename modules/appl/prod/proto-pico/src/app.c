#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/timer.h"

#include "tusb.h"

#include "cortexm.h"
#include "hal_int.h"
#include "hal_int_cpu.h"
#include "hal_time.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "xtime.h"

#include "bmos_task.h"
#include "bmos_queue.h"
#include "bmos_op_msg.h"
#include "bmos_syspool.h"

bmos_queue_t *syspool;

#define DEBUG_UART uart0

static int led_state = 0;
static xtime_ms_t last_blink = 0;
#define LONG_WAIT 6000
#define SHORT_WAIT 100
static xtime_diff_ms_t wait = LONG_WAIT;

#define LED_PIN 25

static void led_set(int num, int v)
{
  gpio_put(LED_PIN, v);
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

unsigned int hal_time_us()
{
  return time_us_32();
}

void debug_putc(int ch)
{
  uart_putc_raw(DEBUG_UART, ch);
}

static void polled_shell(void)
{
  shell_t sh;

  shell_init(&sh, "> ");

  for (;;) {
    if (uart_is_readable(DEBUG_UART)) {
      int c = uart_getc(DEBUG_UART);
      shell_input(&sh, c);
    } else {
      task_delay(10);
    }
  }
}

static void debug_uart_init()
{
  gpio_set_function(0, GPIO_FUNC_UART);
  gpio_set_function(1, GPIO_FUNC_UART);
}

uart_t debug_uart = { "debugser", (void *)0, 0 /* CLOCK */, 20 };

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

int main()
{
    debug_uart_init();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    interrupt_disable();

    task_init(shell_task, NULL, "shell", 2, 0, 4096);

    cdc_tx = queue_create("cdc_tx", QUEUE_TYPE_DRIVER);
    (void)queue_set_put_f(cdc_tx, cdc_shell_put, 0);
    task_init(usb_task, NULL, "usbshell", 2, 0, 4096);

    shell_info.rxq = queue_create("sh1rx", QUEUE_TYPE_TASK);
    shell_info.txq[0] = uart_open(&debug_uart, 115200, shell_info.rxq,
        OP_UART1_DATA);
    shell_info.txop[0] = 0;

    io_set_output(shell_info.txq[0], 0);

    syspool = op_msg_pool_create("sys", QUEUE_TYPE_TASK, SYSPOOL_COUNT,
        SYSPOOL_SIZE);

    systick_init();

    task_start();

    for(;;)
      ;
}
