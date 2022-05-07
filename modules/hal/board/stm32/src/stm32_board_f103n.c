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
#define USART2_BASE (void *)0x40004400

#define CLOCK 72000000
#define APB1_CLOCK (CLOCK / 2)
#define APB2_CLOCK CLOCK

static void pin_init(void)
{
  enable_apb2(0);  /* AFIO */
  enable_apb2(2);  /* GPIOA */
  enable_apb2(3);  /* GPIOB */
  enable_apb2(4);  /* GPIOC */

  enable_apb2(11); /* TIM1 */

  enable_apb1(17); /* USART1 */

  /* USART2 TX */
  gpio_init_attr(GPIO(0, 2), GPIO_ATTR_STM32F1(CNF_ALT_PP, MODE_OUTPUT_HIG));
  /* USART2 RX */
  gpio_init(GPIO(0, 3), MODE_INPUT);

  enable_ahb1(0);  /* DMA1 */

  enable_apb1(0);  /* TIM2 */
  enable_apb1(27); /* BKP */
  enable_apb1(28); /* PWR */

#if APPL
  /* button */
  gpio_init(GPIO(2, 13), GPIO_INPUT);
  stm32_syscfg_set_exti(2, 13);
  stm32_exti_irq_set_edge_rising(13, 0);
  stm32_exti_irq_set_edge_falling(13, 1);
  stm32_exti_irq_enable(13, 1);
#endif
}

#if BMOS
uart_t debug_uart = { "debugser", USART2_BASE, APB1_CLOCK, 38 };
#endif

static const gpio_handle_t leds[] = { GPIO(0, 5) };

struct pll_params_t pll_params = {
  .src     = RCC_F1_CLK_HSE,
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
  clock_init_ls(0);
#endif

  led_init(leds, ARRSIZ(leds));

  debug_uart_init(USART2_BASE, 115200, APB1_CLOCK, 0);
}
