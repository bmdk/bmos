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

/* this supports the RCC on F4XX and F7XX devices */

#include "common.h"
#include "hal_common.h"
#include "stm32_flash.h"
#include "stm32_pwr.h"
#include "stm32_rcc_a.h"

typedef struct {
  unsigned int cr;
  unsigned int pllcfgr;
  unsigned int cfgr;
  unsigned int cir;
  unsigned int ahb1rstr;
  unsigned int ahb2rstr;
  unsigned int ahb3rstr;
  unsigned int pad1;
  unsigned int apb1rstr;
  unsigned int apb2rstr;
  unsigned int pad2[2];
  unsigned int ahb1enr;
  unsigned int ahb2enr;
  unsigned int ahb3enr;
  unsigned int pad3;
  unsigned int apb1enr;
  unsigned int apb2enr;
  unsigned int pad4[2];
  unsigned int ahb1lpenr;
  unsigned int ahb2lpenr;
  unsigned int ahb3lpenr;
  unsigned int pad5;
  unsigned int apb1lpenr;
  unsigned int apb2lpenr;
  unsigned int pad6[2];
  unsigned int bdcr;
  unsigned int csr;
  unsigned int pad7[2];
  unsigned int sscgr;
  unsigned int pll12scfgr;
  unsigned int pllsaicfgr;
  unsigned int dckcfgr1;
  unsigned int dckcfgr2;
} stm32_rcc_t;

#define RCC ((volatile stm32_rcc_t *)0x40023800)

#define RCC_CR_PLLSAIRDY BIT(29)
#define RCC_CR_PLLSAION BIT(28)
#define RCC_CR_PLLRDY BIT(25)
#define RCC_CR_PLLON BIT(24)
#define RCC_CR_HSEBYP BIT(18)
#define RCC_CR_HSERDY BIT(17)
#define RCC_CR_HSEON BIT(16)
#define RCC_CR_HSIRDY BIT(1)
#define RCC_CR_HSION BIT(0)

#define PLLSRC_HSI 0
#define PLLSRC_HSE 1

#define PLLCFGR(pllr, pllq, src, pllp, plln, pllm) \
  ((((pllr) & 0x7) << 28) | (((pllq) & 0xf) << 24) | \
   (((src) & 0x3) << 22) | (((pllp) & 0x3) << 16) | \
   (((plln) & 0x1ff) << 6) | (((pllm) & 0x1f) << 0))

#define RCC_CFGR_SW_HSI 0
#define RCC_CFGR_SW_HSE 1
#define RCC_CFGR_SW_PLL 2

#define CFGR(rtcpre, ppre2, ppre1, hpre, src) \
  ((((rtcpre) & 0x1f) << 16) | \
   (((ppre2) & 0x7) << 13) | (((ppre1) & 0x7) << 10) | \
   (((hpre) & 0xf) << 4) | ((src) & 0x3))

void clock_init_lcd(void)
{
  RCC->pllsaicfgr = (2 << 28) | (4 << 24) | (50 << 6);

  RCC->cr |= RCC_CR_PLLSAION;
  while ((RCC->cr & RCC_CR_PLLSAIRDY) == 0)
    ;

  RCC->dckcfgr1 = (2 << 16);
}

void clock_init_hs(const struct pll_params_t *p)
{
  unsigned int pllsrc;

  RCC->cr |= RCC_CR_HSION;
  while ((RCC->cr & RCC_CR_HSIRDY) == 0)
    asm volatile ("nop");

  RCC->cfgr = CFGR(0, 0, 0, 0, RCC_CFGR_SW_HSI);
  while (((RCC->cfgr >> 2) & 0x3) != RCC_CFGR_SW_HSI)
    asm volatile ("nop");

  if (p->src == RCC_A_CLK_HSE)
    RCC->cr |= RCC_CR_HSEBYP;
  else
    RCC->cr &= ~RCC_CR_HSEBYP;

  RCC->cr &= ~RCC_CR_PLLON;
  while ((RCC->cr & RCC_CR_PLLRDY))
    ;

  if (p->src == RCC_A_CLK_HSI)
    pllsrc = PLLSRC_HSI;
  else {
    pllsrc = PLLSRC_HSE;
    RCC->cr |= RCC_CR_HSEON;
    while ((RCC->cr & RCC_CR_HSERDY) == 0)
      asm volatile ("nop");
  }

  RCC->pllcfgr = PLLCFGR(p->pllr, p->pllq, pllsrc, p->pllp, p->plln, p->pllm);

  RCC->cr |= RCC_CR_PLLON;
  while ((RCC->cr & RCC_CR_PLLRDY) == 0)
    ;

  stm32_flash_latency(p->latency);

  RCC->cfgr = CFGR(p->rtcpre, p->ppre2, p->ppre1, p->hpre, RCC_CFGR_SW_PLL);
  while (((RCC->cfgr >> 2) & 0x3) != RCC_CFGR_SW_PLL)
    asm volatile ("nop");
}

#define RCC_BDCR_LSEON BIT(0)
#define RCC_BDCR_LSERDY BIT(1)
#define RCC_BDCR_RTCEN BIT(15)
#define RCC_BDCR_BDRST BIT(16)

#define RCC_BDCR_RTCSEL_NONE 0
#define RCC_BDCR_RTCSEL_LSE 1
#define RCC_BDCR_RTCSEL_LSI 2
#define RCC_BDCR_RTCSEL_HSE 3

void clock_init_ls()
{
  RCC->bdcr &= ~RCC_BDCR_BDRST;
  RCC->bdcr |= RCC_BDCR_LSEON;

  while ((RCC->bdcr & RCC_BDCR_LSERDY) == 0)
    asm volatile ("nop");

  reg_set_field(&RCC->bdcr, 2, 8, RCC_BDCR_RTCSEL_LSE);

  RCC->bdcr |= RCC_BDCR_RTCEN;
}

void clock_init(const struct pll_params_t *p)
{
  clock_init_hs(p);
#ifdef STM32_LCD
  clock_init_lcd();
#endif
}

void enable_ahb1(unsigned int dev)
{
  RCC->ahb1enr |= BIT(dev);
}

void enable_ahb2(unsigned int dev)
{
  RCC->ahb2enr |= BIT(dev);
}

void enable_apb1(unsigned int dev)
{
  RCC->apb1enr |= BIT(dev);
}

void enable_apb2(unsigned int dev)
{
  RCC->apb2enr |= BIT(dev);
}
