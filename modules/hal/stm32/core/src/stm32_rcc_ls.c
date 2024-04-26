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

#include "common.h"
#include "hal_board.h"
#include "hal_common.h"
#include "stm32_rcc_ls.h"

#define RCC_CSR_LSION BIT(0)
#define RCC_CSR_LSIRDY BIT(1)

#define RCC_BDCR_LSEON BIT(0)
#define RCC_BDCR_LSERDY BIT(1)
#define RCC_BDCR_LSEBYP BIT(2)

/* U5 */
#if STM32_UXXX
#define RCC_BDCR_LSESYSEN BIT(7)
#define RCC_BDCR_LSESYSRDY BIT(11)

#define RCC_BDCR_LSIPREDIV BIT(28)
#endif

/* On U5 and H5 */
#define RCC_BDCR_LSION BIT(26)
#define RCC_BDCR_LSIRDY BIT(27)

#define RCC_BDCR_RTCEN BIT(15)
#define RCC_BDCR_BDRST BIT(16)

#define RCC_BDCR_RTCSEL_NONE 0
#define RCC_BDCR_RTCSEL_LSE 1
#define RCC_BDCR_RTCSEL_LSI 2
#define RCC_BDCR_RTCSEL_HSE 3

#define MAX_CYCLES (hal_cpu_clock * 2)

static int rcc_clock_type_ls(rcc_ls_t *rcc_ls)
{
  return (rcc_ls->bdcr >> 8) & 0x3;
}

static inline void _rcc_clock_set(rcc_ls_t *rcc_ls, unsigned int clock)
{
  reg_set_field(&rcc_ls->bdcr, 2, 8, clock);
}

static void rcc_clock_set(rcc_ls_t *rcc_ls, unsigned int clock)
{
  /* This will work if the backup domain has been reset by hardware -
     if there is no backup battery or it has just been connected */
  _rcc_clock_set(rcc_ls, clock);

  if (rcc_clock_type_ls(rcc_ls) != clock) {
    /* clock could not be set so we need to reset the backup domain */
    rcc_ls->bdcr |= RCC_BDCR_BDRST;
    rcc_ls->bdcr &= ~RCC_BDCR_BDRST;
    _rcc_clock_set(rcc_ls, clock);
  }
}

#if STM32_UXXX
/* set lse drive strength to max */
#define LSE_DRIVE 3
#elif STM32_H5XX
#define LSE_DRIVE 1
#else
#define LSE_DRIVE 0
#endif

static int rcc_clock_init_ls_ext(rcc_ls_t *rcc_ls)
{
  unsigned int cycles = MAX_CYCLES;

  reg_set_field(&rcc_ls->bdcr, 2, 3, LSE_DRIVE);

  rcc_ls->bdcr |= RCC_BDCR_LSEON;

  while ((rcc_ls->bdcr & RCC_BDCR_LSERDY) == 0)
    if (--cycles == 0)
      return -1;

  rcc_clock_set(rcc_ls, RCC_BDCR_RTCSEL_LSE);

  return 0;
}

static void rcc_clock_init_ls_int(rcc_ls_t *rcc_ls)
{
#if STM32_U5XX || STM32_H5XX
  rcc_ls->bdcr |= RCC_BDCR_LSION;

  while ((rcc_ls->bdcr & RCC_BDCR_LSIRDY) == 0)
    ;
#else
  rcc_ls->csr |= RCC_CSR_LSION;

  while ((rcc_ls->csr & RCC_CSR_LSIRDY) == 0)
    ;
#endif

  rcc_clock_set(rcc_ls, RCC_BDCR_RTCSEL_LSI);
}

void rcc_clock_init_ls(rcc_ls_t *rcc_ls, int internal)
{
  rcc_ls->bdcr &= ~RCC_BDCR_BDRST;

  if (internal || (rcc_clock_init_ls_ext(rcc_ls) < 0))
    rcc_clock_init_ls_int(rcc_ls);

  rcc_ls->bdcr |= RCC_BDCR_RTCEN;
}

static const char *rtc_clock_str[] = { "none", "lse", "lsi", "hse" };

const char *rcc_clock_type_ls_str(rcc_ls_t *rcc_ls)
{
  return rtc_clock_str[rcc_clock_type_ls(rcc_ls)];
}
