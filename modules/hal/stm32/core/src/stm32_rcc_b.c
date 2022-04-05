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

/* this supports the RCC on L4XX, G4XX, WBXX devices */

#include "common.h"
#include "hal_common.h"
#include "stm32_hal.h"
#include "stm32_rcc_b.h"
#include "stm32_flash.h"
#include "stm32_rcc_ls.h"

typedef struct {
  reg32_t cr;
  reg32_t icscr;
  reg32_t cfgr;
  reg32_t pllcfgr;
  reg32_t pllsai1cfgr;
  reg32_t pllsai2cfgr;
  reg32_t cier;
  reg32_t cifr;
  reg32_t cicr;
#if STM32_WBXX
  reg32_t smpscr; /* wb55 */
#else
  reg32_t pad0;
#endif
  reg32_t ahb1rstr;
  reg32_t ahb2rstr;
  reg32_t ahb3rstr;
  reg32_t pad1;
  reg32_t apb1rstr1;
  reg32_t apb1rstr2;
  reg32_t apb2rstr;
  reg32_t pad2;
  reg32_t ahb1enr;
  reg32_t ahb2enr;
  reg32_t ahb3enr;
  reg32_t pad3;
  reg32_t apb1enr1;
  reg32_t apb1enr2;
  reg32_t apb2enr;
  reg32_t pad4;
  reg32_t ahb1smenr;
  reg32_t ahb2smenr;
  reg32_t ahb3smenr;
  reg32_t pad5;
  reg32_t apb1smenr1;
  reg32_t apb1smenr2;
  reg32_t apb2smenr;
  reg32_t pad6;
  reg32_t ccipr;
  reg32_t pad7;
  rcc_ls_t rcc_ls;
  reg32_t crrcr;
#if STM32_WBXX
  /* TODO */
#else
  reg32_t ccipr2;
#endif
} stm32_rcc_t;

#define RCC_EXTCFGR (volatile unsigned int *)(RCC_BASE + 0x108)

#if STM32_WBXX
#define RCC_BASE 0x58000000
#define FLASH_BASE 0x58004000
#else
#define RCC_BASE 0x40021000
#define FLASH_BASE 0x40022000
#endif

#define RCC ((stm32_rcc_t *)RCC_BASE)

#define RCC_CR_PLLRDY BIT(25)
#define RCC_CR_PLLON BIT(24)
#define RCC_CR_HSEBYP BIT(18)
#define RCC_CR_HSERDY BIT(17)
#define RCC_CR_HSEON BIT(16)
#define RCC_CR_HSIRDY BIT(10)
#define RCC_CR_HSION BIT(8)
#define RCC_CR_MSIRGSEL BIT(3)
#define RCC_CR_MSIPLLEN BIT(2)
#define RCC_CR_MSIRDY BIT(1)
#define RCC_CR_MSION BIT(0)

void msi_set_range(unsigned int range)
{
  while ((RCC->cr & RCC_CR_MSIRDY) == 0)
    ;

  reg_set_field(&RCC->cr, 4, 4, range);

  RCC->cr |= RCC_CR_MSIRGSEL;

  while ((RCC->cr & RCC_CR_MSIRDY) == 0)
    ;
}

#if STM32_WBXX
#define RCC_PLLCFGR_PLLREN BIT(28)
#define RCC_PLLCFGR_PLLQEN BIT(24)
#else
#define RCC_PLLCFGR_PLLREN BIT(24)
#define RCC_PLLCFGR_PLLQEN BIT(20)
#endif

#define RCC_PLLCFGR_PLLPEN BIT(16)

#define RCC_CFGR_SW_MSI 0
#define RCC_CFGR_SW_HSI16 1
#define RCC_CFGR_SW_HSE 2
#define RCC_CFGR_SW_PLL 3

#if STM32_L4XX || STM32_L4R || STM32_WBXX
#define DEFAULT_CLOCK RCC_CFGR_SW_MSI
#elif STM32_G4XX
#define DEFAULT_CLOCK RCC_CFGR_SW_HSI16
#else
#error Define default clock
#endif

#if STM32_WBXX
#define PLLCFGR(pllr, pllq, pllp, plln, pllm, src) \
  (((((pllr) - 1) & 0x7) << 29) | ((((pllq) - 1) & 0x7) << 25) | \
   ((((pllp) - 1) & 0x1f) << 17) | \
   (((plln) & 0x7f) << 8) | ((((pllm) - 1) & 0x7) << 4) | ((src) & 0x3))
#else
#define PLLCFGR(pllr, pllq, pllp, plln, pllm, src) \
  ((((pllp) & 0x1f) << 27) | (((pllr) & 0x3) << 25) | \
   (((pllq) & 0x3) << 21) | \
   (((plln) & 0x7f) << 8) | ((((pllm) - 1) & 0x7) << 4) | ((src) & 0x3))
#endif

#define CFGR(ppre2, ppre1, hpre, src) \
  ((((ppre2) & 0x7) << 11) | (((ppre1) & 0x7) << 8) | \
   (((hpre) & 0xf) << 4) | ((src) & 0x3))

#define HIGH_CLOCK RCC_CFGR_SW_PLL
#define CFGR_HIGH_VAL CFGR(0, 0, 0, HIGH_CLOCK)

#define CFGR_LOW_VAL CFGR(0, 0, 0, DEFAULT_CLOCK)

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

  RCC->cfgr = CFGR_LOW_VAL;
  while (((RCC->cfgr >> 2) & 0x3) != DEFAULT_CLOCK)
    ;

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

#ifdef STM32_WBXX
  reg_set_field(RCC_EXTCFGR, 4, 4, 8);
#endif

  RCC->cfgr = CFGR_HIGH_VAL;
  while (((RCC->cfgr >> 2) & 0x3) != HIGH_CLOCK)
    ;
}

void clock_init_low(void)
{
  RCC->cfgr = CFGR_LOW_VAL;
  while (((RCC->cfgr >> 2) & 0x3) != DEFAULT_CLOCK)
    ;

  RCC->cr &= ~RCC_CR_PLLON;
  while ((RCC->cr & RCC_CR_PLLRDY))
    ;

  RCC->cr &= ~RCC_CR_HSEON;
  while (RCC->cr & RCC_CR_HSERDY)
    ;

  stm32_flash_latency(0);
}

void clock_init_ls()
{
  rcc_clock_init_ls(&RCC->rcc_ls);
}

const char *clock_ls_name()
{
  return rcc_clock_type_ls_str(&RCC->rcc_ls);
}

void clock_init(const struct pll_params_t *pll_params)
{
  _clock_init(pll_params);
}

void enable_ahb1(unsigned int dev)
{
  RCC->ahb1enr |= BIT(dev);
}

void disable_ahb1(unsigned int dev)
{
  RCC->ahb1enr &= ~BIT(dev);
}

void enable_ahb2(unsigned int dev)
{
  RCC->ahb2enr |= BIT(dev);
}

void disable_ahb2(unsigned int dev)
{
  RCC->ahb2enr &= ~BIT(dev);
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
  RCC->apb2enr |= BIT(dev);
}

void disable_apb2(unsigned int dev)
{
  RCC->apb2enr &= ~BIT(dev);
}
