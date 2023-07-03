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

#include "stm32_h5xx.h"
#include "stm32_pwr.h"
#include "stm32_pwr_h5xx.h"

typedef struct {
  reg32_t pmcr;
  reg32_t pmsr;
  reg32_t pad0[2];
  reg32_t voscr;
  reg32_t vossr;
  reg32_t pad1[2];
  reg32_t bdcr;
  reg32_t dbpcr;
  reg32_t bdsr;
  reg32_t ucpdr;
  reg32_t sccr;
  reg32_t vmcr;
  reg32_t usbscr;
  reg32_t vmsr;
  reg32_t wuscr;
  reg32_t wusr;
  reg32_t wucr;
  reg32_t pad2;
  reg32_t ioretr;
  reg32_t pad3;
  reg32_t seccfgr;
  reg32_t privcfgr;
} stm32_pwr_h5xx_t;

#define PWR_BASE 0x44020800
#define PWR ((stm32_pwr_h5xx_t *)PWR_BASE)

#define PWR_DBPCR_DBP BIT(0)

void backup_domain_protect(int on)
{
  if (on)
    PWR->dbpcr &= ~PWR_DBPCR_DBP;
  else
    PWR->dbpcr |= PWR_DBPCR_DBP;
}

#define PWR_VOSR_VOSRDY BIT(3)

int stm32_pwr_vos_rdy()
{
  return (PWR->vossr & PWR_VOSR_VOSRDY) != 0;
}

void stm32_pwr_vos(unsigned int vos)
{
  reg_set_field(&PWR->voscr, 2, 4, vos);

  while (!stm32_pwr_vos_rdy())
    ;
}

void stm32_syscfg_eth_phy(unsigned int val)
{
  unsigned int reg_val = 0;

  if (val == SYSCFG_ETH_PHY_RMII)
    reg_val = 4;

  reg_set_field((void *)(SBS_BASE + 0x100), 3, 21, reg_val);
}
