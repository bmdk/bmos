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
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "stm32_exti.h"
#include "stm32_hal.h"
#include "stm32_hal_gpio.h"
#include "stm32_hal_spi.h"
#include "stm32_hal_uart.h"
#include "stm32_pwr.h"
#include "stm32_rcc_g0.h"

void pin_init()
{
  enable_io(0); /* GPIOA */

  /* USART 2 */
  enable_apb1(17);

#if APPL
  gpio_init_attr(GPIO(0, 2), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 1, GPIO_ALT));
  gpio_init_attr(GPIO(0, 3), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 1, GPIO_ALT));
#else
  stm32_gpio_set_alt(GPIO(0, 9), 1);
  stm32_gpio_set_alt(GPIO(0, 10), 1);
#endif

#if APPL
  enable_io(1); /* GPIOB */
  enable_io(2); /* GPIOC */

  enable_apb1(46);

  gpio_init_attr(GPIO(0, 9), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 1, GPIO_ALT));
  gpio_init_attr(GPIO(0, 10), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 1, GPIO_ALT));

  enable_ahb1(0);  /* DMA1 */
  enable_ahb1(1);  /* DMA2 */

  enable_apb1(43); /* TIM1 */
#if 0
  enable_apb1(0);  /* TIM2 */
#endif

  enable_apb1(10); /* RTC */

  enable_apb1(28); /* PWR */
  enable_apb1(32); /* SYSCFG */

  /* KEY */
  gpio_init_attr(GPIO(0, 0), GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU,
                                             0, 0, GPIO_INPUT));

  /* BUTTON PA0 */
  stm32_exti_irq_set_edge_falling(0, 1);
  stm32_exti_irq_enable(0, 1);
  stm32_exti_ev_enable(0, 1);
  stm32_syscfg_set_exti(0, 0);
#endif
}

#define APB1_CLOCK 64000000

#if BMOS
uart_t debug_uart = { "debugser", USART1_BASE, APB1_CLOCK, 27 };
#endif

#if APPL
static const gpio_handle_t leds[] = { GPIO(1, 1) };
static const led_flag_t led_flags[] = { LED_FLAG_INV };

/* 8MHz oscillator input
   64MHz CPU clock
   64MHz AHB1
 */
struct pll_params_t pll_params = {
  .flags   = PLL_FLAG_PLLREN,
  .pllsrc  = RCC_PLLCFGR_PLLSRC_HSE,
  .pllm    = 1,
  .plln    = 16,
  .pllr    = 2,
  .latency = 2
};

unsigned int hal_cpu_clock = 64000000;
#endif

void hal_board_init()
{
  pin_init();
#if APPL
  led_init_flags(leds, led_flags, ARRSIZ(leds));
  stm32_pwr_vos(1);
  clock_init(&pll_params);
  backup_domain_protect(0);
  clock_init_ls(0);
#endif
  debug_uart_init(USART1_BASE, 115200, APB1_CLOCK, 0);
}
