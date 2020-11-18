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
#include "stm32_pwr_h7xx.h"
#include "stm32_rcc_c.h"

#if STM32_H7XX
#define RCC_BASE 0x58024400
#else
#error define RCC_BASE
#endif

#define RCC ((volatile stm32_rcc_c_t *)RCC_BASE)

#define RCC_CR_PLL3RDY BIT(29)
#define RCC_CR_PLL3ON BIT(28)
#define RCC_CR_PLL2RDY BIT(27)
#define RCC_CR_PLL2ON BIT(26)
#define RCC_CR_PLL1RDY BIT(25)
#define RCC_CR_PLL1ON BIT(24)

#define RCC_CR_HSECSSON BIT(19)
#define RCC_CR_HSEBYP BIT(18)
#define RCC_CR_HSERDY BIT(17)
#define RCC_CR_HSEON BIT(16)

#define RCC_CR_D2CKRDY BIT(15)
#define RCC_CR_D1CKRDY BIT(14)

#define RCC_CR_HSI48RDY BIT(13)
#define RCC_CR_HSI48ON BIT(12)

#define RCC_CR_CSIKERON BIT(9)
#define RCC_CR_CSIRDY BIT(8)
#define RCC_CR_CSION BIT(7)

#define RCC_CR_HSIDIV(v) ((v) << 3)
#define RCC_CR_HSIDIV_1 HDIDIV(0)
#define RCC_CR_HSIDIV_2 HDIDIV(1)
#define RCC_CR_HSIDIV_4 HDIDIV(2)
#define RCC_CR_HSIDIV_8 HDIDIV(3)
#define RCC_CR_HSIRDY BIT(2)
#define RCC_CR_HSIKERON BIT(1)
#define RCC_CR_HSION BIT(0)

#define RCC_PLLCKSELR_DIVM3_S 20
#define RCC_PLLCKSELR_DIVM2_S 12
#define RCC_PLLCKSELR_DIVM1_S 4
#define RCC_PLLCKSELR_HSI 0
#define RCC_PLLCKSELR_CSI 1
#define RCC_PLLCKSELR_HSE 2
#define RCC_PLLCKSELR_NONE 3

#define RCC_PLLCFGR_DIVR3EN BIT(24)
#define RCC_PLLCFGR_DIVQ3EN BIT(23)
#define RCC_PLLCFGR_DIVP3EN BIT(22)
#define RCC_PLLCFGR_DIVR2EN BIT(21)
#define RCC_PLLCFGR_DIVQ2EN BIT(20)
#define RCC_PLLCFGR_DIVP2EN BIT(19)
#define RCC_PLLCFGR_DIVR1EN BIT(18)
#define RCC_PLLCFGR_DIVQ1EN BIT(17)
#define RCC_PLLCFGR_DIVP1EN BIT(16)

#define RCC_PLLCFGR_PLL3RGE(v) ((v) << 10)
#define RCC_PLLCFGR_PLL3VCOSEL BIT(9)
#define RCC_PLLCFGR_PLL3FRACEN BIT(8)
#define RCC_PLLCFGR_PLL2RGE(v) ((v) << 6)
#define RCC_PLLCFGR_PLL2VCOSEL BIT(5)
#define RCC_PLLCFGR_PLL2FRACEN BIT(4)
#define RCC_PLLCFGR_PLL1RGE(v) ((v) << 2)
#define RCC_PLLCFGR_PLL1VCOSEL BIT(1)
#define RCC_PLLCFGR_PLL1FRACEN BIT(0)

#define RCC_PLL1DIVR_DIVR1(v)  (((v) & 0x7f) << 24)
#define RCC_PLL1DIVR_DIVQ1(v)  (((v) & 0x7f) << 16)
#define RCC_PLL1DIVR_DIVP1(v)  (((v) & 0x7f) << 9)
#define RCC_PLL1DIVR_DIVN1(v)  ((v) & 0x1ff)

typedef struct {
  unsigned int cr;
  unsigned int icscr;
  unsigned int crrcr;
  unsigned int pad0;
  unsigned int cfgr;
  unsigned int pad1;
  unsigned int d1cfgr;
  unsigned int d2cfgr;
  unsigned int d3cfgr;
  unsigned int pad2;
  unsigned int pllckselr;
  unsigned int pllcfgr;
  unsigned int pll1divr;
  unsigned int pll1fracr;
  unsigned int pll2divr;
  unsigned int pll2fracr;
  unsigned int pll3divr;
  unsigned int pll3fracr;
  unsigned int pad3;
  unsigned int d1ccipr;
  unsigned int d2ccip1r;
  unsigned int d2ccip2r;
  unsigned int d3ccipr;
  unsigned int pad4;
  unsigned int cier;
  unsigned int cifr;
  unsigned int cicr;
  unsigned int pad5;
  unsigned int bdcr;
  unsigned int csr;
  unsigned int pad6;
  unsigned int ahb3rstr;
  unsigned int ahb1rstr;
  unsigned int ahb2rstr;
  unsigned int ahb4rstr;
  unsigned int apb3rstr;
  unsigned int apb1lrstr;
  unsigned int apb1hrstr;
  unsigned int apb2rstr;
  unsigned int apb4rstr;
  unsigned int gcr;
  unsigned int pad7;
  unsigned int d3amr;
  unsigned int pad8[9];
  unsigned int rsr;
  unsigned int ahb3enr;
  unsigned int ahb1enr;
  unsigned int ahb2enr;
  unsigned int ahb4enr;
  unsigned int apb3enr;
  unsigned int apb1lenr;
  unsigned int apb1henr;
  unsigned int apb2enr;
  unsigned int apb4enr;
  unsigned int pad9;
  unsigned int ahb3lpenr;
  unsigned int ahb1lpenr;
  unsigned int ahb2lpenr;
  unsigned int ahb4lpenr;
  unsigned int apb3lpenr;
  unsigned int apb1llpenr;
  unsigned int apb1hlpenr;
  unsigned int apb2lpenr;
  unsigned int apb4lpenr;
} stm32_rcc_c_t;

#define CLOCK_PLL_SRC_HSI 0
#define CLOCK_PLL_SRC_CSI 1
#define CLOCK_PLL_SRC_HSE 2
#define CLOCK_PLL_SRC_NONE 3

#define RCC_CFGR_SW_HSI 0
#define RCC_CFGR_SW_CSI 1
#define RCC_CFGR_SW_HSE 2
#define RCC_CFGR_SW_PLL1 3

#define RCC_D1CFGR_D1CPRE_OFS 8
#define RCC_D1CFGR_HPRE_OFS 0
#define RCC_D1CFGR_D1PPRE_OFS 4

#define RCC_DIV4_1 0
#define RCC_DIV4_2 8
#define RCC_DIV4_4 9
#define RCC_DIV4_8 10
#define RCC_DIV4_16 11
#define RCC_DIV4_64 12
#define RCC_DIV4_128 13
#define RCC_DIV4_256 14
#define RCC_DIV4_512 15

#define RCC_DIV3_1 0
#define RCC_DIV3_2 4
#define RCC_DIV3_4 5
#define RCC_DIV3_8 6
#define RCC_DIV3_16 7

#define RCC_PLLCFGR_RGE_1_2 0
#define RCC_PLLCFGR_RGE_2_4 1
#define RCC_PLLCFGR_RGE_4_8 2
#define RCC_PLLCFGR_RGE_8_16 3

static void _clock_pll_src(unsigned int src)
{
  reg_set_field(&RCC->pllckselr, 2, 0, src);
}

static void _clock_pll_presc(unsigned int num, unsigned int div)
{
  unsigned int shift;

  if (num > 2)
    return;

  shift = 4 + num * 8;

  reg_set_field(&RCC->pllckselr, 6, shift, div);
}

static void pll1_set(struct pll_params_t *params)
{
  RCC->cr |= RCC_CR_CSION;
  while ((RCC->cr & RCC_CR_CSIRDY) == 0)
    ;

  reg_set_field(&RCC->cfgr, 3, 0, RCC_CFGR_SW_CSI);
  while (((RCC->cfgr >> 3) & 0x7) != RCC_CFGR_SW_CSI)
    ;

  RCC->cr &= ~RCC_CR_PLL1ON;
  while ((RCC->cr & RCC_CR_PLL1RDY) != 0)
    ;

  /* PLL source is HSE */
  _clock_pll_src(CLOCK_PLL_SRC_HSE);
  _clock_pll_presc(0, params->divm1);

  reg_set_field(&RCC->pll1divr, 9, 0, params->divn1 - 1);
  reg_set_field(&RCC->pll1divr, 7, 9, params->divp1 - 1);
  reg_set_field(&RCC->pll1divr, 7, 16, params->divq1 - 1);
  reg_set_field(&RCC->pll1divr, 7, 24, params->divr1 - 1);

  RCC->cr |= RCC_CR_PLL1ON;
  while ((RCC->cr & RCC_CR_PLL1RDY) == 0)
    ;

  reg_set_field(&RCC->cfgr, 3, 0, RCC_CFGR_SW_PLL1);
  while (((RCC->cfgr >> 3) & 0x7) != RCC_CFGR_SW_PLL1)
    ;
}

static void pll3_set(unsigned int mul, unsigned int div, unsigned int prediv)
{
  RCC->cr &= ~RCC_CR_PLL3ON;
  while ((RCC->cr & RCC_CR_PLL3RDY) != 0)
    ;

  _clock_pll_presc(2, prediv);

  /* DIVN */
  reg_set_field(&RCC->pll3divr, 9, 0, mul - 1);
  /* DIVR */
  reg_set_field(&RCC->pll3divr, 7, 24, div - 1);

  RCC->cr |= RCC_CR_PLL3ON;
  while ((RCC->cr & RCC_CR_PLL3RDY) == 0)
    ;
}

static void _clock_init(struct pll_params_t *params)
{
  unsigned int pllcfgr;

  /* disable pll1,2,3 */
  RCC->cr &= ~(RCC_CR_PLL1ON | RCC_CR_PLL2ON | RCC_CR_PLL3ON);
  /* Turn on high speed external clock */
  RCC->cr |= RCC_CR_HSEBYP | RCC_CR_HSEON;
  while ((RCC->cr & RCC_CR_HSERDY) == 0)
    ;

#if 0
  RCC->cr |= RCC_CR_CSION;
  while ((RCC->cr & RCC_CR_CSIRDY) == 0)
    ;
#endif

  reg_set_field(&RCC->d1cfgr, 4, RCC_D1CFGR_D1CPRE_OFS, RCC_DIV4_1);
  reg_set_field(&RCC->d1cfgr, 4, RCC_D1CFGR_HPRE_OFS, RCC_DIV4_2);
  reg_set_field(&RCC->d1cfgr, 3, RCC_D1CFGR_D1PPRE_OFS, RCC_DIV3_2);

  reg_set_field(&RCC->d2cfgr, 3, 4, RCC_DIV3_2);
  reg_set_field(&RCC->d2cfgr, 3, 8, RCC_DIV3_2);

  reg_set_field(&RCC->d3cfgr, 3, 4, RCC_DIV3_2);

  pllcfgr = RCC->pllcfgr;

  pllcfgr &= ~(RCC_PLLCFGR_DIVQ1EN | RCC_PLLCFGR_DIVR1EN | \
               RCC_PLLCFGR_DIVP2EN | RCC_PLLCFGR_DIVQ2EN |
               RCC_PLLCFGR_DIVR2EN | \
               RCC_PLLCFGR_DIVP3EN | RCC_PLLCFGR_DIVQ3EN |
               RCC_PLLCFGR_DIVR3EN);
  pllcfgr |= RCC_PLLCFGR_DIVP1EN | RCC_PLLCFGR_DIVR3EN;

  if (1)
    pllcfgr |= RCC_PLLCFGR_DIVQ1EN;

  RCC->pllcfgr = pllcfgr;

  reg_set_field(&RCC->pllcfgr, 2, 2, RCC_PLLCFGR_RGE_4_8);

  pll1_set(params);
  pll3_set(5, 10, 1);

  /* turn off hsi */
  RCC->cr &= ~RCC_CR_HSION;
  while ((RCC->cr & RCC_CR_HSIRDY))
    ;
}

void clock_init(struct pll_params_t *params)
{
  stm32_pwr_vos(PWR_VOS_HIG);
  _clock_init(params);
}

void enable_ahb1(unsigned int dev)
{
  RCC->ahb1enr |= (1U << dev);
}

void enable_ahb4(unsigned int dev)
{
  RCC->ahb4enr |= (1U << dev);
}

void enable_apb1(unsigned int dev)
{
  if (dev >= 32) {
    dev -= 32;
    RCC->apb1henr |= BIT(dev);
  } else
    RCC->apb1lenr |= BIT(dev);
}

void disable_apb1(unsigned int dev)
{
  if (dev >= 32) {
    dev -= 32;
    RCC->apb1henr &= ~BIT(dev);
  } else
    RCC->apb1lenr &= ~BIT(dev);
}


void enable_apb2(unsigned int dev)
{
  RCC->apb2enr |= BIT(dev);
}

void enable_apb3(unsigned int dev)
{
  RCC->apb3enr |= BIT(dev);
}

void enable_apb4(unsigned int dev)
{
  RCC->apb4enr |= (1U << dev);
}

void set_fdcansel(unsigned int sel)
{
  reg_set_field(&RCC->d2ccip1r, 2, 28, sel);
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
