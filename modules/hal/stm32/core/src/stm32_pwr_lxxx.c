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

#include "stm32_pwr_lxxx.h"
#include "hal_common.h"

void backup_domain_protect(int on)
{
  if (on)
    PWR->cr[0] &= ~PWR_CR1_DBP;
  else
    PWR->cr[0] |= PWR_CR1_DBP;
}

void vddio2_en(int on)
{
  if (on)
    PWR->cr[1] |= PWR_CR2_IOSV;
  else
    PWR->cr[1] &= ~PWR_CR2_IOSV;
}

#define PWR_SR2_VOSF BIT(10)

int stm32_pwr_vos_rdy(void)
{
  return (PWR->sr[1] & PWR_SR2_VOSF) == 0;
}

void stm32_pwr_vos(unsigned int vos)
{
  reg_set_field(&PWR->cr[0], 2, 9, vos);
}

/* SYSCFG */
typedef struct {
  reg32_t memrmp;
  reg32_t cfgr1;
  reg32_t exticr[4];
  reg32_t scsr;
  reg32_t cfgr2;
  reg32_t swpr;
  reg32_t skr;
} stm32_syscfg_t;

#define SYSCFG ((stm32_syscfg_t *)(0x40010000))

void stm32_syscfg_set_exti(unsigned int v, unsigned int n)
{
  unsigned int reg, ofs;

  if (n > 15)
    return;

  reg = n / 4;
  ofs = n % 4;

  reg_set_field(&SYSCFG->exticr[reg], 4, ofs << 2, v);
}
