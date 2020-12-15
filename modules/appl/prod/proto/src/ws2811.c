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

#include "bmos_task.h"
#include "common.h"
#include "fast_log.h"
#include "hal_int.h"
#include "stm32_hal_bdma.h"
#include "stm32_hal_gpio.h"
#include "stm32_timer.h"

#define STM32_GPIO_ADDR_SET_CLEAR(port) (unsigned int)(&STM32_GPIO(port)->bsrr)
#define GPIO_ADDR STM32_GPIO_ADDR_SET_CLEAR(0)

static unsigned char one = 0xff;

#define PIXELS 250
static unsigned char buf[24 * PIXELS];

static void ws2811_tx()
{
  unsigned int compare[2] = { 27, 55 };

  FAST_LOG('W', "ws2811 tx start\n", 0, 0);

  timer_stop(TIM1_BASE);

  stm32_bdma_set_chan(BDMA1_BASE, 5, 7); /* TIM1 UP */
  stm32_bdma_set_chan(BDMA1_BASE, 1, 7); /* TIM1 CH1 */
  stm32_bdma_set_chan(BDMA1_BASE, 2, 7); /* TIM1 CH2 */

  stm32_bdma_trans(BDMA1_BASE, 5, &one, (void *)(GPIO_ADDR), 24 * PIXELS,
                   CCR_PL(0) | CCR_MSIZ(0) | CCR_PSIZ(0));

  stm32_bdma_trans(BDMA1_BASE, 1, buf, (void *)(GPIO_ADDR + 2), 24 * PIXELS,
                   CCR_PL(0) | CCR_MSIZ(0) | CCR_PSIZ(0) | CCR_PINC | CCR_TCIE);

  stm32_bdma_trans(BDMA1_BASE, 2, &one, (void *)(GPIO_ADDR + 2), 24 * PIXELS,
                   CCR_PL(0) | CCR_MSIZ(0) | CCR_PSIZ(0));

  stm32_bdma_en(BDMA1_BASE, 5, 1);
  stm32_bdma_en(BDMA1_BASE, 1, 1);
  stm32_bdma_en(BDMA1_BASE, 2, 1);

  timer_init_dma(TIM1_BASE, 1, 99, compare, ARRSIZ(compare), 1);
}

#define MASK 0x01010101

static inline void wrint(void *a, unsigned int v, unsigned int string)
{
  unsigned int mask = MASK << string;
  unsigned int val = *(unsigned int *)a;

  val &= ~mask;
  val |= v << string;

  *(unsigned int *)a = val ^ mask;
}

void enc_col(unsigned int pix, unsigned int val, unsigned int string)
{
  unsigned int i;
  unsigned int idx = pix * 24;

  for (i = 0; i < 24; i += 4) {
    unsigned int _val = val >> (20 - i);
    void *a = &buf[idx + i];
    unsigned int v;

    v = ((((_val >> 3) & 1) << 0) +
         (((_val >> 2) & 1) << 8) +
         (((_val >> 1) & 1) << 16) +
         (((_val >> 0) & 1) << 24));

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
  stm32_bdma_irq_ack(BDMA1_BASE, 1, IER_TCIF);
  FAST_LOG('W', "irq_ws2811\n", 0, 0);
}

void task_led(void *arg)
{
  unsigned int col[] = { 0x808080, 0xff0000 };
  unsigned int k = 0, l = 0, last_k = -1;

  irq_register("ws2811", irq_ws2811, 0, 12);

  for (;;) {
    unsigned int i, j, o;

    for (j = 0; j < 256; j++) {

      l++;
      if (l >= 64) {
        k++;
        l = 0;
      }

      for (i = 0; i < PIXELS; i++) {
        o = i + j;
        enc_col(i, scale(wheel(o & 255), 10), 0);
      }

      if (last_k != k) {
        for (i = 0; i < PIXELS; i++) {
          o = (k + i) % ARRSIZ(col);
          enc_col(i, scale(col[o], 10), 1);
        }
        last_k = k;
      }

      ws2811_tx();
      task_delay(50);
    }
  }
#if 0
#if 1
  /* red + white */
  unsigned int col[] = { 0x808080, 0xff0000 };
#else
  /* green, white and gold */
  unsigned int col[] = { 0xffd700, 0x808080, 0x008000 };
#endif
  for (;;) {
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
#endif
#if 0
  for (;;) {
    unsigned int i, j, k;

    for (j = 0; j < PIXELS; j++)
      enc_col(j, 0);

    ws2811_tx();
    task_delay(20);

    for (i = 0; i < PIXELS; i++) {
      for (j = 0; j < PIXELS - i - 1; j++) {
        for (k = 0; k < PIXELS - i - 1; k++)
          enc_col(k, 0);
        enc_col(j, scale(0xff0000, 2));

        ws2811_tx();
        task_delay(20);
      }
    }
  }
#endif
}
