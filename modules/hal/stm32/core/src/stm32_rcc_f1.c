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

/* this supports the RCC on F0XX and F1XX devices */

#include "common.h"
#include "hal_common.h"
#include "stm32_pwr.h"
#include "stm32_rcc_f1.h"
#include "stm32_flash.h"
#include "stm32_rcc_ls.h"

#define RCC_CR_PLLRDY BIT(25)
#define RCC_CR_PLLON BIT(24)
#define RCC_CR_CSSON BIT(19)
#define RCC_CR_HSEBYP BIT(18)
#define RCC_CR_HSERDY BIT(17)
#define RCC_CR_HSEON BIT(16)
#define RCC_CR_HSIRDY BIT(1)
#define RCC_CR_HSION BIT(0)

#define RCC_CFGR_MCO_OFF 0
#define RCC_CFGR_MCO_SYSCLK 4
#define RCC_CFGR_MCO_HSI 5
#define RCC_CFGR_MCO_HSE 6
#define RCC_CFGR_MCO_PLL_2 7
/* AT32 */
#define RCC_CFGR_PLLHEXTDIV_EN BIT(17)

#define RCC_CFGR_ADCPRE_2 0
#define RCC_CFGR_ADCPRE_4 1
#define RCC_CFGR_ADCPRE_6 2
#define RCC_CFGR_ADCPRE_8 3

#define RCC_CFGR_SW_HSI 0
#define RCC_CFGR_SW_HSE 1
#define RCC_CFGR_SW_PLL 2

/* AT32 */
#define RCC_CFGR_PLLRANGE BIT(31)

#define PLL_CFGR_PLLSRC_HSI 0
#define PLL_CFGR_PLLSRC_HSE 1

void clock_uninit(void)
{
  RCC->cr |= RCC_CR_HSION;
  while ((RCC->cr & RCC_CR_HSIRDY) == 0)
    asm volatile ("nop");

  reg_set_field(&RCC->cfgr, 2, 0, RCC_CFGR_SW_HSI);
  while (((RCC->cfgr >> 2) & 0x3) != RCC_CFGR_SW_HSI)
    asm volatile ("nop");

  reg_set_field(&RCC->cfgr, 3, 8, 0);
  reg_set_field(&RCC->cfgr, 3, 11, 0);
  reg_set_field(&RCC->cfgr, 4, 4, 0);

  stm32_flash_latency(0);
}

#define CLKOUT_DIV_1 0
#define CLKOUT_DIV_2 8
#define CLKOUT_DIV_4 9
#define CLKOUT_DIV_8 10
#define CLKOUT_DIV_16 11
#define CLKOUT_DIV_64 12
#define CLKOUT_DIV_128 13
#define CLKOUT_DIV_256 14

#define CLKOUT_SEL_NONE 0
#define CLKOUT_SEL_LICK 2
#define CLKOUT_SEL_LEXT 3
#define CLKOUT_SEL_SCLK 4
#define CLKOUT_SEL_HICK 5
#define CLKOUT_SEL_HEXT 6
#define CLKOUT_SEL_PLL_2 7
#define CLKOUT_SEL_PLL_4 12
#define CLKOUT_SEL_USB 13
#define CLKOUT_SEL_ADC 14

void clock_out_set(unsigned int src, unsigned int div)
{
  reg_set_field(&RCC->cfgr, 3, 24, src);
#if AT32_F4XX
  reg_set_field(&RCC->misc1, 1, 16, src >> 3);
  reg_set_field(&RCC->misc1, 4, 28, div);
#endif
}

void clock_init_hs(const struct pll_params_t *p)
{
  unsigned int pllsrc, pllm;

  RCC->cr |= RCC_CR_HSION;
  while ((RCC->cr & RCC_CR_HSIRDY) == 0)
    ;

  reg_set_field(&RCC->cfgr, 2, 0, RCC_CFGR_SW_HSI);
  while (((RCC->cfgr >> 2) & 0x3) != RCC_CFGR_SW_HSI)
    ;

  RCC->cr &= ~RCC_CR_PLLON;
  while ((RCC->cr & RCC_CR_PLLRDY))
    ;

  if (p->src == RCC_F1_CLK_HSE)
    RCC->cr |= RCC_CR_HSEBYP;
  else
    RCC->cr &= ~RCC_CR_HSEBYP;

  if (p->src == RCC_F1_CLK_HSI)
    pllsrc = PLL_CFGR_PLLSRC_HSI;
  else {
    pllsrc = PLL_CFGR_PLLSRC_HSE;
    RCC->cr |= RCC_CR_HSEON;
    while ((RCC->cr & RCC_CR_HSERDY) == 0)
      ;
  }

  //XASSERT(p->pllm > 1);
#if STM32_F0XX
  reg_set_field(&RCC->cfgr2, 4, 0, p->plln - 1);
#elif AT32_F4XX
  if (p->plln <= 1)
    RCC->cfgr &= ~RCC_CFGR_PLLHEXTDIV_EN;
  else
    RCC->cfgr |= RCC_CFGR_PLLHEXTDIV_EN;
  reg_set_field(&RCC->misc3, 3, 12, p->plln - 2);
#endif
  reg_set_field(&RCC->cfgr, 4, 4, p->hpre);
  reg_set_field(&RCC->cfgr, 3, 8, p->ppre1);
  reg_set_field(&RCC->cfgr, 3, 11, p->ppre2);
  reg_set_field(&RCC->cfgr, 2, 14, p->adcpre);
  reg_set_field(&RCC->cfgr, 1, 16, pllsrc);

#if AT32_F4XX
  pllm = p->pllm - 1;
  if (p->pllm < 16)
    pllm--;
#else
  pllm = p->pllm - 2;
#endif
  reg_set_field(&RCC->cfgr, 4, 18, pllm);
#if AT32_F4XX
  reg_set_field(&RCC->cfgr, 1, 28, p->adcpre >> 2);
  if (p->flags & PLL_FLAGS_HSPEED)
    RCC->cfgr |= RCC_CFGR_PLLRANGE;
  reg_set_field(&RCC->cfgr, 2, 29, pllm >> 4);
#endif

  RCC->cr |= RCC_CR_PLLON;
  while ((RCC->cr & RCC_CR_PLLRDY) == 0)
    ;

#if !AT32_F4XX
  stm32_flash_latency(p->latency);
#endif

#if AT32_F4XX
  reg_set_field(&RCC->misc3, 2, 4, 3);
#endif

  reg_set_field(&RCC->cfgr, 2, 0, RCC_CFGR_SW_PLL);
  while (((RCC->cfgr >> 2) & 0x3) != RCC_CFGR_SW_PLL)
    ;

#if AT32_F4XX
  reg_set_field(&RCC->misc3, 2, 4, 0);
#if 1
  clock_out_set(CLKOUT_SEL_PLL_4, CLKOUT_DIV_64);
#endif
#endif
}

#if !CONFIG_BUS_ENABLE_INLINE
void enable_apb1(unsigned int n)
{
  RCC->apb1enr |= BIT(n);
}

void enable_apb2(unsigned int n)
{
  RCC->apb2enr |= BIT(n);
}

void enable_ahb1(unsigned int n)
{
  RCC->ahbenr |= BIT(n);
}
#endif

void clock_init(const struct pll_params_t *p)
{
  clock_init_hs(p);
}

#include "stm32_rcc_ls_int.h"
