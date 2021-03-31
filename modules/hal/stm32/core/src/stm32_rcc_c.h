#ifndef STM32_RCC_C_H
#define STM32_RCC_C_H

#define RCC_C_CLK_HSI 0
#define RCC_C_CLK_CSI 1
#define RCC_C_CLK_HSE_OSC 2
#define RCC_C_CLK_HSE 3

struct pll_params_t {
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
  /* FLASH_ACR */
  unsigned char acr;
#endif
};

#define FDCANSEL_HSE_CK 0
#define FDCANSEL_PLL1_Q_CK 1
#define FDCANSEL_PLL2_Q_CK 2

#define USB_DISABLE_CK 0
#define USB_PLL1_Q_CK 1
#define USB_PLL3_Q_CK 2
#define USB_HSI48_CK 3

void set_fdcansel(unsigned int sel);
void set_usbsel(unsigned int sel);

#endif
