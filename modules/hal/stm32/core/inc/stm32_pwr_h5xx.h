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

#ifndef STM32_PWR_H5XX_H
#define STM32_PWR_H5XX_H

#include "hal_common.h"

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

#define PWR_VOSR_VOSRDY BIT(3)

#define SYSCFG_ETH_PHY_MII 0
#define SYSCFG_ETH_PHY_RMII 1

void stm32_syscfg_eth_phy(unsigned int val);

#define PWR_LPMS_STOP 0
#define PWR_LPMS_STANDBY 1

static inline void stm32_pwr_lpms(unsigned int val)
{
  reg_set_field(&PWR->pmcr, 1, 0, val);

  (void)PWR->pmcr;
}
#endif
