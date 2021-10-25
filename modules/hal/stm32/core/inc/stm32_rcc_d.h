#ifndef STM32_RCC_C_H
#define STM32_RCC_C_H

#define RCC_D_CLK_MSIS 0
#define RCC_D_CLK_HSI16 1
#define RCC_D_CLK_HSE_OSC 2
#define RCC_D_CLK_HSE 3

#define PLL_FLAG_PLLREN BIT(0)
#define PLL_FLAG_PLLQEN BIT(1)
#define PLL_FLAG_PLLPEN BIT(2)

struct pll_params_t {
  unsigned char flags;
  /* PLLCKSELR */
  unsigned char pllsrc;
  unsigned char divm1;
  /* PLL1DIVR */
  unsigned short divn1;
  unsigned char divp1;
  unsigned char divq1;
  unsigned char divr1;
#if 0
  /* CFGR */
  unsigned char rtcpre;
  unsigned char ppre1;
  unsigned char ppre2;
  unsigned char hpre;
#endif
  /* FLASH_ACR */
  unsigned char acr;
};

void clock_init(const struct pll_params_t *params);

#define FDCANSEL_HSE_CK 0
#define FDCANSEL_PLL1_Q_CK 1
#define FDCANSEL_PLL2_Q_CK 2

#define USB_DISABLE_CK 0
#define USB_PLL1_Q_CK 1
#define USB_PLL3_Q_CK 2
#define USB_HSI48_CK 3

#define USART234578SEL_RCC_PCLK1 0
#define USART234578SEL_PLL2_Q_CK 1
#define USART234578SEL_PLL3_Q_CK 2
#define USART234578SEL_HSI_KER_CK 3
#define USART234578SEL_CSI_KER_CK 4
#define USART234578SEL_LSE_CK 5

void set_fdcansel(unsigned int sel);
void set_usbsel(unsigned int sel);
void set_usart234578sel(unsigned int sel);

#define RCC_CFGR_MCO_DIV_1 0
#define RCC_CFGR_MCO_DIV_2 1
#define RCC_CFGR_MCO_DIV_4 2
#define RCC_CFGR_MCO_DIV_8 3
#define RCC_CFGR_MCO_DIV_16 4

#define RCC_CFGR_MCO_SEL_DIS 0
#define RCC_CFGR_MCO_SEL_SYSCLK 1
#define RCC_CFGR_MCO_SEL_MSIS 2
#define RCC_CFGR_MCO_SEL_HSI16 3
#define RCC_CFGR_MCO_SEL_HSE 4
#define RCC_CFGR_MCO_SEL_PLL1R 5
#define RCC_CFGR_MCO_SEL_LSI 6
#define RCC_CFGR_MCO_SEL_LSE 7
#define RCC_CFGR_MCO_SEL_HSI48 8

void set_mco(unsigned int sel, unsigned int div);

#endif
