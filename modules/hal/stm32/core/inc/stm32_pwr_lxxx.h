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

#ifndef STM32_PWR_LXXX_H
#define STM32_PWR_LXXX_H

#include "common.h"

typedef struct {
  struct {
    unsigned int imr;
    unsigned int emr;
    unsigned int rtsr;
    unsigned int ftsr;
    unsigned int swier;
    unsigned int pr;
    unsigned int pad0[2];
  } r[2];
} stm32_exti_t;

typedef struct {
  unsigned int memrmp;
  unsigned int cfgr1;
  unsigned int exticr[4];
  unsigned int scsr;
  unsigned int cfgr2;
  unsigned int swpr;
  unsigned int skr;
} stm32_syscfg_t;

typedef struct {
  unsigned int cr[4];
  unsigned int sr[2];
  unsigned int scr;
  unsigned int pad0;
  struct {
    unsigned int d;
    unsigned int u;
  } p[8];
} stm32_pwr_t;

#if STM32_WBXX
#define PWR ((volatile stm32_pwr_t *)(0x58000400))
#define EXTI ((volatile stm32_exti_t *)(0x58000800))
#else
#define PWR ((volatile stm32_pwr_t *)(0x40007000))
#define EXTI ((volatile stm32_exti_t *)(0x40010400))
#endif
#define SYSCFG ((volatile stm32_syscfg_t *)(0x40010000))

#define PWR_CR1_DBP BIT(8)

#define PWR_CR2_IOSV BIT(9)

/* validate vddio2 and enable PG2-15 */
void vddio2_en(int on);

void stm32_pwr_vos(unsigned int vos);
int stm32_pwr_vos_rdy(void);

#endif
