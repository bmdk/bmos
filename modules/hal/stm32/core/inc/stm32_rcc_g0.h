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

#ifndef STM32_RCC_G0_H
#define STM32_RCC_G0_H

struct pll_params_t {
  unsigned char flags;
#if STM32_C0XX
  unsigned char cfgrsrc;
#else
  unsigned char pllsrc;
  unsigned char pllm;
  unsigned char plln;
  unsigned char pllp;
  unsigned char pllq;
  unsigned char pllr;
#endif
  /* FLASH latency */
  unsigned char latency;
};

void clock_init(const struct pll_params_t *pll_params);

#define PLL_FLAG_PLLREN BIT(0)
#define PLL_FLAG_PLLQEN BIT(1)
#define PLL_FLAG_PLLPEN BIT(2)
#define PLL_FLAG_BYPASS BIT(3)

#define RCC_PLLCFGR_PLLSRC_NONE 0
#define RCC_PLLCFGR_PLLSRC_HSI16 2
#define RCC_PLLCFGR_PLLSRC_HSE 3

#define RCC_CFGR_SW_HSISYS 0
#define RCC_CFGR_SW_HSE 1
#define RCC_CFGR_SW_PLLRCLK 2
#define RCC_CFGR_SW_LSI 3
#define RCC_CFGR_SW_LSE 4

void enable_io(unsigned int dev);
void disable_io(unsigned int dev);

#endif
