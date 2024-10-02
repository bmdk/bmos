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

#if STM32_G4XX
#define CONFIG_TIMER_EXTENDED 1
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
#if STM32_G4XX
  reg32_t ccr5;
  reg32_t ccr6;
  reg32_t ccmr3;
  reg32_t dtr2;
  reg32_t ecr;
  reg32_t tisel;
  reg32_t af[2];
  reg32_t pad1[221];
  reg32_t dcr;
  reg32_t dmar;
#else
  reg32_t dcr;
  reg32_t dmar;
  reg32_t or1;
  reg32_t ccmr3;
  reg32_t ccr5;
  reg32_t ccr6;
  reg32_t or2;
  reg32_t or3;
  reg32_t tisel;
#endif
} stm32_timer_t;

#define CR1_CEN BIT(0)
#define CR1_DIR BIT(4)
#define CR1_PMEN BIT(10) /* AT32 plus mode */

#define DIER_UIE BIT(0)
#define DIER_CCIE(n) BIT(n + 1)
#define DIER_CC1IE DIER_CCIE(0)
#define DIER_CC2IE DIER_CCIE(1)
#define DIER_CC3IE DIER_CCIE(2)
#define DIER_CC4IE DIER_CCIE(3)
#define DIER_COMIE BIT(5)
#define DIER_TIE BIT(6)
#define DIER_BIE BIT(7)
#define DIER_UDE BIT(8)
#define DIER_CCDE(n) BIT(n + 9)
#define DIER_CC1DE DIER_CCDE(0)
#define DIER_CC2DE DIER_CCDE(1)
#define DIER_CC3DE DIER_CCDE(2)
#define DIER_CC4DE DIER_CCDE(3)
#define DIER_COMDE BIT(13)
#define DIER_TDE BIT(14)


#define CCER_CC1E BIT(0)
#define CCER_CC1P BIT(1)
#define CCER_CC1NE BIT(2)
#define CCER_CC1NP BIT(3)

#define CCER_CC2E BIT(4)
#define CCER_CC2P BIT(5)
#define CCER_CC2NE BIT(6)
#define CCER_CC2NP BIT(7)

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

static void _timer_mms(stm32_timer_t *t, unsigned int mode)
{
  /* the 4 bits are split the msb is at bit 25 and the rest at bit offset 4 */
  reg_set_field(&t->cr2, 1, 25, mode >> 3);
  reg_set_field(&t->cr2, 3, 4, mode);
}

void timer_mms(void *base, unsigned int mode)
{
  stm32_timer_t *t = (stm32_timer_t *)base;

  _timer_mms(t, mode);
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

unsigned int timer_get_clear_status(void *base)
{
  stm32_timer_t *t = (stm32_timer_t *)base;
  unsigned int status = t->sr;

  t->sr = 0;

  return status;
}

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
#if BOARD_H745NM4
#else
  timer_init(TIM_BASE, timer_calc_div());
#endif
}

unsigned int timer_get(void *base)
{
  stm32_timer_t *t = (stm32_timer_t *)base;

  return t->cnt;
}

unsigned int timer_get_cap(void *base, unsigned int unit)
{
  stm32_timer_t *t = (stm32_timer_t *)base;

  if (unit > 4)
    return 0;

  return t->ccr[unit];
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

static void timer_set_trigger(stm32_timer_t *t, unsigned int trigger)
{
  reg_set_field(&t->smcr, 2, 20, (trigger >> 3)); /* MSb */
  reg_set_field(&t->smcr, 3, 4, trigger);
}

static void timer_set_slave_mode(stm32_timer_t *t, unsigned int mode)
{
  reg_set_field(&t->smcr, 1, 16, (mode >> 3)); /* MSb */
  reg_set_field(&t->smcr, 3, 0, mode);
}

#define CCMR_CCXS_OUT 0
#define CCMR_CCXS_IN_DIRECT 1
#define CCMR_CCXS_IN_SWAP 2
#define CCMR_CCXS_IN_TRC 3

void timer_init_encoder(void *base, unsigned int presc,
                        unsigned int max, unsigned int filter)
{
  stm32_timer_t *t = (stm32_timer_t *)base;
  unsigned int map;

  t->cr1 &= ~CR1_CEN;

  t->cnt = 0;

  if (1)
    /* map input pin 1 to input 2 and input pin 2 to input 1 */
    map = CCMR_CCXS_IN_SWAP;
  else
    /* map input pin 1 to input 1 and input pin 2 to input 2 */
    map = CCMR_CCXS_IN_DIRECT;

  reg_set_field(&t->ccmr[0], 2, 0, map); /* CC1S = 1 */
  reg_set_field(&t->ccmr[0], 2, 8, map); /* CC2S = 1 */

  /* set event counter prescaler to 1 */
  reg_set_field(&t->ccmr[0], 2, 2, 0);
  reg_set_field(&t->ccmr[0], 2, 10, 0);

  reg_set_field(&t->ccmr[0], 4, 4, filter); /* IC1F */
  reg_set_field(&t->ccmr[0], 4, 12, filter); /* IC2F */

  t->ccer &= ~(CCER_CC1P | CCER_CC1NP | CCER_CC2P | CCER_CC2NP);
  timer_set_slave_mode(t, 3); /* quadrature encoder x4 */
  t->arr = max;
  t->psc = presc - 1;

  t->cr1 |= CR1_CEN;
}

void timer_init_capture(void *base, unsigned int presc, unsigned int trig_src)
{
  stm32_timer_t *t = (stm32_timer_t *)base;

  t->cr1 &= ~CR1_CEN;

  t->cnt = 0;

  /* compare1 from tim_trc */
  reg_set_field(&t->ccmr[0], 2, 0, CCMR_CCXS_IN_TRC);
  reg_set_field(&t->ccmr[0], 2, 8, CCMR_CCXS_OUT);

  /* set event counter prescaler to 1 */
  reg_set_field(&t->ccmr[0], 2, 2, 0);
  reg_set_field(&t->ccmr[0], 2, 10, 0);

#define FILTER_LEN 0
  reg_set_field(&t->ccmr[0], 4, 4, FILTER_LEN); /* IC1F */
  reg_set_field(&t->ccmr[0], 4, 12, FILTER_LEN); /* IC2F */

  t->ccer &= ~(CCER_CC1P | CCER_CC1NP | CCER_CC2P | CCER_CC2NP);
  t->ccer |= CCER_CC1E;

  timer_set_trigger(t, trig_src);
  timer_set_slave_mode(t, 0);
  t->arr = 0xffff;
  t->psc = presc - 1;

  t->dier = DIER_CC1IE;

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
#define TIM_CMD_BASE TIM3_BASE
#else
#define TIM_CMD_BASE TIM1_BASE
#endif
int cmd_timer(int argc, char *argv[])
{
  unsigned int compare[] = { 10000, 20000, 30000 };

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'i':
    timer_init_pwm(TIM_CMD_BASE, 1, 65535, compare, ARRSIZ(compare));
    break;
  case 'c':
    if (argc < 3)
      return -1;
    timer_set_compare(TIM_CMD_BASE, 2, atoi(argv[2]));
    break;
  case 's':
    timer_stop(TIM_CMD_BASE);
    break;
  case 'e':
    timer_init_encoder(TIM_CMD_BASE, 1, -1, 0xf);
    break;
  default:
  case 'g':
    xprintf("%u\n", timer_get(TIM_CMD_BASE));
    break;
  }

  return 0;
}

SHELL_CMD(timer, cmd_timer);
#endif
