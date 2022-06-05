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

/* this supports the RCC on L0XX */

#include "common.h"
#include "hal_common.h"
#include "stm32_hal.h"
#include "stm32_rcc_l0.h"
#include "stm32_flash.h"
#include "stm32_rcc_ls.h"

typedef struct {
  reg32_t cr;
  reg32_t icscr;
  reg32_t crrcr;
  reg32_t cfgr;
  reg32_t cier;
  reg32_t cifr;
  reg32_t cicr;
  reg32_t ioprstr;
  reg32_t ahbrstr;
  reg32_t apb2rstr;
  reg32_t apb1rstr;
  reg32_t iopenr;
  reg32_t ahbenr;
  reg32_t apb2enr;
  reg32_t apb1enr;
  reg32_t iopsmenr;
  reg32_t ahbsmenr;
  reg32_t apb2smenr;
  reg32_t apb1smenr;
  reg32_t ccipr;
  reg32_t csr;
} stm32_rcc_t;

#define RCC_BASE 0x40021000
#define RCC ((stm32_rcc_t *)RCC_BASE)

#define RCC_CR_PLLRDY BIT(25)
#define RCC_CR_PLLON BIT(24)
#define RCC_CR_HSEBYP BIT(18)
#define RCC_CR_HSERDY BIT(17)
#define RCC_CR_HSEON BIT(16)
#define RCC_CR_MSIRDY BIT(9)
#define RCC_CR_MSION BIT(8)

#define RCC_CFGR_SW_MSI 0
#define RCC_CFGR_SW_HSI16 1
#define RCC_CFGR_SW_HSE 2
#define RCC_CFGR_SW_PLL 3

#define RCC_CFGR_PLLSRC_HSE BIT(16)

static void _clock_init(const struct pll_params_t *p)
{
  unsigned int cfgr_flags = 0;

  if (p->pllsrc == RCC_PLLSRC_HSE) {
    unsigned int flags = RCC_CR_HSEON;

    if (p->flags & PLL_FLAG_BYPASS)
      flags |= RCC_CR_HSEBYP;

    RCC->cr |= flags;
    while ((RCC->cr & RCC_CR_HSERDY) == 0)
      ;

    cfgr_flags |= RCC_CFGR_PLLSRC_HSE;
  }

  reg_set_field(&RCC->cfgr, 2, 0, RCC_CFGR_SW_MSI);
  while (((RCC->cfgr >> 2) & 0x3) != RCC_CFGR_SW_MSI)
    ;

  RCC->cr &= ~RCC_CR_PLLON;
  while (RCC->cr & RCC_CR_PLLRDY)
    ;

  reg_set_field(&RCC->cfgr, 2, 22, p->plldiv);
  reg_set_field(&RCC->cfgr, 4, 18, p->pllmul);
  RCC->cfgr |= cfgr_flags;

  RCC->cr |= RCC_CR_PLLON;
  while ((RCC->cr & RCC_CR_PLLRDY) == 0)
    ;

  stm32_flash_latency(p->latency);

  reg_set_field(&RCC->cfgr, 2, 0, RCC_CFGR_SW_PLL);
  while (((RCC->cfgr >> 2) & 0x3) != RCC_CFGR_SW_PLL)
    ;
}

void clock_init(const struct pll_params_t *pll_params)
{
  _clock_init(pll_params);
}

void enable_io(unsigned int dev)
{
  RCC->iopenr |= BIT(dev);
}

void disable_io(unsigned int dev)
{
  RCC->iopenr &= ~BIT(dev);
}

void enable_ahb1(unsigned int dev)
{
  RCC->ahbenr |= BIT(dev);
}

void disable_ahb1(unsigned int dev)
{
  RCC->ahbenr &= ~BIT(dev);
}

void enable_apb2(unsigned int dev)
{
  RCC->apb2enr |= BIT(dev);
}

void disable_apb2(unsigned int dev)
{
  RCC->apb2enr &= ~BIT(dev);
}

void enable_apb1(unsigned int dev)
{
  RCC->apb1enr |= BIT(dev);
}

void disable_apb1(unsigned int dev)
{
  RCC->apb1enr &= ~BIT(dev);
}

const char *clock_ls_name(void)
{
  return "none";
}
