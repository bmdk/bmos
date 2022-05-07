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

#ifndef STM32_RCC_B_H
#define STM32_RCC_B_H

struct pll_params_t {
  /* PLLCFGR */
  unsigned char flags;
  unsigned char pllsrc;
  unsigned char pllm;
  unsigned char plln;
  unsigned char pllp;
  unsigned char pllq;
  unsigned char pllr;
  /* FLASH latency */
  unsigned char latency;
};

void clock_init(const struct pll_params_t *pll_params);

#define PLL_FLAG_PLLREN BIT(0)
#define PLL_FLAG_PLLQEN BIT(1)
#define PLL_FLAG_PLLPEN BIT(2)
#define PLL_FLAG_BYPASS BIT(3)

#define RCC_PLLCFGR_PLLSRC_NONE 0
#define RCC_PLLCFGR_PLLSRC_MSI 1
#define RCC_PLLCFGR_PLLSRC_HSI16 2
#define RCC_PLLCFGR_PLLSRC_HSE 3

#define PLLR_DIV_2 0
#define PLLR_DIV_4 1
#define PLLR_DIV_6 2
#define PLLR_DIV_8 3

#define PLLQ_DIV_2 0
#define PLLQ_DIV_4 1
#define PLLQ_DIV_6 2
#define PLLQ_DIV_8 3

#define PLLP_DIV_7 0
#define PLLP_DIV_17 1

#define MSI_RANGE_100 0
#define MSI_RANGE_200 1
#define MSI_RANGE_400 2
#define MSI_RANGE_800 3
#define MSI_RANGE_1M  4
#define MSI_RANGE_2M  5
#define MSI_RANGE_4M  6
#define MSI_RANGE_8M  7
#define MSI_RANGE_16M 8
#define MSI_RANGE_24M 9
#define MSI_RANGE_32M 10
#define MSI_RANGE_48M 11

void msi_set_range(unsigned int range);

#endif
