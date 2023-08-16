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
  stm32_gpio_set_alt(GPIO(0, 2), 1);
  stm32_gpio_set_alt(GPIO(0, 3), 1);
#endif

#if 0
  enable_apb1(43); /* TIM1 */
#endif

#if APPL
  enable_io(1);    /* GPIOB */
  enable_io(2);    /* GPIOC */

  enable_ahb1(0);  /* DMA1 */

  enable_apb1(43); /* TIM1 */
#if 0
  enable_apb1(0);  /* TIM2 */
#endif

  enable_apb1(10); /* RTC */

  enable_apb1(28); /* PWR */
  enable_apb1(32); /* SYSCFG */

  enable_apb1(52); /* ADC */

#if 0
  /* analogue 0 */
  gpio_init(GPIO(0, 0), GPIO_ANALOG);
#endif

  enable_apb1(21);
  gpio_init_attr(GPIO(1, 8), GPIO_ATTR_STM32(GPIO_FLAG_OPEN_DRAIN,
                                             GPIO_SPEED_LOW, 6, GPIO_ALT));
  gpio_init_attr(GPIO(1, 9), GPIO_ATTR_STM32(GPIO_FLAG_OPEN_DRAIN,
                                             GPIO_SPEED_LOW, 6, GPIO_ALT));

  /* KEY */
  gpio_init_attr(GPIO(2, 13), GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU,
                                              0, 0, GPIO_INPUT));

  /* BUTTON PC13 */
  stm32_exti_irq_set_edge_falling(13, 1);
  stm32_exti_irq_enable(13, 1);
  stm32_exti_ev_enable(13, 1);
  stm32_syscfg_set_exti(2, 13);
#endif
}

#define CLOCK_DEFAULT 12000000
#define CLOCK_EXT 48000000

#if BOOT
#define EXT 0
#else
#define EXT 1
#endif

#if EXT
#define CLOCK CLOCK_EXT
#else
#define CLOCK CLOCK_DEFAULT
#endif

#define APB1_CLOCK CLOCK

#if BMOS
uart_t debug_uart = { "debugser", USART2_BASE, APB1_CLOCK, 28 };
#endif

static const gpio_handle_t leds[] = { GPIO(0, 5) };

unsigned int hal_cpu_clock = CLOCK;

#if EXT
struct pll_params_t pll_params = {
  .flags   = 0,
  .cfgrsrc = RCC_CFGR_SW_HSE,
  .latency = 1
};
#endif

void hal_board_init()
{
  pin_init();
  led_init(leds, ARRSIZ(leds));
#if EXT
  clock_init(&pll_params);
#endif
#if APPL
  /* errata means RTC is reset on chip reset */
  backup_domain_protect(0);
  clock_init_ls(0);
#endif
  debug_uart_init(USART2_BASE, 115200, APB1_CLOCK, 0);
}
