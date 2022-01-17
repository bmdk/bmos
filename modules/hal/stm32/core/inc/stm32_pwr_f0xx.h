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

#ifndef STM32_PWR_FXXX
#define STM32_PWR_FXXX

#include "common.h"
#include "hal_common.h"

typedef struct {
  reg32_t cfgr1;
  reg32_t rsvd0;
  reg32_t exticr[4];
  reg32_t cfgr2;
} stm32_syscfg_t;

typedef struct {
  reg32_t cr;
  reg32_t csr;
} stm32_pwr_t;

#define SYSCFG ((stm32_syscfg_t *)(0x40010000))
#define PWR ((stm32_pwr_t *)(0x40007000))

#define SYSCFG_MEMMAP_MAIN 0
#define SYSCFG_MEMMAP_SYS 1
#define SYSCFG_MEMMAP_SRAM 3

static inline void stm32_memmap(unsigned int val)
{
  reg_set_field(&SYSCFG->cfgr1, 2, 0, val);
}

#endif
