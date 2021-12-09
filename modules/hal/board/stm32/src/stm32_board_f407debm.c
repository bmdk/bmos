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

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "debug_ser.h"
#include "debug_ser.h"
#include "hal_board.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "stm32_exti.h"
#include "stm32_hal.h"
#include "hal_board.h"
#include "stm32_hal_gpio.h"
#include "stm32_hal_spi.h"
#include "stm32_pwr.h"
#include "stm32_pwr_f4xx.h"
#include "stm32_rcc_a.h"

void pin_init()
{
  enable_ahb1(0); /* GPIOA */
  enable_ahb1(1); /* GPIOB */
  enable_ahb1(2); /* GPIOC */

  /* USART 1 */
  enable_apb2(4);

  gpio_init_attr(GPIO(0, 9), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(0, 10), GPIO_ATTR_STM32(0, \
                                              GPIO_SPEED_HIG, 7, GPIO_ALT));

  /* TIM1 */
  enable_apb2(0);

  /* TIM2 */
  enable_apb1(0);

  /* DMA 1 */
  enable_ahb1(21);

  /* DMA 2 */
  enable_ahb1(22);

  enable_apb1(28); /* PWR */

  /* USB */
  enable_ahb2(7);

  gpio_init_attr(GPIO(0, 11), GPIO_ATTR_STM32(0, \
                                              GPIO_SPEED_VHI, 10, GPIO_ALT));
  gpio_init_attr(GPIO(0, 12), GPIO_ATTR_STM32(0, \
                                              GPIO_SPEED_VHI, 10, GPIO_ALT));


  /* KEY */
  gpio_init(GPIO(0, 0), GPIO_INPUT);

  gpio_init_attr(GPIO(0, 0), GPIO_ATTR_STM32(GPIO_FLAG_PULL_PD,
                                             0, 0, GPIO_INPUT));

  /* BUTTON PA0 */
  stm32_exti_irq_set_edge_falling(0, 1);
  stm32_exti_irq_enable(0, 1);
  stm32_exti_ev_enable(0, 1);
  stm32_syscfg_set_exti(0, 0);
}

#define USART1_BASE (void *)0x40011000
#define USART2_BASE (void *)0x40004400
#define APB1_CLOCK 84000000
#define APB2_CLOCK 168000000

#if BMOS
uart_t debug_uart = { "debugser", USART1_BASE, APB2_CLOCK, 37 };
#endif

static const gpio_handle_t leds[] = { GPIO(0, 1) };
static const led_flag_t led_flags[] = { LED_FLAG_INV };

/* 8MHz crystal input
   168MHz CPU clock
   84MHz AHB1
   168MHz AHB2
 */
static const struct pll_params_t pll_params = {
  .src     = RCC_A_CLK_HSE_OSC,
  .pllr    = 0,
  .pllp    = RCC_A_PLLP_2,
  .pllq    = 4,
  .plln    = 168,
  .pllm    = 4,
  .hpre    = 0,
  .ppre1   = RCC_A_PPRE_2,
  .ppre2   = RCC_A_PPRE_1,
  .latency = 5
};

unsigned int hal_cpu_clock = 168000000;

void hal_board_init()
{
  pin_init();
  stm32_pwr_vos(1);
  led_init_flags(leds, led_flags, ARRSIZ(leds));
  clock_init(&pll_params);
  backup_domain_protect(0);
  clock_init_ls();
  debug_uart_init(USART1_BASE, 115200, APB2_CLOCK, 0);
}
