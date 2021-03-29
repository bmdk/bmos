#ifndef STM32_RCC_A_H
#define STM32_RCC_A_H

#define RCC_A_CLK_HSI 0
#define RCC_A_CLK_HSE 1
#define RCC_A_CLK_HSE_OSC 2

#define RCC_A_PLLP_2 0
#define RCC_A_PLLP_4 1
#define RCC_A_PLLP_6 2
#define RCC_A_PLLP_8 3

#define RCC_A_PPRE_1 0
#define RCC_A_PPRE_2 4
#define RCC_A_PPRE_4 5
#define RCC_A_PPRE_8 6
#define RCC_A_PPRE_16 7

struct pll_params_t {
  unsigned char src;
  /* PLLCFGR */
  unsigned char pllr;
  unsigned char pllq;
  unsigned char pllp;
  unsigned short plln;
  unsigned char pllm;
  /* CFGR */
  unsigned char rtcpre;
  unsigned char ppre1;
  unsigned char ppre2;
  unsigned char hpre;
  /* FLASH_ACR */
  unsigned char acr;
};

#endif
