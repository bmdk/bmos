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
#include <string.h>

#include "common.h"
#include "debug_ser.h"
#include "hal_board.h"
#include "hal_gpio.h"
#include "hal_rtc.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "stm32_exti.h"
#include "stm32_hal.h"
#include "stm32_hal_gpio.h"
#include "stm32_hal_spi.h"
#include "stm32_hal_uart.h"
#include "stm32_pwr.h"
#include "stm32_pwr_f0.h"
#include "stm32_rcc_f1.h"

void pin_init()
{
#if APPL
  enable_ahb1(17); /* GPIOA */
  enable_apb2(14); /* USART 1 */

  enable_apb2(11); /* TIM1 */
  enable_ahb1(18); /* GPIOB */
  enable_ahb1(0);  /* DMA */
  enable_apb1(28); /* PWR */
  enable_apb2(0);  /* SYSCFG */

  gpio_init_attr(GPIO(0, 9), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 1, GPIO_ALT));
  gpio_init_attr(GPIO(0, 10), GPIO_ATTR_STM32(0, \
                                              GPIO_SPEED_HIG, 1, GPIO_ALT));
#else
  RCC->ahbenr = BIT(17);  /* GPIOA */
  RCC->apb2enr = BIT(14); /* USART 1 */

  stm32_gpio_set_alt(GPIO(0, 9), 1);
  stm32_gpio_set_alt(GPIO(0, 10), 1);
#endif
}

#define APB1_CLOCK 48000000
#define APB2_CLOCK 48000000

#if BMOS
uart_t debug_uart = { "debugser", USART1_BASE, APB2_CLOCK, 27 };
#endif

#if APPL
static const gpio_handle_t leds[] = { GPIO(0, 4) };
static const led_flag_t led_flags[] = { LED_FLAG_INV };

/* 8MHz crystal input
   48MHz CPU clock
   48MHz AHB1
   48MHz AHB2
 */
struct pll_params_t pll_params = {
  .src     = RCC_F1_CLK_HSE_OSC,
  .plln    = 1,
  .pllm    = 6,
  .ppre1   = RCC_F1_PPRE_1,
  .ppre2   = RCC_F1_PPRE_1,
  .hpre    = RCC_F1_HPRE_1,
  .latency = 1
};

unsigned int hal_cpu_clock = 48000000;
#endif

void hal_board_init()
{
  pin_init();
#if APPL
  led_init_flags(leds, led_flags, ARRSIZ(leds));
  clock_init(&pll_params);
  backup_domain_protect(0);
  clock_init_ls(1);
  stm32_memmap(SYSCFG_MEMMAP_SRAM);
#endif
  debug_uart_init(USART1_BASE, 115200, APB2_CLOCK, 0);
}
