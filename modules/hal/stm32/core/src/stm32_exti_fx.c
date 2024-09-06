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
#include "stm32_exti.h"

typedef struct {
  reg32_t imr;
  reg32_t emr;
  reg32_t rtsr;
  reg32_t ftsr;
  reg32_t swier;
  reg32_t pr;
  reg32_t pad0[2];
} stm32_exti_t;

#ifdef STM32_F1XX
#define EXTI_BASE 0x40010400
#else
#define EXTI_BASE 0x40013c00
#endif

#define EXTI ((stm32_exti_t *)(EXTI_BASE))

void stm32_exti_irq_set_edge_rising(unsigned int n, int en)
{
  bit_en(&EXTI->rtsr, n, en);
}

void stm32_exti_irq_set_edge_falling(unsigned int n, int en)
{
  bit_en(&EXTI->ftsr, n, en);
}

void stm32_exti_irq_enable(unsigned int n, int en)
{
  bit_en(&EXTI->imr, n, en);
}

void stm32_exti_irq_ack(unsigned int n)
{
  if (n >= 32)
    return;

  EXTI->pr = BIT(n);
}

void stm32_exti_ev_enable(unsigned int n, int en)
{
  bit_en(&EXTI->emr, n, en);
}
