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

/* this supports the RCC on G0XX */

#include "common.h"
#include "hal_common.h"
#include "stm32_hal.h"
#include "stm32_rcc_g0.h"
#include "stm32_flash.h"
#include "stm32_rcc_ls.h"

typedef struct {
  reg32_t cr;
  reg32_t icscr;
  reg32_t cfgr;
  reg32_t pllcfgr;
  reg32_t pad0;
  reg32_t crrcr;
  reg32_t cier;
  reg32_t cifr;
  reg32_t cicr;
  reg32_t ioprstr;
  reg32_t ahbrstr;
  reg32_t apb1rstr1;
  reg32_t apb1rstr2;
  reg32_t iopenr;
  reg32_t ahbenr;
  reg32_t apb1enr[2];
  reg32_t iopsmenr;
  reg32_t ahbsmenr;
  reg32_t apb1smenr1;
  reg32_t apb1smenr2;
  reg32_t ccipr[2];
  rcc_ls_t rcc_ls;
} stm32_rcc_t;

#define RCC_BASE 0x40021000
#define RCC ((stm32_rcc_t *)RCC_BASE)

#define RCC_CR_PLLRDY BIT(25)
#define RCC_CR_PLLON BIT(24)
#define RCC_CR_HSEBYP BIT(18)
#define RCC_CR_HSERDY BIT(17)
#define RCC_CR_HSEON BIT(16)
#define RCC_CR_HSIRDY BIT(10)
#define RCC_CR_HSION BIT(8)

#define RCC_PLLCFGR_PLLREN BIT(28)
#define RCC_PLLCFGR_PLLQEN BIT(24)
#define RCC_PLLCFGR_PLLPEN BIT(16)

#define PLLCFGR(pllr, pllq, pllp, plln, pllm, src) \
  (((((pllr) - 1) & 0x7) << 29) | ((((pllq) - 1) & 0x7) << 25) | \
   (((pllp) & 0x1f) << 17) | \
   (((plln) & 0x7f) << 8) | ((((pllm) - 1) & 0x7) << 4) | ((src) & 0x3))

#define CFGR(ppre, hpre, src) \
  ( (((ppre) & 0x7) << 12) | (((hpre) & 0xf) << 8) | ((src) & 0x7))

static void _clock_select(unsigned int clk)
{
  RCC->cfgr = CFGR(0, 0, clk);
  while (((RCC->cfgr >> 3) & 0x7) != clk)
    ;
}

#if STM32_C0XX
static void _clock_init(const struct pll_params_t *p)
{
  if (p->cfgrsrc == RCC_CFGR_SW_HSE) {
    unsigned int flags = RCC_CR_HSEON;

    if (p->flags & PLL_FLAG_BYPASS)
      flags |= RCC_CR_HSEBYP;

    RCC->cr |= flags;
    while ((RCC->cr & RCC_CR_HSERDY) == 0)
      ;
  }

  stm32_flash_latency(p->latency);

  _clock_select(p->cfgrsrc);
}
#else
static void _clock_init(const struct pll_params_t *p)
{
  unsigned int pllcfgr;

  if (p->pllsrc == RCC_PLLCFGR_PLLSRC_HSE) {
    unsigned int flags = RCC_CR_HSEON;

    if (p->flags & PLL_FLAG_BYPASS)
      flags |= RCC_CR_HSEBYP;

    RCC->cr |= flags;
    while ((RCC->cr & RCC_CR_HSERDY) == 0)
      ;
  }

  _clock_select(RCC_CFGR_SW_HSISYS);

  RCC->cr &= ~RCC_CR_PLLON;
  while (RCC->cr & RCC_CR_PLLRDY)
    ;

  pllcfgr = PLLCFGR(p->pllr, p->pllq, p->pllp, p->plln, p->pllm, p->pllsrc);

  if (p->flags & PLL_FLAG_PLLREN)
    pllcfgr |= RCC_PLLCFGR_PLLREN;

  if (p->flags & PLL_FLAG_PLLQEN)
    pllcfgr |= RCC_PLLCFGR_PLLQEN;

  if (p->flags & PLL_FLAG_PLLPEN)
    pllcfgr |= RCC_PLLCFGR_PLLPEN;

  RCC->pllcfgr = pllcfgr;

  RCC->cr |= RCC_CR_PLLON;
  while ((RCC->cr & RCC_CR_PLLRDY) == 0)
    ;

  stm32_flash_latency(p->latency);

  _clock_select(RCC_CFGR_SW_PLLRCLK);
}
#endif

#include "stm32_rcc_ls_int.h"

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

void enable_apb1(unsigned int dev)
{
  unsigned int n = 0;

  if (dev >= 32) {
    dev -= 32;
    n = 1;
  }

  RCC->apb1enr[n] |= BIT(dev);
}

void disable_apb1(unsigned int dev)
{
  unsigned int n = 0;

  if (dev >= 32) {
    dev -= 32;
    n = 1;
  }

  RCC->apb1enr[n] &= ~BIT(dev);
}

void set_fdcansel(unsigned int sel)
{
  reg_set_field(&RCC->ccipr[1], 2, 8, sel);
}
