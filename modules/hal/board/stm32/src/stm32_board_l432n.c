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

#include <stdlib.h>

#include "common.h"
#include "cortexm.h"
#include "debug_ser.h"
#include "hal_common.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "stm32_exti.h"
#include "stm32_hal.h"
#include "hal_board.h"
#include "stm32_hal_gpio.h"
#include "stm32_hal_spi.h"
#include "stm32_hal_uart.h"
#include "stm32_pwr.h"
#include "stm32_rcc_b.h"

void pin_init()
{
  enable_ahb2(0); /* GPIOA */
  enable_ahb2(1); /* GPIOB */
  enable_ahb2(2); /* GPIOC */

  /* USART1 */
  enable_apb2(14);
  gpio_init_attr(GPIO(0, 9), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(0, 10), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));

  /* USART2 */
  enable_apb1(17);
  gpio_init_attr(GPIO(0, 2), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(0, 15), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 3, GPIO_ALT));

  /* SPI1 */
  enable_apb2(12);

  gpio_init_attr(GPIO(0, 7), GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 5, GPIO_ALT));
  gpio_init_attr(GPIO(0, 5), GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 5, GPIO_ALT));
#if 0
  gpio_init_attr(GPIO(0, 4), GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 5, GPIO_ALT));
#else
  gpio_init(GPIO(0, 4), GPIO_OUTPUT);
#endif
  gpio_init(GPIO(0, 6), GPIO_OUTPUT);

  /* SYSCFG */
  enable_apb2(0);

  /* PWR */
  enable_apb1(28);

  /* TIM 1 */
  enable_apb2(11);

  /* TIM 2 */
  enable_apb1(0);

  /* DMA 1 */
  enable_ahb1(0);

  /* DMA 2 */
  enable_ahb1(1);

  /* ADC */
  enable_ahb2(13);
}

#define APB2_CLOCK 80000000
#define APB1_CLOCK 80000000

#if BMOS
uart_t debug_uart = { "debug", USART2_BASE, APB1_CLOCK, 38 };
#endif

static const gpio_handle_t leds[] = { GPIO(1, 3) };

static const struct pll_params_t pll_params = {
  .flags   = PLL_FLAG_PLLREN,
  .pllsrc  = RCC_PLLCFGR_PLLSRC_MSI,
  .pllm    = 1,
  .plln    = 40,
  .pllr    = PLLR_DIV_2,
  .latency = 4
};

unsigned int hal_cpu_clock = 80000000;

void hal_board_init()
{
  pin_init();
  led_init(leds, ARRSIZ(leds));
  clock_init(&pll_params);
#if APPL
  backup_domain_protect(0);
  clock_init_ls(0);
#endif

  debug_uart_init(USART2_BASE, 115200, APB1_CLOCK, 0);
}
