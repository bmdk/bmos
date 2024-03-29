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

#include "stm32_rcc_ls.h"

typedef struct {
  reg32_t cr;
  reg32_t cfgr;
  reg32_t cir;
  reg32_t apb2rstr;
  reg32_t apb1rstr;
  reg32_t ahbenr;
  reg32_t apb2enr;
  reg32_t apb1enr;
  rcc_ls_t rcc_ls;
  reg32_t ahbrstr;
  reg32_t cfgr2;
#if AT32_F4XX
  reg32_t misc1;
  reg32_t pad1[7];
  reg32_t misc2;
  reg32_t misc3;
  reg32_t intmap;
#elif STM32_F0XX || STM32_F3XX
  reg32_t cfgr3;
  reg32_t cr2;
#endif
} stm32_rcc_f1_t;

#define RCC ((stm32_rcc_f1_t *)(0x40021000))

#if CONFIG_BUS_ENABLE_INLINE
static inline void enable_apb1(unsigned int n)
{
  RCC->apb1enr |= BIT(n);
}

static inline void enable_apb2(unsigned int n)
{
  RCC->apb2enr |= BIT(n);
}

static inline void enable_ahb1(unsigned int n)
{
  RCC->ahbenr |= BIT(n);
}
#endif

#ifndef STM32_RCC_A_H
#define STM32_RCC_A_H

#define RCC_F1_CLK_HSI 0
#define RCC_F1_CLK_HSE 1
#define RCC_F1_CLK_HSE_OSC 2

#define RCC_F1_ADCPRE_2 0
#define RCC_F1_ADCPRE_4 1
#define RCC_F1_ADCPRE_6 2
#define RCC_F1_ADCPRE_8 3
/* AT32 */
#define RCC_F1_ADCPRE_12 5
#define RCC_F1_ADCPRE_16 7

#define RCC_F1_PPRE_1 0
#define RCC_F1_PPRE_2 4
#define RCC_F1_PPRE_4 5
#define RCC_F1_PPRE_8 6
#define RCC_F1_PPRE_16 7

#define RCC_F1_HPRE_1 0
#define RCC_F1_HPRE_2 8
#define RCC_F1_HPRE_4 9
#define RCC_F1_HPRE_8 10
#define RCC_F1_HPRE_16 11
#define RCC_F1_HPRE_64 12
#define RCC_F1_HPRE_128 13
#define RCC_F1_HPRE_256 14
#define RCC_F1_HPRE_512 15

#define PLL_FLAGS_HSPEED BIT(0)

struct pll_params_t {
  unsigned char src;
  unsigned char flags;
  unsigned short plln;
  unsigned char pllm;
  unsigned char ppre1;
  unsigned char ppre2;
  unsigned char hpre;
  unsigned char adcpre;
  unsigned char latency;
};

void clock_init(const struct pll_params_t *p);

#endif
