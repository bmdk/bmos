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

/* this supports the RCC on H7XX devices */

#include "common.h"
#include "hal_common.h"
#include "stm32_hal.h"
#include "stm32_pwr.h"
#include "stm32_pwr_h7xx.h"
#include "stm32_rcc_h7.h"
#include "stm32_rcc_ls.h"

#if STM32_H7XX
#define RCC_BASE 0x58024400
#else
#error define RCC_BASE
#endif

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
  reg32_t cr;
  reg32_t icscr;
  reg32_t crrcr;
  reg32_t pad0;
  reg32_t cfgr;
  reg32_t pad1;
  reg32_t d1cfgr;
  reg32_t d2cfgr;
  reg32_t d3cfgr;
  reg32_t pad2;
  reg32_t pllckselr;
  reg32_t pllcfgr;
  reg32_t pll1divr;
  reg32_t pll1fracr;
  reg32_t pll2divr;
  reg32_t pll2fracr;
  reg32_t pll3divr;
  reg32_t pll3fracr;
  reg32_t pad3;
  reg32_t d1ccipr;
  reg32_t d2ccip1r;
  reg32_t d2ccip2r;
  reg32_t d3ccipr;
  reg32_t pad4;
  reg32_t cier;
  reg32_t cifr;
  reg32_t cicr;
  reg32_t pad5;
  rcc_ls_t rcc_ls;
  reg32_t pad6;
  reg32_t ahb3rstr;
  reg32_t ahb1rstr;
  reg32_t ahb2rstr;
  reg32_t ahb4rstr;
  reg32_t apb3rstr;
  reg32_t apb1lrstr;
  reg32_t apb1hrstr;
  reg32_t apb2rstr;
  reg32_t apb4rstr;
  reg32_t gcr;
  reg32_t pad7;
  reg32_t d3amr;
  reg32_t pad8[9];
  reg32_t rsr;
  reg32_t ahb3enr;
  reg32_t ahb1enr;
  reg32_t ahb2enr;
  reg32_t ahb4enr;
  reg32_t apb3enr;
  reg32_t apb1lenr;
  reg32_t apb1henr;
  reg32_t apb2enr;
  reg32_t apb4enr;
  reg32_t pad9;
  reg32_t ahb3lpenr;
  reg32_t ahb1lpenr;
  reg32_t ahb2lpenr;
  reg32_t ahb4lpenr;
  reg32_t apb3lpenr;
  reg32_t apb1llpenr;
  reg32_t apb1hlpenr;
  reg32_t apb2lpenr;
  reg32_t apb4lpenr;
} stm32_rcc_h7_t;

#define RCC ((stm32_rcc_h7_t *)RCC_BASE)

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

static void pll1_set(const struct pll_params_t *params)
{
  unsigned int src;

  RCC->cr |= RCC_CR_CSION;
  while ((RCC->cr & RCC_CR_CSIRDY) == 0)
    ;

  reg_set_field(&RCC->cfgr, 3, 0, RCC_CFGR_SW_CSI);
  while (((RCC->cfgr >> 3) & 0x7) != RCC_CFGR_SW_CSI)
    ;

  RCC->cr &= ~RCC_CR_PLL1ON;
  while ((RCC->cr & RCC_CR_PLL1RDY) != 0)
    ;

  if (params->pllsrc == RCC_C_CLK_CSI)
    src = CLOCK_PLL_SRC_CSI;
  else if (params->pllsrc == RCC_C_CLK_HSI)
    src = CLOCK_PLL_SRC_HSI;
  else
    src = CLOCK_PLL_SRC_HSE;

  _clock_pll_src(src);
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

static void _clock_init(const struct pll_params_t *params)
{
  unsigned int pllcfgr;

  /* disable pll1,2,3 */
  RCC->cr &= ~(RCC_CR_PLL1ON | RCC_CR_PLL2ON | RCC_CR_PLL3ON);
  /* Turn on high speed external clock */

  if (params->pllsrc == RCC_C_CLK_CSI) {
    RCC->cr |= RCC_CR_CSION;
    while ((RCC->cr & RCC_CR_CSIRDY) == 0)
      ;
  } else if (params->pllsrc == RCC_C_CLK_HSI) {
    RCC->cr |= RCC_CR_HSION;
    while ((RCC->cr & RCC_CR_HSIRDY) == 0)
      ;
  } else {
    if (params->pllsrc == RCC_C_CLK_HSE_OSC)
      RCC->cr &= ~RCC_CR_HSEBYP;
    else if (params->pllsrc == RCC_C_CLK_HSE)
      RCC->cr |= RCC_CR_HSEBYP;

    RCC->cr |= RCC_CR_HSEON;

    while ((RCC->cr & RCC_CR_HSERDY) == 0)
      ;
  }

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
  pllcfgr |= RCC_PLLCFGR_DIVP1EN | RCC_PLLCFGR_DIVQ1EN | RCC_PLLCFGR_DIVR3EN;

  if (1)
    pllcfgr |= RCC_PLLCFGR_DIVQ1EN;

  RCC->pllcfgr = pllcfgr;

  reg_set_field(&RCC->pllcfgr, 2, 2, RCC_PLLCFGR_RGE_4_8);

  pll1_set(params);
  pll3_set(5, 10, 1);
}

void clock_init(const struct pll_params_t *params)
{
  stm32_pwr_vos(PWR_VOS_SCALE_0);
  _clock_init(params);
}

#define BOOT_C1 BIT(2)
#define BOOT_C2 BIT(3)

void boot_cpu(int idx)
{
  if (idx == 0)
    RCC->gcr |= BOOT_C1;
  else if (idx == 1)
    RCC->gcr |= BOOT_C2;
}

void enable_ahb1(unsigned int dev)
{
  RCC->ahb1enr |= BIT(dev);
}

void enable_ahb2(unsigned int dev)
{
  RCC->ahb2enr |= BIT(dev);
}

void enable_ahb4(unsigned int dev)
{
  RCC->ahb4enr |= BIT(dev);
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

void disable_apb2(unsigned int dev)
{
  RCC->apb2enr &= ~BIT(dev);
}

void enable_apb3(unsigned int dev)
{
  RCC->apb3enr |= BIT(dev);
}

void enable_apb4(unsigned int dev)
{
  RCC->apb4enr |= BIT(dev);
}

void set_spi123sel(unsigned int sel)
{
  reg_set_field(&RCC->d2ccip1r, 3, 12, sel);
}

void set_spi45sel(unsigned int sel)
{
  reg_set_field(&RCC->d2ccip1r, 3, 16, sel);
}

void set_fdcansel(unsigned int sel)
{
  reg_set_field(&RCC->d2ccip1r, 2, 28, sel);
}

void set_usbsel(unsigned int sel)
{
  reg_set_field(&RCC->d2ccip2r, 2, 20, sel);
}

void set_usart234578sel(unsigned int sel)
{
  reg_set_field(&RCC->d2ccip2r, 3, 0, sel);
}

void set_spi6sel(unsigned int sel)
{
  reg_set_field(&RCC->d3ccipr, 3, 28, sel);
}

void set_mco1(unsigned int sel, unsigned int div)
{
  reg_set_field(&RCC->cfgr, 3, 22, sel);
  reg_set_field(&RCC->cfgr, 4, 18, div);
}

void clock_init_ls()
{
  rcc_clock_init_ls(&RCC->rcc_ls);
}
