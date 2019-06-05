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

#include "common.h"
#include "hal_time.h"
#include "io.h"
#include "shell.h"
#include "stm32_hal_board.h"
#include "stm32_timer.h"

typedef struct {
  unsigned int cr1;
  unsigned int cr2;
  unsigned int smcr;
  unsigned int dier;
  unsigned int sr;
  unsigned int egr;
  unsigned int ccmr[2];
  unsigned int ccer;
  unsigned int cnt;
  unsigned int psc;
  unsigned int arr;
  unsigned int pad0;
  unsigned int ccr[4];
  unsigned int bdtr;
  unsigned int dcr;
  unsigned int dmar;
  unsigned int or[2];
} stm32_timer_t;

#define CR1_CEN BIT(0)

#define EGR_UG BIT(0)

static void timer_presc(void *base, unsigned int presc)
{
  volatile stm32_timer_t *t = (volatile stm32_timer_t *)base;

  t->psc = presc - 1;
}

void timer_init(void *base, unsigned int presc)
{
  volatile stm32_timer_t *t = (volatile stm32_timer_t *)base;

  t->cr1 &= ~CR1_CEN;
  t->cnt = 0;
  t->arr = 0xffffffff;
  t->smcr = 0;
  t->psc = presc - 1;
  t->egr = EGR_UG;
  t->cr1 = CR1_CEN;
}

void hal_delay_us(unsigned int us)
{
  unsigned int now = timer_get(TIM2_BASE);

  while (timer_get(TIM2_BASE) - now < us)
    ;
}

unsigned int hal_time_us(void)
{
  return timer_get(TIM2_BASE);
}

#if STM32_F767
#define TIM2_DIV 2
#elif STM32_H7XX
#define TIM2_DIV 2
#elif STM32_L4XX
#define TIM2_DIV 1
#else
#define TIM2_DIV 1
#endif

static unsigned int timer_calc_div(void)
{
  return (CLOCK / TIM2_DIV  + (1000000 / 2 - 1)) / 1000000;
}

void hal_time_reinit(void)
{
  timer_presc(TIM2_BASE, timer_calc_div());
}

/* set up timer2 as a us timer */
void hal_time_init(void)
{
  timer_init(TIM2_BASE, timer_calc_div());
}

void timer1_init(void *base, unsigned int presc)
{
  volatile stm32_timer_t *t = (volatile stm32_timer_t *)base;

  t->cnt = 0;
  t->arr = (3831 - 1);
  t->smcr = 0;
  t->psc = presc - 1;

  t->ccr[3] = 3831 / 2;
  t->ccmr[1] = (6 << 12);
  t->egr = EGR_UG;
  t->ccer |= BIT(12);
  t->cr1 = CR1_CEN;
}

void timer_note(void *base, unsigned int hz)
{
  volatile stm32_timer_t *t = (volatile stm32_timer_t *)base;
  unsigned int per;

  if (hz == 0) {
    t->bdtr = 0;
    return;
  }

  t->bdtr |= BIT(15);

  per = (1000000 + hz / 2) / hz;

  t->arr = (per - 1);
  t->ccr[3] = per / 2;
}

unsigned int notes[] = {
  262,
  294,
  330,
  349,
  392,
  440,
  493,
  523
};

unsigned int timer_get(void *base)
{
  volatile stm32_timer_t *t = (volatile stm32_timer_t *)base;

  return t->cnt;
}

#if 0
int cmd_timer(int argc, char *argv[])
{
  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'i':
    timer_init(TIM2_BASE, 200);
    break;
  default:
  case 'g':
    xprintf("%u\n", timer_get(TIM2_BASE));
    break;
  case 'x':
    timer1_init(TIM1_BASE, 80);
    break;
  case 'y':
    xprintf("%u\n", timer_get(TIM1_BASE));
    break;
#if 0
  case 'p':
    for (unsigned int i = 0; i < ARRSIZ(notes); i++) {
      timer_note(TIM1_BASE, notes[i]);
      task_delay(300);
    }
    timer_note(TIM1_BASE, 0);
    break;
#endif
  }

  return 0;
}

SHELL_CMD(timer, cmd_timer);
#endif
