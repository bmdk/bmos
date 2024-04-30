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
#include "hal_rtc.h"
#include "stm32_hal_gpio.h"
#include "stm32_hal_spi.h"
#include "stm32_hal_uart.h"
#include "stm32_pwr.h"
#include "stm32_rcc_b.h"

void pin_init()
{
  enable_ahb2(0);  /* GPIOA */
  enable_ahb2(1);  /* GPIOB */
  enable_ahb2(2);  /* GPIOC */

  enable_ahb2(13); /* ADC12 */

  /* USART2 */
  enable_apb1(17);
  gpio_init_attr(GPIO(0, 2), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(0, 3),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PD, GPIO_SPEED_HIG, 7,
                                 GPIO_ALT));

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

  /* FDCAN */
  enable_apb1(25);

  gpio_init_attr(GPIO(0, 11), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
  gpio_init_attr(GPIO(0, 12), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
}

#define APB2_CLOCK 170000000
#define APB1_CLOCK 170000000

#if BMOS
uart_t debug_uart = { "debug", USART2_BASE, APB1_CLOCK, 38, STM32_UART_FIFO };
#endif

static const gpio_handle_t leds[] = { GPIO(1, 8) };

/* configure system clock to run at 170MHz */
#if 1
/* HSI16 */
static const struct pll_params_t pll_params = {
  .flags   = PLL_FLAG_PLLREN,
  .pllsrc  = RCC_PLLCFGR_PLLSRC_HSI16,
  .pllm    = 4,
  .plln    = 85,
  .pllr    = PLLR_DIV_2,
  .latency = 4
};
#else
/* 24MHz external oscillator */
static const struct pll_params_t pll_params = {
  .flags   = PLL_FLAG_PLLREN,
  .pllsrc  = RCC_PLLCFGR_PLLSRC_HSE,
  .pllm    = 6,
  .plln    = 85,
  .pllr    = PLLR_DIV_2,
  .latency = 4
};
#endif


unsigned int hal_cpu_clock = 170000000;

void hal_board_init()
{
  pin_init();
  led_init(leds, ARRSIZ(leds));
  led_set(0, 1);
  clock_init(&pll_params);
#if APPL
  backup_domain_protect(0);
  clock_init_ls(1);
  rtc_init(0);
#endif

  debug_uart_init(USART2_BASE, 115200, APB1_CLOCK, 0);
}
