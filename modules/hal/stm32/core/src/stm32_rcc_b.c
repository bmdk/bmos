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
#include "stm32_hal.h"
#include "stm32_regs.h"

#define FLASH_ACR ((volatile unsigned int *)0x40022000)
#define RCC ((volatile stm32_rcc_t *)0x40021000)

#define RCC_CR_PLLRDY BIT(25)
#define RCC_CR_PLLON BIT(24)
#define RCC_CR_HSEBYP BIT(18)
#define RCC_CR_HSERDY BIT(17)
#define RCC_CR_HSEON BIT(16)
#define RCC_CR_HSIRDY BIT(10)
#define RCC_CR_HSION BIT(8)

#define RCC_PLLCFGR_PLLREN BIT(24)

#define RCC_PLLCFGR_PLLSRC_NONE 0
#define RCC_PLLCFGR_PLLSRC_MSI 1
#define RCC_PLLCFGR_PLLSRC_HSI16 2
#define RCC_PLLCFGR_PLLSRC_HSE 3

#define RCC_CFGR_SW_MSI 0
#define RCC_CFGR_SW_HSI16 1
#define RCC_CFGR_SW_HSE 2
#define RCC_CFGR_SW_PLL 3

#define PLLCFGR(pllr, pllren, pllq, pllqen, plln, pllm, src) \
  ((((pllr) & 0x3) << 25) | (((pllren) & 0x1) << 24) | \
   (((pllq) & 0x3) << 21) | (((pllqen) & 0x1) << 20) | \
   (((plln) & 0x7f) << 8) | (((pllm) & 0x7) << 4) | ((src) & 0x3))

/* 80Mhz clock from 8Mhz external source */
#define PLLCFGR_HSE_VAL PLLCFGR(0, 1, 0, 0, 40, 1, RCC_PLLCFGR_PLLSRC_HSE)
/* 80Mhz clock from 4Mhz internal msi */
#define PLLCFGR_MSI_VAL PLLCFGR(0, 1, 0, 0, 40, 0, RCC_PLLCFGR_PLLSRC_MSI)

#define CFGR(ppre2, ppre1, hpre, src) \
  ((((ppre2) & 0x7) << 11) | (((ppre1) & 0x7) << 8) | \
   (((hpre) & 0xf) << 4) | ((src) & 0x3))

#define CFGR_HIGH_VAL CFGR(0, 0, 0, RCC_CFGR_SW_PLL)

#define CFGR_LOW_VAL CFGR(0, 0, 0, RCC_CFGR_SW_MSI)

#define CLK_TYPE_EXT_8MHZ 0
#define CLK_TYPE_EXT_8MHZ_OSC 1
#define CLK_TYPE_INT_4MHZ 2

void clock_init_high(int type)
{
  if (type == CLK_TYPE_EXT_8MHZ_OSC) {
    RCC->cr |= RCC_CR_HSEON;
    while ((RCC->cr & RCC_CR_HSERDY) == 0)
      ;
  } else if (type == CLK_TYPE_EXT_8MHZ)
    RCC->cr |= RCC_CR_HSEBYP;

  RCC->cfgr = CFGR_LOW_VAL;
  while (((RCC->cfgr >> 2) & 0x3) != RCC_CFGR_SW_MSI)
    asm volatile ("nop");

  RCC->cr &= ~RCC_CR_PLLON;
  while (RCC->cr & RCC_CR_PLLRDY)
    ;

  if (type == CLK_TYPE_INT_4MHZ)
    RCC->pllcfgr = PLLCFGR_MSI_VAL;
  else
    RCC->pllcfgr = PLLCFGR_HSE_VAL;

  RCC->cr |= RCC_CR_PLLON;
  while ((RCC->cr & RCC_CR_PLLRDY) == 0)
    ;

  reg_set_field(FLASH_ACR, 4, 0, 4);

  RCC->cfgr = CFGR_HIGH_VAL;
  while (((RCC->cfgr >> 2) & 0x3) != RCC_CFGR_SW_PLL)
    asm volatile ("nop");
}

void clock_init_low(void)
{
  RCC->cfgr = CFGR_LOW_VAL;
  while (((RCC->cfgr >> 2) & 0x3) != RCC_CFGR_SW_MSI)
    asm volatile ("nop");

  RCC->cr &= ~RCC_CR_PLLON;
  while ((RCC->cr & RCC_CR_PLLRDY))
    ;

  reg_set_field(FLASH_ACR, 4, 0, 0);
}

#define RCC_BDCR_LSEON BIT(0)
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
  clock_init_high(CLK_TYPE_INT_4MHZ);
}

void enable_ahb1(unsigned int dev)
{
  RCC->ahb1enr |= (1U << dev);
}

void disable_ahb1(unsigned int dev)
{
  RCC->ahb1enr &= ~(1U << dev);
}

void enable_ahb2(unsigned int dev)
{
  RCC->ahb2enr |= (1U << dev);
}

void disable_ahb2(unsigned int dev)
{
  RCC->ahb2enr &= ~(1U << dev);
}

void enable_apb1(unsigned int dev)
{
  if (dev >= 32)
    RCC->apb1enr2 |= BIT(dev - 32);
  else
    RCC->apb1enr1 |= BIT(dev);
}

void disable_apb1(unsigned int dev)
{
  if (dev >= 32)
    RCC->apb1enr2 &= ~BIT(dev - 32);
  else
    RCC->apb1enr1 &= ~BIT(dev);
}

void enable_apb2(unsigned int dev)
{
  RCC->apb2enr |= (1U << dev);
}

void disable_apb2(unsigned int dev)
{
  RCC->apb2enr &= ~(1U << dev);
}
