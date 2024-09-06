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

#include "hal_common.h"

/* similar to l4xx */
typedef struct {
  reg32_t cr[4];
  reg32_t sr[2];
  reg32_t scr;
  reg32_t pad0;
  struct {
    reg32_t d;
    reg32_t u;
  } p[8];
} stm32_pwr_t;

#define PWR ((stm32_pwr_t *)(0x40007000))

#define PWR_CR1_DBP BIT(8)

void backup_domain_protect(int on)
{
  if (on)
    PWR->cr[0] &= ~PWR_CR1_DBP;
  else
    PWR->cr[0] |= PWR_CR1_DBP;
}

#define PWR_SR2_VOSF BIT(10)

int stm32_pwr_vos_rdy(void)
{
  return (PWR->sr[1] & PWR_SR2_VOSF) == 0;
}

void stm32_pwr_vos(unsigned int vos)
{
  reg_set_field(&PWR->cr[0], 2, 9, vos);

  while (!stm32_pwr_vos_rdy())
    ;
}
