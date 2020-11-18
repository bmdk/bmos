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
#include "hal_common.h"
#include "stm32_pwr_h7xx.h"

#define PWR_BASE 0x58024800

typedef struct {
  unsigned int cr1;
  unsigned int csr1;
  unsigned int cr2;
  unsigned int cr3;
  unsigned int cpucr;
  unsigned int pad0;
  unsigned int d3cr;
  unsigned int wkupcr;
  unsigned int wkupfr;
  unsigned int wkupepr;
} stm32_pwr_h7xx_t;

#define PWR ((volatile stm32_pwr_h7xx_t *)PWR_BASE)

#define SYSCFG_BASE 0x58000400

typedef struct {
  unsigned int pad0;
  unsigned int pmcr;
  unsigned int exticr[4];
  unsigned int cfgr;
  unsigned int ccsr;
  unsigned int ccvr;
  unsigned int cccr;
  unsigned int pad1[63];
  unsigned int pkgr;
  unsigned int pad2[118];
  unsigned int ur[18];
} stm32_syscfg_h7xx_t;

#define SYSCFG ((volatile stm32_syscfg_h7xx_t *)SYSCFG_BASE)

#define PWR_CR1_DBP BIT(8)

void backup_domain_protect(int on)
{
  if (on)
    PWR->cr1 &= ~PWR_CR1_DBP;
  else
    PWR->cr1 |= PWR_CR1_DBP;
}

void stm32_syscfg_set_exti(unsigned int v, unsigned int n)
{
  unsigned int reg, ofs;

  if (n > 15)
    return;

  reg = n / 4;
  ofs = n % 4;

  reg_set_field(&SYSCFG->exticr[reg], 4, ofs << 2, v);
}

void stm32_syscfg_eth_phy(unsigned int val)
{
  unsigned int reg_val = 0;

  if (val == SYSCFG_ETH_PHY_RMII)
    reg_val = 4;

  reg_set_field(&SYSCFG->pmcr, 3, 21, reg_val); /* EPIS */
}

void stm32_pwr_wkup_en(unsigned int n, int en)
{
  if (n > 5)
    return;

  if (en)
    PWR->wkupepr |= BIT(n);
  else
    PWR->wkupepr &= ~BIT(n);
}

void stm32_pwr_power(unsigned int val)
{
  reg_set_field(&PWR->cr3, 6, 0, val);
}

#define PWR_D3CR_VOSRDY BIT(13)

void stm32_pwr_vos(unsigned int vos)
{
  reg_set_field(&PWR->d3cr, 2, 14, vos);

  while ((PWR->d3cr & PWR_D3CR_VOSRDY) == 0)
    ;
}
