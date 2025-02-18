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

/* this supports the RCC on U5XX devices */

#include "common.h"
#include "hal_common.h"
#include "stm32_flash.h"
#include "stm32_hal.h"
#include "stm32_rcc_ls.h"
#include "stm32_rcc_u5.h"

#if STM32_U5XX
#define RCC_BASE 0x46020C00
#else
#error define RCC_BASE
#endif

#define RCC ((stm32_rcc_u5_t *)RCC_BASE)

#define RCC_CR_PLL3RDY BIT(29)
#define RCC_CR_PLL3ON BIT(28)
#define RCC_CR_PLL2RDY BIT(27)
#define RCC_CR_PLL2ON BIT(26)
#define RCC_CR_PLL1RDY BIT(25)
#define RCC_CR_PLL1ON BIT(24)

#define RCC_CR_HSEEXT BIT(20)
#define RCC_CR_CSSON BIT(19)
#define RCC_CR_HSEBYP BIT(18)
#define RCC_CR_HSERDY BIT(17)
#define RCC_CR_HSEON BIT(16)

#define RCC_CR_SHSIRDY BIT(15)
#define RCC_CR_SHSION BIT(14)

#define RCC_CR_HSI48RDY BIT(13)
#define RCC_CR_HSI48ON BIT(12)

#define RCC_CR_HSIRDY BIT(10)
#define RCC_CR_HSIKERON BIT(9)
#define RCC_CR_HSION BIT(8)

#define RCC_CR_MSIPLLFAST BIT(7)
#define RCC_CR_MSIPLLSEL BIT(6)
#define RCC_CR_MSIKRDY BIT(5)
#define RCC_CR_MSIKON BIT(4)
#define RCC_CR_MSIPLLEN BIT(3)
#define RCC_CR_MSISRDY BIT(2)
#define RCC_CR_MSIKERON BIT(1)
#define RCC_CR_MSISON BIT(0)

#define RCC_PLLCFGR_DIVREN BIT(18)
#define RCC_PLLCFGR_DIVQEN BIT(17)
#define RCC_PLLCFGR_DIVPEN BIT(16)

#define RCC_PLLCFGR_PLLMBOOST(v) (((v) & 0xf) << 12)
#define RCC_PLLCFGR_PLLM(v) (((v) & 0xf) << 8)
#define RCC_PLLCFGR_PLLFRACEN BIT(4)
#define RCC_PLLCFGR_PLLRGE(v) (((v) & 0x3) << 2)
#define RCC_PLLCFGR_PLLSRC(v) ((v) & 0x3)

#define RCC_PLLDIVR_DIVR(v)  (((v) & 0x7f) << 24)
#define RCC_PLLDIVR_DIVQ(v)  (((v) & 0x7f) << 16)
#define RCC_PLLDIVR_DIVP(v)  (((v) & 0x7f) << 9)
#define RCC_PLLDIVR_DIVN(v)  ((v) & 0x1ff)

#define CLOCK_PLL_SRC_NONE 0
#define CLOCK_PLL_SRC_MSIS 1
#define CLOCK_PLL_SRC_HSI16 2
#define CLOCK_PLL_SRC_HSE 3

#define RCC_CFGR_SW_MSIS 0
#define RCC_CFGR_SW_HSI16 1
#define RCC_CFGR_SW_HSE 2
#define RCC_CFGR_SW_PLL1 3

typedef struct {
  reg32_t cr;
  unsigned int pad0;
  reg32_t icscr[3];
  reg32_t crrcr;
  unsigned int pad1;
  reg32_t cfgr[3];
  reg32_t pllcfgr[3];
  struct {
    reg32_t divr;
    reg32_t fracr;
  } pll[3];
  unsigned int pad2;
  reg32_t cier;
  reg32_t cifr;
  reg32_t cicr;
  unsigned int pad3;
  reg32_t ahb1rstr;
  reg32_t ahb2rstr[2];
  reg32_t ahb3rstr;
  unsigned int pad4;
  reg32_t apb1rstr[2];
  reg32_t apb2rstr;
  reg32_t apb3rstr;
  unsigned int pad5;
  reg32_t ahb1enr;
  reg32_t ahb2enr[2];
  reg32_t ahb3enr;
  unsigned int pad6;
  reg32_t apb1enr[2];
  reg32_t apb2enr;
  reg32_t apb3enr;
  unsigned int pad7;
  reg32_t ahb1smenr;
  reg32_t ahb2smenr[2];
  reg32_t ahb3smenr;
  unsigned int pad8;
  reg32_t apb1smenr[2];
  reg32_t apb2smenr;
  reg32_t apb3smenr;
  unsigned int pad9;
  reg32_t srdamr;
  unsigned int pad10;
  reg32_t ccipr[3];
  unsigned int pad11;
  rcc_ls_t rcc_ls;
  unsigned int pad12[6];
  reg32_t seccfgr;
  reg32_t privcfgr;
} stm32_rcc_u5_t;

/* 4 bit dividers */
#define RCC_DIV4_1 0
#define RCC_DIV4_2 8
#define RCC_DIV4_4 9
#define RCC_DIV4_8 10
#define RCC_DIV4_16 11
#define RCC_DIV4_64 12
#define RCC_DIV4_128 13
#define RCC_DIV4_256 14
#define RCC_DIV4_512 15

/* 3 bit dividers */
#define RCC_DIV3_1 0
#define RCC_DIV3_2 4
#define RCC_DIV3_4 5
#define RCC_DIV3_8 6
#define RCC_DIV3_16 7

#define RCC_PLLCFGR_RGE_4_8 2
#define RCC_PLLCFGR_RGE_8_16 3

static void _clock_pll_src(unsigned int num, unsigned int src)
{
  if (num > 2)
    return;

  reg_set_field(&RCC->pllcfgr[num], 2, 0, src);
}

static void _clock_pll_presc(unsigned int num, unsigned int div)
{
  if (num > 2)
    return;

  reg_set_field(&RCC->pllcfgr[num], 4, 8, div - 1);
}

#define RCC_DIVP 0
#define RCC_DIVQ 1
#define RCC_DIVR 2

static void _clock_pll_div_en(unsigned int num, unsigned int div, int en)
{
  unsigned int bit;

  if (num > 2 || div > 2)
    return;

  bit = BIT(16 + div);

  if (en)
    RCC->pllcfgr[num] |= bit;
  else
    RCC->pllcfgr[num] &= ~bit;
}

static void pll1_set(const struct pll_params_t *params)
{
  unsigned int src;

  RCC->cr |= RCC_CR_MSISON;
  while ((RCC->cr & RCC_CR_MSISRDY) == 0)
    ;

  reg_set_field(&RCC->cfgr[0], 2, 0, RCC_CFGR_SW_MSIS);
  while (((RCC->cfgr[0] >> 2) & 0x3) != RCC_CFGR_SW_MSIS)
    ;

  RCC->cr &= ~RCC_CR_PLL1ON;
  while ((RCC->cr & RCC_CR_PLL1RDY) != 0)
    ;

  if (params->pllsrc == RCC_D_CLK_MSIS)
    src = CLOCK_PLL_SRC_MSIS;
  else if (params->pllsrc == RCC_D_CLK_HSI16)
    src = CLOCK_PLL_SRC_HSI16;
  else
    src = CLOCK_PLL_SRC_HSE;

  _clock_pll_src(0, src);
  _clock_pll_presc(0, params->divm1);

  reg_set_field(&RCC->pll[0].divr, 9, 0, params->divn1 - 1);
  reg_set_field(&RCC->pll[0].divr, 7, 9, params->divp1 - 1);
  reg_set_field(&RCC->pll[0].divr, 7, 16, params->divq1 - 1);
  reg_set_field(&RCC->pll[0].divr, 7, 24, params->divr1 - 1);

  RCC->cr |= RCC_CR_PLL1ON;
  while ((RCC->cr & RCC_CR_PLL1RDY) == 0)
    ;

  stm32_flash_latency(params->latency);

  reg_set_field(&RCC->cfgr[0], 2, 0, RCC_CFGR_SW_PLL1);
  while (((RCC->cfgr[0] >> 2) & 0x3) != RCC_CFGR_SW_PLL1)
    ;
}

static void _clock_init(const struct pll_params_t *params)
{
  unsigned int i, j;

  /* disable pll1,2,3 */
  RCC->cr &= ~(RCC_CR_PLL1ON | RCC_CR_PLL2ON | RCC_CR_PLL3ON);

  if (params->pllsrc == RCC_D_CLK_MSIS) {
    RCC->cr |= RCC_CR_MSISON;
    while ((RCC->cr & RCC_CR_MSISRDY) == 0)
      ;
  } else if (params->pllsrc == RCC_D_CLK_HSI16) {
    RCC->cr |= RCC_CR_HSION;
    while ((RCC->cr & RCC_CR_HSIRDY) == 0)
      ;
  } else {
    if (params->pllsrc == RCC_D_CLK_HSE_OSC)
      RCC->cr &= ~RCC_CR_HSEBYP;
    else if (params->pllsrc == RCC_D_CLK_HSE)
      RCC->cr |= RCC_CR_HSEBYP;

    RCC->cr |= RCC_CR_HSEON;

    while ((RCC->cr & RCC_CR_HSERDY) == 0)
      ;
  }

  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      _clock_pll_div_en(i, j, 0);

  if (params->flags & PLL_FLAG_PLLREN)
    _clock_pll_div_en(0, RCC_DIVR, 1);
  if (params->flags & PLL_FLAG_PLLQEN)
    _clock_pll_div_en(0, RCC_DIVQ, 1);
  if (params->flags & PLL_FLAG_PLLPEN)
    _clock_pll_div_en(0, RCC_DIVP, 1);

  reg_set_field(&RCC->pllcfgr[0], 2, 2, RCC_PLLCFGR_RGE_4_8);

  pll1_set(params);
}

void clock_init(const struct pll_params_t *params)
{
  _clock_init(params);
}

void enable_ahb1(unsigned int dev)
{
  RCC->ahb1enr |= BIT(dev);
}

void enable_ahb3(unsigned int dev)
{
  RCC->ahb3enr |= BIT(dev);
}

void enable_ahb2(unsigned int dev)
{
  unsigned int reg;

  reg = dev / 32;
  dev %= 32;

  RCC->ahb2enr[reg] |= BIT(dev);
}

void enable_apb1(unsigned int dev)
{
  unsigned int reg;

  reg = dev / 32;
  dev %= 32;

  RCC->apb1enr[reg] |= BIT(dev);
}

void disable_apb1(unsigned int dev)
{
  unsigned int reg;

  reg = dev / 32;
  dev %= 32;

  RCC->apb1enr[reg] &= ~BIT(dev);
}

void enable_apb2(unsigned int dev)
{
  RCC->apb2enr |= BIT(dev);
}

void enable_apb3(unsigned int dev)
{
  RCC->apb3enr |= BIT(dev);
}

#include "stm32_rcc_ls_int.h"

void set_mco(unsigned int sel, unsigned int div)
{
  reg_set_field(&RCC->cfgr[0], 4, 24, sel);
  reg_set_field(&RCC->cfgr[0], 3, 28, div);
}

void set_fdcansel(unsigned int sel)
{
  reg_set_field(&RCC->ccipr[0], 2, 24, sel);
}

void set_i2c3sel(unsigned int sel)
{
  reg_set_field(&RCC->ccipr[2], 2, 6, sel);
}
