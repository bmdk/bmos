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

#include "ws2811_dma.h"

#define WSPERIOD_NS 1250
#define TIMER_CLOCK hal_cpu_clock

/* Number of clocks in 1250 ns - the nominal one bit period for WS281X */
#define WSCLOCKS ((TIMER_CLOCK / 1000) * WSPERIOD_NS + 1000000 / 2) / 1000000

#define WCPCCLOCKS(period_ns) \
  (WSCLOCKS * (period_ns) + WSPERIOD_NS / 2) / WSPERIOD_NS

#define STM32_GPIO_ADDR_SET(port) (unsigned char *)(&STM32_GPIO(port)->bsrr)
#if STM32_F1XX
#define STM32_GPIO_ADDR_CLEAR(port) (unsigned char *)(&STM32_GPIO(port)->brr)
#else
#define STM32_GPIO_ADDR_CLEAR(port) (STM32_GPIO_ADDR_SET(port) + 2)
#endif

void irq_ws2811(void *data)
{
  ws2811_dma_t *w = (ws2811_dma_t *)data;

  dma_irq_ack(w->dmanum, w->chan_tim_ch1);
  FAST_LOG('W', "irq_ws2811\n", 0, 0);
}

void ws2811_dma_init(ws2811_dma_t *w, unsigned int pixels)
{
  w->one = BIT(w->wsbit);
  w->compare[0] = WCPCCLOCKS(350);
  w->compare[1] = WCPCCLOCKS(700);
  w->buflen = 24 * pixels;
  w->gpio_addr_set = (void *)STM32_GPIO_ADDR_SET(w->wsgpio);
  w->gpio_addr_clear = (void *)STM32_GPIO_ADDR_CLEAR(w->wsgpio);

  irq_register("ws2811", irq_ws2811, w, w->wsirq);

  gpio_init(GPIO(w->wsgpio, w->wsbit), GPIO_OUTPUT);
}

void ws2811_dma_tx(ws2811_dma_t *w, unsigned char *buf)
{
  dma_attr_t attr;

  FAST_LOG('W', "ws2811 tx start\n", 0, 0);

  timer_stop(TIM1_BASE);

#if !STM32_F1XX
  dma_set_chan(w->dmanum, w->chan_tim_up, w->devid_tim_up);
  dma_set_chan(w->dmanum, w->chan_tim_ch1, w->devid_tim_ch1);
  dma_set_chan(w->dmanum, w->chan_tim_ch2, w->devid_tim_ch2);
#endif

  attr.ssiz = DMA_SIZ_1;
#if STM32_F1XX
  /* only word (32 bit) writes are allowed to gpio registers on f1xx.
     This pads the byte with zeros out to 32 bits. */
  attr.dsiz = DMA_SIZ_4;
#else
  attr.dsiz = DMA_SIZ_1;
#endif
  attr.dir = DMA_DIR_TO;
  attr.prio = 0;
  attr.sinc = 0;
  attr.dinc = 0;
  attr.irq = 0;

  dma_trans(w->dmanum, w->chan_tim_up, &w->one,
            w->gpio_addr_set, w->buflen, attr);

  attr.sinc = 1;
  attr.irq = 1;

  dma_trans(w->dmanum, w->chan_tim_ch1, buf,
            w->gpio_addr_clear, w->buflen, attr);

  attr.sinc = 0;
  attr.irq = 0;

  dma_trans(w->dmanum, w->chan_tim_ch2, &w->one,
            w->gpio_addr_clear, w->buflen, attr);

  dma_en(w->dmanum, w->chan_tim_up, 1);
  dma_en(w->dmanum, w->chan_tim_ch1, 1);
  dma_en(w->dmanum, w->chan_tim_ch2, 1);

  timer_init_dma(TIM1_BASE, 1, WSCLOCKS - 1, w->compare, 2, 1);
}

#define MASK 0x01010101

static inline void wrint(unsigned int *a, unsigned int v, unsigned int string)
{
  *a = (*a & ~(MASK << string)) | ((v ^ MASK) << string);
}

void ws2811_dma_enc_col(unsigned char *buf, unsigned int pix,
                        unsigned int val, unsigned int string)
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

static unsigned int _scale(unsigned int v, unsigned int s)
{
  return (v + s / 2) / s;
}

#define BYTE(_v_, _n_) (((_v_) >> ((_n_) << 3)) & 0xff)

unsigned int ws2811_dma_scale(unsigned int v, unsigned int s)
{
  return (_scale(BYTE(v, 2), s) << 16) +
         (_scale(BYTE(v, 1), s) << 8) +
         (_scale(BYTE(v, 0), s) << 0);
}
