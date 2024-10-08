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
#include "stm32_hal_uart.h"
#include "stm32_pwr.h"
#include "stm32_pwr_f4xx.h"
#include "stm32_rcc_a.h"

int bl_enter(void)
{
  return !gpio_get(GPIO(0, 0));
}

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

#if CONFIG_ESP01_UART
  /* USART 2 - to ESP01 */
  enable_apb1(17);

  gpio_init_attr(GPIO(0, 2), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(0, 3), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 7, GPIO_ALT));
#endif

  /* USB */
  enable_ahb2(7);

  gpio_init_attr(GPIO(0, 11), GPIO_ATTR_STM32(0, \
                                              GPIO_SPEED_VHI, 10, GPIO_ALT));
  gpio_init_attr(GPIO(0, 12), GPIO_ATTR_STM32(0, \
                                              GPIO_SPEED_VHI, 10, GPIO_ALT));

  /* TIM1 */
  enable_apb2(0);

  /* TIM2 */
  enable_apb1(0);

  /* DMA 1 */
  enable_ahb1(21);

  /* DMA 2 */
  enable_ahb1(22);

  enable_apb1(28); /* PWR */

  /* KEY */
  gpio_init(GPIO(0, 0), GPIO_INPUT);

  gpio_init_attr(GPIO(0, 0), GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU,
                                             0, 0, GPIO_INPUT));

  /* BUTTON PA0 */
  stm32_exti_irq_set_edge_falling(0, 1);
  stm32_exti_irq_enable(0, 1);
  stm32_exti_ev_enable(0, 1);
  stm32_syscfg_set_exti(0, 0);

  enable_apb2(8); /* ADC1 */
}

#define APB1_CLOCK 42000000
#define APB2_CLOCK 84000000

#if BMOS
uart_t debug_uart = { "debugser", USART1_BASE, APB2_CLOCK, 37 };
#if CONFIG_ESP01_UART
uart_t esp_uart = {
  .name    = "esp",
  .base    = USART2_BASE,
  .clock   = APB1_CLOCK,
  .irq     = 38,
  .msg_cnt = 8
};
#endif
#endif

static const gpio_handle_t leds[] = { GPIO(2, 13) };
static const led_flag_t led_flags[] = { LED_FLAG_INV };

/* 25MHz crystal input
   84MHz CPU clock
   42MHz AHB1
   84MHz AHB2
 */
static const struct pll_params_t pll_params = {
  .src     = RCC_A_CLK_HSE_OSC,
  .pllr    = 0,
  .pllp    = RCC_A_PLLP_4,
  .pllq    = 7,
  .plln    = 336,
  .pllm    = 25,
  .hpre    = 0,
  .ppre1   = RCC_A_PPRE_2,
  .ppre2   = RCC_A_PPRE_1,
  .latency = 2
};

unsigned int hal_cpu_clock = 84000000;

void hal_board_init()
{
  pin_init();
  stm32_pwr_vos(2);
  led_init_flags(leds, led_flags, ARRSIZ(leds));
  clock_init(&pll_params);
#if APPL
  backup_domain_protect(0);
  clock_init_ls(0);
#endif
  debug_uart_init(USART1_BASE, 115200, APB2_CLOCK, 0);
}
