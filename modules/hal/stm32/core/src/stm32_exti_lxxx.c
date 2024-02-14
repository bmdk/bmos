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

#include "common.h"
#include "hal_common.h"
#include "stm32_pwr.h"

typedef struct {
  struct {
    reg32_t imr;
    reg32_t emr;
    reg32_t rtsr;
    reg32_t ftsr;
    reg32_t swier;
    reg32_t pr;
    reg32_t pad0[2];
  } r[2];
} stm32_exti_t;

#if STM32_WBXX
#define EXTI_BASE 0x58000800
#else
#define EXTI_BASE 0x40010400
#endif

#define EXTI ((stm32_exti_t *)EXTI_BASE)

void stm32_exti_irq_set_edge_rising(unsigned int n, int en)
{
  unsigned int reg = n / 32;

  n %= 32;

  if (reg > 1)
    return;

  bit_en(&EXTI->r[reg].rtsr, n, en);
}

void stm32_exti_irq_set_edge_falling(unsigned int n, int en)
{
  unsigned int reg = n / 32;

  n %= 32;

  if (reg > 1)
    return;

  bit_en(&EXTI->r[reg].ftsr, n, en);
}

void stm32_exti_irq_enable(unsigned int n, int en)
{
  unsigned int reg = n / 32;

  n %= 32;

  if (reg > 1)
    return;

  bit_en(&EXTI->r[reg].imr, n, en);
}

void stm32_exti_irq_ack(unsigned int n)
{
  unsigned int reg = n / 32;

  n %= 32;

  if (reg > 1)
    return;

  EXTI->r[reg].pr = BIT(n);
}

unsigned int stm32_exti_irq_status(unsigned int n)
{
  unsigned int reg = n / 32;

  n %= 32;

  if (reg > 1)
    return 0;

  return (EXTI->r[reg].pr >> n) & 1;
}

void stm32_exti_ev_enable(unsigned int n, int en)
{
  unsigned int reg = n / 32;

  n %= 32;

  if (reg > 1)
    return;

  bit_en(&EXTI->r[reg].emr, n, en);
}
