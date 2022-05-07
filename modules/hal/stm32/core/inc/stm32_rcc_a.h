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

#ifndef STM32_RCC_A_H
#define STM32_RCC_A_H

#define RCC_A_CLK_HSI 0
#define RCC_A_CLK_HSE 1
#define RCC_A_CLK_HSE_OSC 2

#define RCC_A_PLLP_2 0
#define RCC_A_PLLP_4 1
#define RCC_A_PLLP_6 2
#define RCC_A_PLLP_8 3

#define RCC_A_PPRE_1 0
#define RCC_A_PPRE_2 4
#define RCC_A_PPRE_4 5
#define RCC_A_PPRE_8 6
#define RCC_A_PPRE_16 7

struct pll_params_t {
  unsigned char src;
  /* PLLCFGR */
  unsigned char pllr;
  unsigned char pllq;
  unsigned char pllp;
  unsigned short plln;
  unsigned char pllm;
  /* CFGR */
  unsigned char rtcpre;
  unsigned char ppre1;
  unsigned char ppre2;
  unsigned char hpre;
  /* FLASH latency */
  unsigned char latency;
};

void clock_init(const struct pll_params_t *p);

#endif
