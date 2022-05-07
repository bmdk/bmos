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
  /* FLASH latency */
  unsigned char latency;
#endif
};

void clock_init(const struct pll_params_t *params);

#define SPI123SEL_PLL1_Q_CK 0
#define SPI123SEL_PLL2_P_CK 1
#define SPI123SEL_PLL3_P_CK 2
#define SPI123SEL_I2S_CKIN 3
#define SPI123SEL_CSI_KER_CK 4
#define SPI123SEL_HSE_CK 5

#define SPI45SEL_APB 0
#define SPI45SEL_PLL2_Q_CK 1
#define SPI45SEL_PLL3_Q_CK 2
#define SPI45SEL_HSI_KER_CK 3
#define SPI45SEL_CSI_KER_CK 4

#define SPI6SEL_PCLK4 0
#define SPI6SEL_PLL2_Q_CK 1
#define SPI6SEL_PLL3_Q_CK 2
#define SPI6SEL_HSI_KER_CK 3
#define SPI6SEL_CSI_KER_CK 4
#define SPI6SEL_HSE_CK 5

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

void set_spi123sel(unsigned int sel);
void set_spi45sel(unsigned int sel);
void set_fdcansel(unsigned int sel);
void set_usbsel(unsigned int sel);
void set_usart234578sel(unsigned int sel);

#define SEL_MCO1_HSI 0
#define SEL_MCO1_LCE 1
#define SEL_MCO1_HSE 2
#define SEL_MCO1_PLL1_Q 3
#define SEL_MCO1_HSI48 4
void set_mco1(unsigned int sel, unsigned int div);

/* this boots the cpu with index idx - unfortunately when this method is used
   the cpu cannot be stopped again since the bit can only be written to one.
   When the m4 is rebooted it starts immediately. The only way to start with
   it's clock gated is to reboot both cpus. */
void boot_cpu(int idx);

#endif
