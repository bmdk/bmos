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

/* This implements control of WS2811 LEDs using the STM32 timer 1 and dma.
   It works on L4XX(BDMA) and F4XX(DMA). The naming of BDMA, DMA and MDMA
   come from H7XX where all three types of DMA controllers are present. It
   can control a whole bank of GPIOs simultaneously - that is 16 - bit to save
   memory right now only the first 8 in the bank will work out of the box here.
   I had an ambition to make a video wall at some point but that is not yet :).
 */
#include <string.h>

#include "bmos_task.h"
#include "common.h"
#include "fast_log.h"
#include "fb.h"
#include "hal_board.h"
#include "hal_dma.h"
#include "hal_int.h"
#include "io.h"
#include "hal_gpio.h"
#include "stm32_hal_gpio.h"
#include "stm32_timer.h"

#define WSPERIOD_NS 1250
#define TIMER_CLOCK hal_cpu_clock

/* Number of clocks in 1250 ns - the nominal one bit period for WS281X */
#define WSCLOCKS ((TIMER_CLOCK / 1000) * WSPERIOD_NS + 1000000 / 2) / 1000000

#define WCPCCLOCKS(period_ns) \
  (WSCLOCKS * (period_ns) + WSPERIOD_NS / 2) / WSPERIOD_NS

/* this has nothing to do with the DMA type so it should probably be it's own
   configuration parameter(s) in the future */
#if STM32_F411BP || STM32_F401BP

/* the bit in the bank that is used */
#define WSBIT 0
#define WSIRQ 57
/* the gpio bank (B) */
#define WSGPIO 1
#define DMANUM 1

#define CHAN_TIM1_UP 5
#define CHAN_TIM1_CH1 1
#define CHAN_TIM1_CH2 2

#define DEVID_TIM1_UP 6
#define DEVID_TIM1_CH1 6
#define DEVID_TIM1_CH2 6

#elif STM32_U575N

#define WSBIT 0
#define WSIRQ 31 /* GPDMA1_CH2 */
#define WSGPIO 0
#define DMANUM 0

#define CHAN_TIM1_UP 1
#define CHAN_TIM1_CH1 2
#define CHAN_TIM1_CH2 3

#define DEVID_TIM1_CH1 42
#define DEVID_TIM1_CH2 43
#define DEVID_TIM1_UP 46

#else

#define WSBIT 0
#define WSIRQ 12
#define WSGPIO 0
#define DMANUM 0

#define CHAN_TIM1_UP 5
#define CHAN_TIM1_CH1 1
#define CHAN_TIM1_CH2 2

#define DEVID_TIM1_UP 6
#define DEVID_TIM1_CH1 6
#define DEVID_TIM1_CH2 6

#endif

#define STM32_GPIO_ADDR_SET_CLEAR(port) (unsigned int)(&STM32_GPIO(port)->bsrr)
#define GPIO_ADDR STM32_GPIO_ADDR_SET_CLEAR(WSGPIO)

static unsigned char one = BIT(WSBIT);
static unsigned int compare[2];

#define PIXELS 300
static unsigned char buf[24 * PIXELS];

static void ws2811_tx()
{
  dma_attr_t attr;

  FAST_LOG('W', "ws2811 tx start\n", 0, 0);

  timer_stop(TIM1_BASE);

  dma_set_chan(DMANUM, CHAN_TIM1_UP, DEVID_TIM1_UP);
  dma_set_chan(DMANUM, CHAN_TIM1_CH1, DEVID_TIM1_CH1);
  dma_set_chan(DMANUM, CHAN_TIM1_CH2, DEVID_TIM1_CH2);

  attr.ssiz = DMA_SIZ_1;
  attr.dsiz = DMA_SIZ_1;
  attr.dir = DMA_DIR_TO;
  attr.prio = 0;
  attr.sinc = 0;
  attr.dinc = 0;
  attr.irq = 0;

  dma_trans(DMANUM, CHAN_TIM1_UP, &one,
            (void *)(GPIO_ADDR), 24 * PIXELS, attr);

  attr.sinc = 1;
  attr.irq = 1;

  dma_trans(DMANUM, CHAN_TIM1_CH1, buf,
            (void *)(GPIO_ADDR + 2), 24 * PIXELS, attr);

  attr.sinc = 0;
  attr.irq = 0;

  dma_trans(DMANUM, CHAN_TIM1_CH2, &one,
            (void *)(GPIO_ADDR + 2), 24 * PIXELS, attr);

  dma_en(DMANUM, CHAN_TIM1_UP, 1);
  dma_en(DMANUM, CHAN_TIM1_CH1, 1);
  dma_en(DMANUM, CHAN_TIM1_CH2, 1);

  timer_init_dma(TIM1_BASE, 1, WSCLOCKS - 1, compare, ARRSIZ(compare), 1);
}

#define MASK 0x01010101

static inline void wrint(unsigned int *a, unsigned int v, unsigned int string)
{
  *a = (*a & ~(MASK << string)) | ((v ^ MASK) << string);
}

void enc_col(unsigned int pix, unsigned int val, unsigned int string)
{
  unsigned int i;
  unsigned int idx = pix * 24;

  for (i = 0; i < 24; i += 4) {
    unsigned int _val = (val >> (20 - i)) & 0xf;
    unsigned int v;
    void *a;

    v = ((_val >> 3) + (_val << 6) + (_val << 15) + (_val << 24)) & MASK;

#if WS2811_STRING_GRB
    if (i < 8)
      a = &buf[idx + i + 8];
    else if (i < 16)
      a = &buf[idx + i - 8];
    else
      a = &buf[idx + i];
#else
    a = &buf[idx + i];
#endif

    wrint(a, v, string);
  }
}

unsigned int wheel(unsigned int pos)
{
  unsigned int val;

  if (pos > 255)
    val = 0;
  else if (pos < 85)
    val = ((pos * 3) << 16) + ((255 - pos * 3) << 8);
  else if (pos < 170) {
    pos -= 85;
    val = ((255 - pos * 3) << 16) + pos * 3;
  } else {
    pos -= 170;
    val = ((pos * 3) << 8) + (255 - pos * 3);
  }

  return val;
}

static unsigned int _scale(unsigned int v, unsigned int s)
{
  return (v + s / 2) / s;
}

#define BYTE(_v_, _n_) (((_v_) >> ((_n_) << 3)) & 0xff)

unsigned int scale(unsigned int v, unsigned int s)
{
  return (_scale(BYTE(v, 2), s) << 16) +
         (_scale(BYTE(v, 1), s) << 8) +
         (_scale(BYTE(v, 0), s) << 0);
}

void irq_ws2811(void *data)
{
  dma_irq_ack(DMANUM, CHAN_TIM1_CH1);
  FAST_LOG('W', "irq_ws2811\n", 0, 0);
}

static void compare_init()
{
  compare[0] = WCPCCLOCKS(350);
  compare[1] = WCPCCLOCKS(700);
}

#if WS2811_DISPLAY_32X8_CLOCK
#include <stdio.h>

#include "hal_rtc.h"

extern const char font1[];

unsigned int colours[] =
{ 0x010000, 0x010100, 0x000100, 0x000101, 0x000001, 0x010101, 0x010000,
  0x010100 };

void disp_char(fb_t *fb, int x, int y, char c)
{
  const char *b = &font1[2 + c * 8];
  int i;

  for (i = 0; i < 8; i++) {
    unsigned ch = *(b + i);
    unsigned int j;

    for (j = 0; j < 8; j++) {
      unsigned int v = (ch >> (8 - 1 - j)) & 1;

      if (v)
        fb_draw(fb, x + j, y + i, colours[i]);
    }
  }
}

void disp_str(fb_t *fb, int x, int y, char *s, unsigned int slen)
{
  unsigned int k;

  for (k = 0; k < slen; k++)
    disp_char(fb, k * 8 + x, y, s[k]);
}

void fb_to_ws2811disp(fb_t *fb)
{
  unsigned int x, y, h, w;
  unsigned int *data;

  w = fb_width(fb);
  h = fb_height(fb);
  data = fb_get(fb);

  for (x = 0; x < w; x++) {
    for (y = 0; y < h; y++) {
      if (x % 2 == 0)
        enc_col(x * h + y, data[x + y * w], WSBIT);
      else
        enc_col(x * h + (h - 1 - y), data[x + y * w], WSBIT);
    }
  }

  ws2811_tx();
}

/* *INDENT-OFF* */
static const char *dayname[] = {
 "THINGY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY",
 "FRIDAY", "SATURDAY", "SUNDAY"
};
/* *INDENT-ON* */

static const char *get_dayname(unsigned int daynum)
{
  if (daynum > ARRSIZ(dayname))
    daynum = 0;

  return dayname[daynum];
}

void task_led(void *arg)
{
  unsigned int w = 32, h = 8;
  char message[64];

  compare_init();

  irq_register("ws2811", irq_ws2811, 0, WSIRQ);

  gpio_init(GPIO(WSGPIO, WSBIT), GPIO_OUTPUT);

  fb_t *fb = fb_init(w, h, 24, 0);

  for (;;) {
    int i, len, ofs;
    rtc_time_t t;

    rtc_get_time(&t);

    len = snprintf(message, sizeof(message), "%d%02d", t.hours, t.mins);
    fb_clear(fb);

    if (len == 3)
      ofs = 4;
    else
      ofs = 0;

    disp_str(fb, ofs, 0, message, len);

    fb_to_ws2811disp(fb);

    task_delay(2000);

    rtc_get_time(&t);
    if (t.hours <= 4 || t.hours >= 22)
      snprintf(message, sizeof(message), "GOOD NIGHT");
    else
      snprintf(message, sizeof(message), "HAPPY %s", get_dayname(t.dayno));

    len = strlen(message);

    for (i = (int)(w); i >= -8 * len; i--) {
      fb_clear(fb);

      disp_str(fb, i, 0, message, len);

      fb_to_ws2811disp(fb);

      task_delay(100);
    }
  }
}
#else
static void body_rainbow(void)
{
  unsigned int i, j, o;

  for (j = 0; j < 256; j++) {

    FAST_LOG('w', "ws2811 start\n", 0, 0);

    for (i = 0; i < PIXELS; i++) {
      o = i + j;
      enc_col(i, scale(wheel(o & 255), 32), WSBIT);
    }

    FAST_LOG('w', "ws2811 end\n", 0, 0);

    ws2811_tx();
    task_delay(50);
  }
}

#if 1
#define COLS 0x808080, 0xff0000           /* red + white */
#else
#define COLS 0xffd700, 0x808080, 0x008000 /* green, white and gold */
#endif

static void body_colseq(void)
{
  static unsigned int col[] = { COLS };
  unsigned int i, j, o;

  for (j = 0; j < PIXELS; j++) {
    for (i = 0; i < PIXELS; i++) {
      o = (j + i) % ARRSIZ(col);
      enc_col(i, scale(col[o], 10), 1);
    }

    ws2811_tx();
    task_delay(2000);
  }
}

void task_led(void *arg)
{
  compare_init();

  irq_register("ws2811", irq_ws2811, 0, WSIRQ);

  gpio_init(GPIO(WSGPIO, WSBIT), GPIO_OUTPUT);

  for (;;) {
    if (1)
      body_rainbow();
    else
      body_colseq();
  }
}
#endif
