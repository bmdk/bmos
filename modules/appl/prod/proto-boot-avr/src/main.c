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

#include "common.h"
#include "debug_ser.h"
#include "hal_avr_common.h"
#include "hal_board.h"
#include "hal_gpio.h"
#include "hal_avr_uart.h"
#include "hal_avr_i2c.h"
#include "hal_avr_spi.h"
#include "hal_time.h"
#include "io.h"
#include "shell.h"
#include "xmodem.h"
#include "xtime.h"
#include "hd44780.h"

#include "util.h"

#include <avr/interrupt.h>

#define APP_VER 0x0001

volatile xtime_ms_t systick_count = 0;

static char led_state = 0;
static unsigned int last_blink = 0;
#define LONG_WAIT 6000
#define SHORT_WAIT 100
static unsigned int wait = LONG_WAIT;
static unsigned int count = 0;

#if __AVR_ATmega128__
#define LED GPIO(0, 0)
#elif __AVR_AT90CAN128__
#define LED GPIO(4, 4)
#elif __AVR_ATmega328P__
#define LED GPIO(1, 5)
#else
#error FIXME
#endif

#define WDTCR_WDE BIT(3)

void avr_reset()
{
  hal_delay_us(5000000);
#if __AVR_ATmega328P__
  WDTCSR = WDTCR_WDE;
#else
  WDTCR = WDTCR_WDE;
#endif
}

#if __AVR_ATmega328P__
#define LED_ACTIVE_HIGH 1
#else
#define LED_ACTIVE_HIGH 0
#endif

#if LED_ACTIVE_HIGH
#define LED_ACTIVE 1
#define LED_INACTIVE 0
#else
#define LED_ACTIVE 0
#define LED_INACTIVE 1
#endif

void blink()
{
  count++;

  if (count - last_blink >= wait) {
    led_state ^= 1;
    if (led_state)
      gpio_set(LED, LED_ACTIVE);
    else
      gpio_set(LED, LED_INACTIVE);
    last_blink = count;
    if (wait == LONG_WAIT)
      wait = SHORT_WAIT;
    else
      wait = LONG_WAIT;
  }
}

#define TCCR_CS_OFF 0
#define TCCR_CS1 1
#define TCCR_CS8 2
#define TCCR_CS64 3
#define TCCR_CS256 4
#define TCCR_CS1024 5

#define MS_PERIOD 2000

static unsigned int next_period;

static void update_period()
{
  next_period += MS_PERIOD;
  OCR1A = next_period;
}

static unsigned int last_cnt;

typedef uint32_t time_us_t;

static time_us_t acc_us;

static void update_us(void)
{
  unsigned int now = TCNT1;

  acc_us += (now - last_cnt) >> 1;

  last_cnt = now;
}

hal_time_us_t hal_time_us()
{
  time_us_t t;

  /* disable interrupts */
  update_us();
  t = acc_us;
  /* enable interrupts */

  return t;
}

void hal_delay_us(hal_time_us_t us)
{
  hal_time_us_t start = hal_time_us();

  while (hal_time_us() - start < us)
    ;
}

ISR(TIMER1_COMPA_vect){
  systick_count++;

  update_period();

  update_us();

  blink();
}

void timer_init(void)
{
  TCCR1B = 0; /* disable */

  TCNT1 = 0;

  update_period();

#if __AVR_ATmega128__
  TIMSK = BIT(OCIE1A);  /* enable output compare 1 */
#elif __AVR_AT90CAN128__
  TIMSK1 = BIT(OCIE1A); /* enable output compare 1 */
#elif __AVR_ATmega328P__
  TIMSK1 = BIT(OCIE1A); /* enable output compare 1 */
#else
#error FIXME
#endif

  TCCR1A = 0;
  TCCR1B = TCCR_CS8 & 0x7; /* one tick every .5 us */
}

char pchar(unsigned char b);

static int cmd_i2c(int argc, char *argv[])
{
  unsigned char i;
  int err;
  unsigned char buf[8], rbuf[8];
  unsigned long addr, reg, len = 1;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'p':
    for (i = 8; i < 0x78; i++) {
      err = i2c_read_buf(i, 0, 0);
      if (err == 0)
        xprintf("addr %02x\n", i);
    }
    break;
  case 'r':
    if (argc < 4)
      return -1;
    addr = strtoul(argv[2], 0, 0);
    reg = strtoul(argv[3], 0, 0);
    if (argc > 4)
      len = strtoul(argv[4], 0, 0);
    for (i = 0; i < len; i++) {
      buf[0] = (unsigned char)(reg + i);
      err = i2c_write_read_buf(addr, buf, 1, rbuf, 1);
      if (err == 0)
        xprintf("%02x: %02x(%c)\n", buf[0], rbuf[0], pchar(rbuf[0]));
    }
  case 'i':
    i2c_info();
    break;
  }

  return 0;
}

SHELL_CMD(i2c, cmd_i2c);

#define SPI_CS GPIO(1, 2)
static int cmd_spi(int argc, char *argv[])
{
  unsigned char blank[] = { 0xff, 0xff };
  unsigned char rdata[2];
  unsigned int data;

  spi_write_read_buf(SPI_CS, blank, rdata, 2);

  data = (rdata[0] << 8) + rdata[1];

  /* remove msb - always 1 */
  data &= ~BIT(15);

  xprintf("%x %u\n", data, data);

  return 0;
}

SHELL_CMD(spi, cmd_spi);

xtime_ms_t spi_last;

static void spi_task_init(void *data)
{
}

static void spi_task_body(void *data)
{
  unsigned char blank[] = { 0xff, 0xff };
  unsigned char rdata[2];
  unsigned int enc_data;
  xtime_ms_t now;

  now = systick_count;

  if (now - spi_last < 20)
    return;

  spi_write_read_buf(SPI_CS, blank, rdata, 2);

  enc_data = (rdata[0] << 8) + rdata[1];

  xprintf("%u\n", enc_data);

  spi_last = now;
}


static unsigned int invalid_interrupts;

void __vector_default()
{
  xprintf("invalid interrupt - ignored\n");
  invalid_interrupts++;
}

void extint(unsigned char n, unsigned char type)
{
  unsigned char v;
  reg8_t *eicr;

  type &= 0x3;

  if (n >= 4) {
#if __AVR_ATmega328P__
    return;
#else
    n -= 4;
    eicr = &EICRB;
#endif
  } else
    eicr = &EICRA;

  v = *eicr;
  v &= ~(0x3 << (n << 1));
  v |= (type << (n << 1));
  *eicr = v;

  gpio_init(GPIO(3, n), GPIO_INPUT);

  EIMSK |= BIT(n);
}

typedef void task_init_t(void *data);
typedef void task_body_t(void *data);

typedef struct {
  task_init_t *init;
  task_body_t *body;
  void *data;
} polled_task_t;

static shell_t shell_task_data;

static void shell_task_init(void *data)
{
  shell_t *sh = (shell_t *)data;

  shell_init(sh, "> ");
}

static void shell_task_body(void *data)
{
  shell_t *sh = (shell_t *)data;
  int c = debug_getc();

  if (c >= 0)
    shell_input(sh, c);
}

#if 0
static void hd44780_write_byte(unsigned int b)
{
  unsigned char c = b;

  i2c_write_buf(0x27, &c, 1);
}

static void hd44780_delay()
{
  hal_delay_us(1000);
}

static hd44780_data_t disp_data = { hd44780_write_byte, hd44780_delay };
#endif

/* *INDENT-OFF* */
static polled_task_t tasks[] = {
  { shell_task_init, shell_task_body, (void *)&shell_task_data },
  { spi_task_init, spi_task_body, 0 },
};
/* *INDENT-ON* */

int main()
{
  unsigned int i;

  sei();

#if 0
  extint(0, 2);
  extint(1, 2);
#endif

  gpio_init(LED, GPIO_OUTPUT);

  timer_init();
  uart_init();
  i2c_init();
  spi_init(SPI_CS);

  xputs("\nBOOT\n\n");

#if 0
  hd44780_init(&disp_data);
  hd44780_write_str(&disp_data, 0, "Hello Brian");
  hd44780_write_str(&disp_data, 64, "Testing, Testing...");
#endif

  for (i = 0; i < ARRSIZ(tasks); i++) {
    polled_task_t *t = &tasks[i];
    t->init(t->data);
  }

  for (;;) {
    for (i = 0; i < ARRSIZ(tasks); i++) {
      polled_task_t *t = &tasks[i];
      t->body(t->data);
    }
  }

  return 0;
}
