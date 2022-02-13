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

#include "common.h"
#include "cortexm.h"
#include "debug_ser.h"
#include "hal_board.h"
#include "hal_common.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "stm32_exti.h"
#include "stm32_hal.h"
#include "hal_board.h"
#include "stm32_hal_gpio.h"
#include "stm32_pwr.h"
#include "stm32_pwr_h7xx.h"
#include "stm32_rcc_h7.h"
#include "stm32_timer.h"

/* H7xx memory layout:
 * ITCM  0x00000000 64K
 * DTCM  0x20000000 128K
 * AXI   0x24000000 512K
 * SRAM1 0x30000000 128K
 * SRAM2 0x30020000 128K
 * SRAM3 0x30040000 32K
 * SRAM4 0x38000000 64K
 * Backup SRAM 0x38800000 4K
 */


static void pin_init()
{
  enable_ahb4(0);                     /* GPIOA */
  enable_ahb4(1);                     /* GPIOB */
  enable_ahb4(2);                     /* GPIOC */
  enable_ahb4(3);                     /* GPIOD */
  enable_ahb4(4);                     /* GPIOE */
  enable_ahb4(5);                     /* GPIOF */
  enable_ahb4(6);                     /* GPIOG */

  enable_ahb4(21);                    /* BDMA */

  /* Button */
  gpio_init_attr(GPIO(2, 13), GPIO_ATTR_STM32(GPIO_FLAG_PULL_PD,
                                              0, 0, GPIO_INPUT));

  /* USART2 */
  enable_apb1(17);
  gpio_init_attr(GPIO(0, 2), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(0, 3), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));

  /* SPI1 - SPI FLASH */
  enable_apb2(12);
  /* PB4 MISO */
  gpio_init_attr(GPIO(1, 4), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 5, GPIO_ALT));
  /* PD7 MOSI */
  gpio_init_attr(GPIO(3, 7), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 5, GPIO_ALT));
  /* PB3 CLK */
  gpio_init_attr(GPIO(1, 3), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 5, GPIO_ALT));
  /* PD6 CS */
  gpio_init(GPIO(3, 6), GPIO_OUTPUT);

  /* SPI4 - LCD DISPLAY */
  enable_apb2(13);
  /* SPI4 MOSI */
  gpio_init_attr(GPIO(4, 14), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 5, GPIO_ALT));
  /* SPI4 SCK */
  gpio_init_attr(GPIO(4, 12), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 5, GPIO_ALT));

  gpio_init(GPIO(4, 13), GPIO_OUTPUT); /* LCD_WR_RS */
  gpio_init(GPIO(4, 11), GPIO_OUTPUT); /* LCD_CD */
  gpio_init(GPIO(4, 10), GPIO_OUTPUT); /* Backlight */

  enable_apb4(1);                      /* SYSCFG */
  enable_apb1(0);                      /* TIM2 */
  enable_apb2(0);                      /* TIM1 */
}

#define USART2_BASE 0x40004400
#define USART3_BASE 0x40004800
#define APB2_CLOCK 120000000
#if BMOS
#if 0
uart_t debug_uart = { "debugser", (void *)USART2_BASE, APB2_CLOCK, 38 };
#else
uart_t debug_uart =
{ "debugser2", (void *)USART2_BASE, APB2_CLOCK, 38, STM32_UART_FIFO,
  "u2pool",    "u2tx" };
uart_t debug_uart_2 =
{ "debugser3", (void *)USART3_BASE, APB2_CLOCK, 39, STM32_UART_FIFO,
  "u3pool",    "u3tx" };
#endif
#endif

/* Red, Green, Blue */
static const gpio_handle_t leds[] = { GPIO(4, 3) };

#if 1
static struct pll_params_t clock_params = {
  .pllsrc = RCC_C_CLK_HSE_OSC,
  .divm1  = 5,
  .divn1  = 192,
  .divp1  = 2,
  .divq1  = 2,
  .divr1  = 2
};
#endif

unsigned int hal_cpu_clock = 480000000;

void hal_board_init()
{
  pin_init();
  led_init(leds, ARRSIZ(leds));
  stm32_pwr_power(PWR_CR3_SCUEN | PWR_CR3_LDOEN);
  clock_init(&clock_params);

  /* 48MHz - USB clock */
  set_spi123sel(SPI123SEL_PLL1_Q_CK);
  /* 120MHz - APB clock */
  set_spi45sel(SPI45SEL_APB);

#if APPL
  backup_domain_protect(0);
  clock_init_ls();

  stm32_syscfg_set_exti(2, 13);
  stm32_exti_irq_set_edge_rising(13, 1);
  stm32_exti_irq_enable(13, 1);
  stm32_exti_ev_enable(13, 1);
#endif

  debug_uart_init((void *)USART2_BASE, 115200, APB2_CLOCK, 0);
}
