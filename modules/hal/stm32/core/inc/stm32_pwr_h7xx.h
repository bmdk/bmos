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

#ifndef STM32_PWR_H7XX_H
#define STM32_PWR_H7XX_H

#define SYSCFG_ETH_PHY_MII 0
#define SYSCFG_ETH_PHY_RMII 1

void stm32_syscfg_eth_phy(unsigned int val);

void stm32_pwr_wkup_en(unsigned int n, int en);

#define PWR_VOS_LOW 1
#define PWR_VOS_MED 2
#define PWR_VOS_HIG 3

#define PWR_CR3_BYPASS BIT(0)
#define PWR_CR3_LDOEN BIT(1)
#define PWR_CR3_SDEN BIT(2)
#define PWR_CR3_SCUEN BIT(2)
#define PWR_CR3_SDLEVEL(_v_) (((_v_) & 0x3) << 4)
#define PWR_CR3_SDLEVEL_1V8 PWR_CR3_SDLEVEL(1)
#define PWR_CR3_SDLEVEL_2V5 PWR_CR3_SDLEVEL(2)

void stm32_pwr_power(unsigned int val);
void stm32_pwr_vos(unsigned int vos);
void stm32_pwr_usbvdetect(int en);
void stm32_pwr_usbreg(int en);

#endif
