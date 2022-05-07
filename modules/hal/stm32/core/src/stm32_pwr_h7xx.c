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

#define SYSCFG_N_UR 18

typedef struct {
  unsigned int pad0;
  unsigned int pmcr;
  unsigned int exticr[4];
  unsigned int cfgr;
  unsigned int ccsr;
  unsigned int ccvr;
  unsigned int cccr;
  unsigned int pwrcr;
  unsigned int pad1[62];
  unsigned int pkgr;
  unsigned int pad2[118];
  unsigned int ur[SYSCFG_N_UR];
} stm32_syscfg_h7xx_t;

#define SYSCFG ((volatile stm32_syscfg_h7xx_t *)SYSCFG_BASE)

#define SYSCFG_PWRCR_ODEN BIT(0)

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

#define PWR_CR3_USB33DEN BIT(24)
#define PWR_CR3_USBREGEN BIT(25)

void stm32_pwr_usbvdetect(int en)
{
  if (en)
    PWR->cr3 |= PWR_CR3_USB33DEN;
  else
    PWR->cr3 &= ~PWR_CR3_USB33DEN;
}

void stm32_pwr_usbreg(int en)
{
  if (en)
    PWR->cr3 |= PWR_CR3_USBREGEN;
  else
    PWR->cr3 &= ~PWR_CR3_USBREGEN;
}

#define PWR_D3CR_VOSRDY BIT(13)

int stm32_pwr_vos_rdy()
{
  return (PWR->d3cr & PWR_D3CR_VOSRDY) != 0;
}

static void _stm32_pwr_vos(unsigned int vos)
{
  reg_set_field(&PWR->d3cr, 2, 14, vos);

  while (!stm32_pwr_vos_rdy())
    ;
}

void stm32_pwr_vos(unsigned int vos)
{
#if STM32_VOS_HACK
  if (vos == PWR_VOS_SCALE_0) {
    _stm32_pwr_vos(PWR_VOS_SCALE_1);
    SYSCFG->pwrcr |= SYSCFG_PWRCR_ODEN;

    while (!stm32_pwr_vos_rdy())
      ;
  } else {
    SYSCFG->pwrcr &= ~SYSCFG_PWRCR_ODEN;
    _stm32_pwr_vos(vos);
  }
#else
  _stm32_pwr_vos(vos);
#endif
}

unsigned int stm32_ur_get(unsigned int idx)
{
  if (idx > SYSCFG_N_UR)
    return 0;

  return SYSCFG->ur[idx];
}

/* this enables and disables the clock to the cortex m4.
   this method of clock control has the advantage that we can stop
   the cortex m4 at any point - in particular if we gate the cpu clock
   before rebooting, after boot the cpu waits to have the clock enabled
   again */
void stm32_m4_en(int en)
{
  if (en)
    SYSCFG->ur[1] |= 1;
  else
    SYSCFG->ur[1] &= ~1;
}
