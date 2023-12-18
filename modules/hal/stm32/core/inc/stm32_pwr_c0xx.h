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

#ifndef STM32_PWR_C0XX_H
#define STM32_PWR_C0XX_H

#include "common.h"
#include "hal_common.h"

typedef struct {
  reg32_t cr[4];
  reg32_t sr[2];
  reg32_t scr;
  reg32_t cr5;
  struct {
    reg32_t d;
    reg32_t u;
  } p[8];
} stm32_pwr_t;

#define PWR ((stm32_pwr_t *)(0x40007000))

#define PWR_LPMS_STOP0 0
#define PWR_LPMS_STOP1 1
#define PWR_LPMS_STOP2 2
#define PWR_LPMS_STANDBY 3
#define PWR_LPMS_SHUTDOWN 4

static inline void stm32_pwr_lpms(unsigned int val)
{
  reg_set_field(&PWR->cr[0], 3, 0, val);

  (void)PWR->cr[0];
}
#endif
