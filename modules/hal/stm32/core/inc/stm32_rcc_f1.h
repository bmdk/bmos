#ifndef STM32_RCC_A_H
#define STM32_RCC_A_H

#define RCC_F1_CLK_HSI 0
#define RCC_F1_CLK_HSE 1
#define RCC_F1_CLK_HSE_OSC 2

#define RCC_F1_PPRE_1 0
#define RCC_F1_PPRE_2 4
#define RCC_F1_PPRE_4 5
#define RCC_F1_PPRE_8 6
#define RCC_F1_PPRE_16 7

#define RCC_F1_HPRE_1 0
#define RCC_F1_HPRE_2 8
#define RCC_F1_HPRE_4 9
#define RCC_F1_HPRE_8 10
#define RCC_F1_HPRE_16 11
#define RCC_F1_HPRE_64 12
#define RCC_F1_HPRE_128 13
#define RCC_F1_HPRE_256 14
#define RCC_F1_HPRE_512 15

struct pll_params_t {
  unsigned char src;
  unsigned short plln;
  unsigned char pllm;
  unsigned char ppre1;
  unsigned char ppre2;
  unsigned char hpre;
};

void clock_init(const struct pll_params_t *p);

#endif
