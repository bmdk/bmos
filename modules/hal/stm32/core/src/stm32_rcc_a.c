/* Copyright (c) 2019 Brian Thomas Murphy
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

#include "common.h"
#include "hal_common.h"
#include "stm32_regs.h"
#include "stm32_pwr.h"

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

#define FLASH_ACR ((volatile unsigned int *)0x40023c00)
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

#ifdef STM32_F746
#define CLOCK_EXT 1
#define PLLSRC PLLSRC_HSE
#define HPRE 0
#define PLLM 25
#define PLLN 240
#else
#ifdef STM32_F429
#define CLOCK_EXT 1
#define PLLSRC PLLSRC_HSE
#define HPRE 0
#define PLLM 4
#define PLLN 120
#else
#define CLOCK_EXT 1
#define PLLSRC PLLSRC_HSE
#define HPRE 0
#define PLLM 4
#define PLLN 120
#endif
#endif

#define PLLCFGR_VAL PLLCFGR(0, 5, PLLSRC, 0, PLLN, PLLM)

#define RCC_CFGR_SW_HSI 0
#define RCC_CFGR_SW_HSE 1
#define RCC_CFGR_SW_PLL 2

#define CFGR(rtcpre, ppre2, ppre1, hpre, src) \
  ((((rtcpre) & 0x1f) << 16) | \
   (((ppre2) & 0x7) << 13) | (((ppre1) & 0x7) << 10) | \
   (((hpre) & 0xf) << 4) | ((src) & 0x3))

#define CFGR_VAL CFGR(0, 4, 5, HPRE, RCC_CFGR_SW_PLL)

void clock_init_hs(void)
{
  RCC->cfgr = CFGR(0, 4, 5, HPRE, RCC_CFGR_SW_HSI);
  while (((RCC->cfgr >> 2) & 0x3) != RCC_CFGR_SW_HSI)
    asm volatile ("nop");

#if CLOCK_EXT
  RCC->cr |= RCC_CR_HSEBYP;
#else
  RCC->cr &= ~RCC_CR_HSEBYP;
#endif

  RCC->cr &= ~RCC_CR_PLLON;
  while ((RCC->cr & RCC_CR_PLLRDY))
    ;

#if CLOCK_EXT
  RCC->cr |= RCC_CR_HSEON;
  while ((RCC->cr & RCC_CR_HSERDY) == 0)
    asm volatile ("nop");
#else
  RCC->cr |= RCC_CR_HSION;
  while ((RCC->cr & RCC_CR_HSIRDY) == 0)
    asm volatile ("nop");
#endif

  RCC->pllcfgr = PLLCFGR_VAL;

  RCC->cr |= RCC_CR_PLLON;
  while ((RCC->cr & RCC_CR_PLLRDY) == 0)
    ;

#ifdef STM32_LCD
  RCC->pllsaicfgr = (2 << 28) | (4 << 24) | (50 << 6);

  RCC->cr |= RCC_CR_PLLSAION;
  while ((RCC->cr & RCC_CR_PLLSAIRDY) == 0)
    ;
#endif

  unsigned int l = *FLASH_ACR;
  l &= ~0xf;
  l |= 5;
  *FLASH_ACR = l;

  RCC->cfgr = CFGR_VAL;
  while (((RCC->cfgr >> 2) & 0x3) != RCC_CFGR_SW_PLL)
    asm volatile ("nop");

#ifdef STM32_LCD
  RCC->dckcfgr1 = (2 << 16);
#endif
#if 0
#ifndef STM32_F429
  RCC->dckcfgr1 = (0 << 16) | (3 << 8);
  RCC->dckcfgr2 = (0 << 2); /* USART2 HSI */
#endif
#endif
}

#define RCC_BDCR_LSEON BIT(0)
#define RCC_BDCR_LSERDY BIT(1)
#define RCC_BDCR_LSERDY BIT(1)
#define RCC_BDCR_RTCEN BIT(15)
#define RCC_BDCR_BDRST BIT(16)

void clock_init_ls()
{
  RCC->bdcr &= ~RCC_BDCR_BDRST;
  RCC->bdcr |= RCC_BDCR_LSEON | RCC_BDCR_RTCEN;

  while ((RCC->bdcr & RCC_BDCR_LSERDY) == 0)
    asm volatile ("nop");

  reg_set_field(&RCC->bdcr, 2, 8, 1);
}

void clock_init(void)
{
#if 1
  clock_init_hs();
#endif
}

void enable_ahb1(unsigned int dev)
{
  RCC->ahb1enr |= (1U << dev);
}

void enable_ahb2(unsigned int dev)
{
  RCC->ahb2enr |= (1U << dev);
}

void enable_apb1(unsigned int dev)
{
  RCC->apb1enr |= (1U << dev);
}

void enable_apb2(unsigned int dev)
{
  RCC->apb2enr |= (1U << dev);
}
