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

#include <stdlib.h>

#include "common.h"
#include "hal_board.h"
#include "hal_common.h"
#include "hal_int.h"
#include "hal_time.h"
#include "hal_cpu.h"
#include "io.h"
#include "shell.h"
#include "stm32_timer.h"

/* should probably be more flexible here */
#if CONFIG_TIMER_16BIT
 #if STM32_L0XX
 #define TIM_BASE TIM2_BASE
 #else
 #define TIM_BASE TIM1_BASE
 #endif
#else
#define TIM_BASE TIM2_BASE
#endif

typedef struct {
  reg32_t cr1;
  reg32_t cr2;
  reg32_t smcr;
  reg32_t dier;
  reg32_t sr;
  reg32_t egr;
  reg32_t ccmr[2];
  reg32_t ccer;
  reg32_t cnt;
  reg32_t psc;
  reg32_t arr;
  reg32_t rcr;
  reg32_t ccr[4];
  reg32_t bdtr;
  reg32_t dcr;
  reg32_t dmar;
  reg32_t or[2];
} stm32_timer_t;

#define CR1_CEN BIT(0)
#define CR1_PMEN BIT(10) /* AT32 plus mode */

#define EGR_UG BIT(0)

#define BDTR_MOE BIT(15);

/* Change the timer prescaler value while preserving the timer value
   This is useful when dynamically adjusting the master clock for power
   saving so the timer can continue to count at the same rate even though
   its input clock is changed.
 */
static void timer_presc(void *base, unsigned int presc)
{
  stm32_timer_t *t = (stm32_timer_t *)base;
  unsigned int ocnt;

  t->cr1 &= ~CR1_CEN;
  /* save the current counter value */
  ocnt = t->cnt;
  t->psc = presc - 1;
  /* this is needed to load the new prescaler value but it also
     resets the counter value */
  t->egr = EGR_UG;
  t->cr1 = CR1_CEN;
  /* restore the the saved counter */
  t->cnt = ocnt;
}

void timer_init(void *base, unsigned int presc)
{
  stm32_timer_t *t = (stm32_timer_t *)base;

  t->cr1 &= ~CR1_CEN;
#if AT32_F4XX && !CONFIG_TIMER_16BIT
  t->cr1 |= CR1_PMEN;
#endif
  t->cnt = 0;
  t->arr = 0xffffffff;
  t->smcr = 0;
  t->psc = presc - 1;
  t->egr = EGR_UG;
  t->cr1 |= CR1_CEN;
}

void hal_delay_us(hal_time_us_t us)
{
  unsigned int now = hal_time_us();

  while (hal_time_us() - now < us)
    ;
}

#if CONFIG_TIMER_16BIT
static unsigned int hal_time_us_acc;
static unsigned int hal_time_us_last;

void hal_time_us_update(void)
{
  unsigned int saved, now;

  saved = interrupt_disable();

  now = timer_get(TIM_BASE);
  hal_time_us_acc += (unsigned int)(unsigned short)(now - hal_time_us_last);
  hal_time_us_last = now;

  interrupt_enable(saved);
}

hal_time_us_t hal_time_us(void)
{
  hal_time_us_update();

  return hal_time_us_acc;
}
#else
hal_time_us_t hal_time_us(void)
{
  return timer_get(TIM_BASE);
}
#endif

#if STM32_F767
#define TIM_DIV 2
#elif STM32_H7XX
#define TIM_DIV 2
#elif STM32_L4XX
#define TIM_DIV 1
#elif STM32_G4XX
#define TIM_DIV 1
#else
#define TIM_DIV 1
#endif

static unsigned int timer_calc_div(void)
{
  return (HAL_CPU_CLOCK / TIM_DIV  + (1000000 / 2 - 1)) / 1000000;
}

void hal_time_reinit(void)
{
  timer_presc(TIM_BASE, timer_calc_div());
}

/* set up timer2 as a us timer */
void hal_time_init(void)
{
#if STM32_H745NM4
#else
  timer_init(TIM_BASE, timer_calc_div());
#endif
}

unsigned int timer_get(void *base)
{
  stm32_timer_t *t = (stm32_timer_t *)base;

  return t->cnt;
}

int timer_init_compare(void *base, unsigned int presc, unsigned int max,
                       const unsigned int *compare, unsigned int compare_len)
{
  unsigned int i;
  stm32_timer_t *t = (stm32_timer_t *)base;

  if (compare_len > 4)
    return -1;

  t->cr1 &= ~CR1_CEN;
  t->cnt = 0;
  t->arr = max;
  t->smcr = 0;
  t->psc = presc - 1;
  t->egr = 0;

  for (i = 0; i < compare_len; i++)
    t->ccr[i] = compare[i];

  return 0;
}

void timer_set_compare(void *base, unsigned int num, unsigned int v)
{
  stm32_timer_t *t = (stm32_timer_t *)base;

  if (num >= 4)
    return;

  t->ccr[num] = v;
}

#define TIMER_MODE_PWM 6

int timer_mode(void *base, int output, unsigned int mode)
{
  stm32_timer_t *t = (stm32_timer_t *)base;
  unsigned int reg = 0;

  if (output > 4)
    return -1;

  if (output >= 2) {
    reg = 1;
    output -= 2;
  }

  reg_set_field(&t->ccmr[reg], 3, 8 * output + 4, mode);

  return 0;
}

int timer_output_en(void *base, int en)
{
  stm32_timer_t *t = (stm32_timer_t *)base;

  if (en)
    t->bdtr |= BIT(15);
  else
    t->bdtr &= ~BIT(15);

  return 0;
}

void timer_init_pwm(void *base, unsigned int presc, unsigned int max,
                    const unsigned int *compare, unsigned int compare_len)
{
  unsigned int ccer_val = 0, i;
  stm32_timer_t *t = (stm32_timer_t *)base;

  if (timer_init_compare(base, presc, max, compare, compare_len) < 0)
    return;

  for (i = 0; i < compare_len; i++) {
    ccer_val |= BIT(4 * i);
    timer_mode(base, i, TIMER_MODE_PWM);
  }

  t->ccer = ccer_val;
  t->bdtr |= BDTR_MOE;
  t->cr1 |= CR1_CEN;
}

void timer_init_dma(void *base, unsigned int presc, unsigned int max,
                    const unsigned int *compare, unsigned int compare_len,
                    int update_en)
{
  unsigned int ccer_val = 0, dier_val = 0, i;
  stm32_timer_t *t = (stm32_timer_t *)base;

  if (timer_init_compare(base, presc, max, compare, compare_len) < 0)
    return;

  t->cnt = max - 1;

  for (i = 0; i < compare_len; i++) {
    ccer_val |= BIT(4 * i);
    dier_val |= BIT(9 + i);
  }

  if (update_en)
    dier_val |= BIT(8);

  t->ccer = ccer_val;
  t->dier = dier_val;
  t->cr1 |= CR1_CEN;
}

void timer_enable(void *base)
{
  stm32_timer_t *t = (stm32_timer_t *)base;

  t->cr1 |= CR1_CEN;
}

void timer_stop(void *base)
{
  stm32_timer_t *t = (stm32_timer_t *)base;

  t->ccer = 0;
  t->dier = 0;
  t->cr1 &= ~CR1_CEN;
}

#if CONFIG_CMD_TIMER
#if STM32_H7XX
#define TIM_BASE TIM3_BASE
#else
#define TIM_BASE TIM1_BASE
#endif
int cmd_timer(int argc, char *argv[])
{
  unsigned int compare[] = { 10000, 20000, 30000 };

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'i':
    timer_init_pwm(TIM_BASE, 1, 65535, compare, ARRSIZ(compare));
    break;
  case 'c':
    if (argc < 3)
      return -1;
    timer_set_compare(TIM_BASE, 2, atoi(argv[2]));
    break;
  case 's':
    timer_stop(TIM_BASE);
    break;
  default:
  case 'g':
    xprintf("%u\n", timer_get(TIM_BASE));
    break;
  }

  return 0;
}

SHELL_CMD(timer, cmd_timer);
#endif
