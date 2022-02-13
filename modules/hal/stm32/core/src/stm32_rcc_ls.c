#include "common.h"
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

#define RCC_BDCR_LSION BIT(26)
#define RCC_BDCR_LSIRDY BIT(27)
#define RCC_BDCR_LSIPREDIV BIT(28)
#endif

#define RCC_BDCR_RTCEN BIT(15)
#define RCC_BDCR_BDRST BIT(16)

#define RCC_BDCR_RTCSEL_NONE 0
#define RCC_BDCR_RTCSEL_LSE 1
#define RCC_BDCR_RTCSEL_LSI 2
#define RCC_BDCR_RTCSEL_HSE 3

#define MAX_CYCLES 1000000

static int rcc_clock_init_ls_ext(rcc_ls_t *rcc_ls)
{
  unsigned int cycles = MAX_CYCLES;

#if STM32_UXXX
  /* set lse drive strength to max */
  reg_set_field(&rcc_ls->bdcr, 2, 3, 0x3);
#endif

  rcc_ls->bdcr |= RCC_BDCR_LSEON;

  while ((rcc_ls->bdcr & RCC_BDCR_LSERDY) == 0)
    if (--cycles == 0)
      return -1;

  reg_set_field(&rcc_ls->bdcr, 2, 8, RCC_BDCR_RTCSEL_LSE);

  return 0;
}

static void rcc_clock_init_ls_int(rcc_ls_t *rcc_ls)
{
#if STM32_U5XX
  rcc_ls->bdcr |= RCC_BDCR_LSION;

  while ((rcc_ls->bdcr & RCC_BDCR_LSIRDY) == 0)
    ;
#else
  rcc_ls->csr |= RCC_CSR_LSION;

  while ((rcc_ls->csr & RCC_CSR_LSIRDY) == 0)
    ;
#endif

  reg_set_field(&rcc_ls->bdcr, 2, 8, RCC_BDCR_RTCSEL_LSI);
}

void rcc_clock_init_ls(rcc_ls_t *rcc_ls)
{
  rcc_ls->bdcr &= ~RCC_BDCR_BDRST;

  if (rcc_clock_init_ls_ext(rcc_ls) < 0)
    rcc_clock_init_ls_int(rcc_ls);

  rcc_ls->bdcr |= RCC_BDCR_RTCEN;
}
