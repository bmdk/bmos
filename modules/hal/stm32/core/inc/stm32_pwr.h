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

#ifndef STM32_PWR_H
#define STM32_PWR_H

void backup_domain_protect(int on);
void stm32_pwr_vos(unsigned int vos);
int stm32_pwr_vos_rdy(void);

#if STM32_C0XX
#include "stm32_pwr_c0xx.h"
#elif STM32_F0XX
#include "stm32_pwr_f0.h"
#elif STM32_F4XX
#include "stm32_pwr_f4.h"
#elif STM32_F7XX
#include "stm32_pwr_f7.h"
#elif STM32_H5XX
#include "stm32_pwr_h5.h"
#elif STM32_H7XX
#include "stm32_pwr_h7.h"
#elif STM32_L4XX || STM32_L4R || STM32_G4XX || STM32_WBXX
#include "stm32_pwr_lx.h"
#elif STM32_U5XX
#include "stm32_pwr_ux.h"
#elif STM32_F1XX || STM32_F3XX || AT32_F4XX || STM32_L0XX || STM32_G0XX || \
  STM32_U0XX
/* no inline header for these targets */
#else
#error Add pwr header for target
#endif

#endif
