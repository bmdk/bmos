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
#include "debug_ser.h"
#include "hal_board.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "stm32_exti.h"
#include "stm32_hal.h"
#include "stm32_hal_gpio.h"
#include "stm32_pwr.h"
#include "stm32_rcc_f1.h"

#define USART1_BASE (void *)0x40013800

#define CLOCK 72000000
#define APB1_CLOCK (CLOCK / 2)
#define APB2_CLOCK CLOCK

static void pin_init(void)
{
  enable_apb2(2);  /* GPIOA */
  enable_apb2(3);  /* GPIOB */
  enable_apb2(4);  /* GPIOC */

  enable_apb2(11); /* TIM1 */

  enable_apb2(14); /* USART1 */

  /* USART1 TX */
  gpio_init_attr(GPIO(0, 9), GPIO_ATTR_STM32F1(CNF_ALT_PP, MODE_OUTPUT_HIG));
  /* USART1 RX */
  gpio_init(GPIO(0, 10), MODE_INPUT);

  enable_ahb1(0);  /* DMA1 */

  enable_apb1(0);  /* TIM2 */
  enable_apb1(27); /* BKP */
  enable_apb1(28); /* PWR */

#if APPL
  /* KEY */
  gpio_set(GPIO(0, 0), 0);
  gpio_init_attr(GPIO(0, 0), GPIO_ATTR_STM32F1(CNF_INP_PUD, MODE_INPUT));

  /* BUTTON PA0 */
  stm32_exti_irq_set_edge_falling(0, 0);
  stm32_exti_irq_set_edge_rising(0, 1);
  stm32_exti_irq_enable(0, 1);
  stm32_exti_ev_enable(0, 1);
  stm32_syscfg_set_exti(0, 0);
#endif
}

#if BMOS
uart_t debug_uart = { "debugser", USART1_BASE, APB2_CLOCK, 37 };
#endif

static const gpio_handle_t leds[] = { GPIO(2, 13) };
static const led_flag_t led_flags[] = { LED_FLAG_INV };

struct pll_params_t pll_params = {
  .src     = RCC_F1_CLK_HSE_OSC,
  .plln    = 1,
  .pllm    = 9,
  .ppre1   = RCC_F1_PPRE_2,
  .ppre2   = RCC_F1_PPRE_1,
  .hpre    = RCC_F1_HPRE_1,
  .latency = 2
};

unsigned int hal_cpu_clock = CLOCK;

void hal_board_init()
{
  pin_init();
  clock_init(&pll_params);
#if APPL
  backup_domain_protect(0);
  clock_init_ls();
#endif

  led_init_flags(leds, led_flags, ARRSIZ(leds));

  debug_uart_init(USART1_BASE, 115200, APB2_CLOCK, 0);
}
