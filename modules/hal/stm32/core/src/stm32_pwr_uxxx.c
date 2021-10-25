/* Copyright (c) 2019-2021 Brian Thomas Murphy
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
  reg32_t cr[3];
  reg32_t vosr;
  reg32_t svmcr;
  reg32_t wucr[3];
  reg32_t bdcr[2];
  reg32_t dbpr;
  reg32_t ucpdr;
  reg32_t seccfgr;
  reg32_t privcfgr;
  reg32_t sr;
  reg32_t svmsr;
  reg32_t bdsr;
  reg32_t wusr;
  reg32_t wuscr;
  reg32_t apcr;
  struct {
    reg32_t ucr;
    reg32_t dcr;
  } p[9];
} stm32_pwr_uxxx_t;

#define PWR_BASE 0x46020800
#define PWR ((stm32_pwr_uxxx_t *)PWR_BASE)

typedef struct {
  reg32_t seccfgr;
  reg32_t cfgr1;
  reg32_t fpuimr;
  reg32_t cnslckr;
  reg32_t cslockr;
  reg32_t cfgr2;
  reg32_t mesr;
  reg32_t cccsr;
  reg32_t ccvr;
  reg32_t cccr;
  unsigned int pad0;
  reg32_t rsscmdr;
} stm32_syscfg_uxxx_t;

#define SYSCFG_BASE 0x46000400
#define SYSCFG ((stm32_syscfg_uxxx_t *)SYSCFG_BASE)

#define PWR_DBPR_DBP BIT(0)

#define PWR_SVMCR_IO2SV BIT(29)

void vddio2_en(int on)
{
  if (on)
    PWR->svmcr |= PWR_SVMCR_IO2SV;
  else
    PWR->svmcr &= ~PWR_SVMCR_IO2SV;
}

void backup_domain_protect(int on)
{
  if (on)
    PWR->dbpr &= ~PWR_DBPR_DBP;
  else
    PWR->dbpr |= PWR_DBPR_DBP;
}

#define PWR_VOSR_BOOSTRDY BIT(14)
#define PWR_VOSR_VOSRDY BIT(15)
#define PWR_VOSR_BOOSTEN BIT(18)

void stm32_pwr_boost(int on)
{
  if (on) {
    PWR->vosr |= PWR_VOSR_BOOSTEN;

#if 0
    while ((PWR->vosr & PWR_VOSR_BOOSTRDY) == 0)
      ;
#endif
  } else
    PWR->vosr &= ~PWR_VOSR_BOOSTEN;
}

void stm32_pwr_vos(unsigned int vos)
{
  reg_set_field(&PWR->vosr, 2, 16, vos);

  while ((PWR->vosr & PWR_VOSR_VOSRDY) == 0)
    ;
}

void stm32_pwr_power(unsigned int val)
{
  reg_set_field(&PWR->cr[2], 1, 1, val);
}

#if 0
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

#endif
