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
  enable_ahb2(5); /* GPIOF */

  /* LPUART1 */
  enable_apb1(32);
  gpio_init_attr(GPIO(0, 2), GPIO_ATTR_STM32(0, GPIO_SPEED_LOW, 12, GPIO_ALT));
  gpio_init_attr(GPIO(0, 3), GPIO_ATTR_STM32(0, GPIO_SPEED_LOW, 12, GPIO_ALT));

#if APPL
#if 1
  /* USART1 */
  enable_apb2(14);
  gpio_init_attr(GPIO(2, 4), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(2, 5), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
#endif

  /* SYSCFG (OPAMP) */
  enable_apb2(0);

  /* PWR */
  enable_apb1(28);

  /* TIM 1 */
  enable_apb2(11);

#if 1
  /* ADC1 ch 1 */
  gpio_init(GPIO(0, 1), GPIO_ANALOG);
#endif
  /* DAC1 ch 1 */
  gpio_init(GPIO(0, 4), GPIO_ANALOG);
#if 0
  /* opamp 5 in:PA7,out:PA6 */
  gpio_init(GPIO(0, 6), GPIO_ANALOG);
  gpio_init(GPIO(0, 7), GPIO_ANALOG);
  gpio_init(GPIO(0, 8), GPIO_ANALOG);
  gpio_init(GPIO(1, 1), GPIO_ANALOG);
  gpio_init(GPIO(1, 12), GPIO_ANALOG);
  //gpio_init(GPIO(0, 5), GPIO_ANALOG);
#endif
#define ENCODER_TIM1 1
#if ENCODER_TIM1
  /* TIM1 CH1 */
  gpio_init_attr(GPIO(2, 0), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 2, GPIO_ALT));
  /* TIM1 CH2 */
  gpio_init_attr(GPIO(2, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 2, GPIO_ALT));
#endif

  /* TIM 2 */
  enable_apb1(0);

#if 1
  /* HRTIM1 */
  enable_apb2(26);

  /* HRTIM1 A CHA1 - PA8 CHA2 - PA9 */
  gpio_init_attr(GPIO(0, 8), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 13, GPIO_ALT));
  gpio_init_attr(GPIO(0, 9), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 13, GPIO_ALT));
  /* HRTIM1 B CHA1 - PA10 CHA2 - PA11 */
  gpio_init_attr(GPIO(0, 10), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 13, GPIO_ALT));
  gpio_init_attr(GPIO(0, 11), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 13, GPIO_ALT));
#endif

  /* DAC1 */
  enable_ahb2(16);
#if 0
  /* DAC3 */
  enable_ahb2(18);
  /* DAC4 */
  enable_ahb2(19);
#endif

  /* DMA 1 */
  enable_ahb1(0);

  /* DMA 2 */
  enable_ahb1(1);

  enable_ahb2(13); /* ADC12 */

  /* FDCAN */
  enable_apb1(25);

#if 0
  gpio_init_attr(GPIO(0, 11), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
  gpio_init_attr(GPIO(0, 12), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
#endif

  /* I2C1 */
  enable_apb1(21);

#if 1
  /* CN7 17 (CN7 16 3V3)*/
  gpio_init_attr(GPIO(0, 15),
                 GPIO_ATTR_STM32(GPIO_FLAG_OPEN_DRAIN | GPIO_FLAG_PULL_PU,
                                 GPIO_SPEED_HIG, 4, GPIO_ALT)); /* SCL */
  /* CN7 21 (CN7 22 GND) */
  gpio_init_attr(GPIO(1, 7),
                 GPIO_ATTR_STM32(GPIO_FLAG_OPEN_DRAIN | GPIO_FLAG_PULL_PU,
                                 GPIO_SPEED_HIG, 4, GPIO_ALT)); /* SDA */
#endif

  gpio_init_attr(GPIO(2, 13),
                 GPIO_ATTR_STM32(0, GPIO_SPEED_LOW, 0, GPIO_INPUT));

  stm32_exti_irq_set_edge_rising(13, 1);
  stm32_exti_irq_enable(13, 1);
  stm32_exti_ev_enable(13, 1);
  stm32_syscfg_set_exti(2, 13);
#endif
}

#define APB2_CLOCK 170000000
#define APB1_CLOCK 170000000

#if BMOS
#if 1
uart_t debug_uart = { "debug", LPUART1_BASE, APB1_CLOCK, 91,
                      STM32_UART_LP | STM32_UART_FIFO };
#endif
#if 1
uart_t debug_uart_2 = { "debug", USART1_BASE, APB2_CLOCK, 37, STM32_UART_FIFO };
#endif
#endif

static const gpio_handle_t leds[] = { GPIO(0, 5) };

static const struct pll_params_t pll_params = {
  .flags   = PLL_FLAG_PLLREN,
  .pllsrc  = RCC_PLLCFGR_PLLSRC_HSE,
  .pllm    = 6,
  .plln    = 85,
  .pllr    = PLLR_DIV_2,
  .latency = 4
};

unsigned int hal_cpu_clock = 170000000;

void hal_board_init()
{
  pin_init();
  led_init(leds, ARRSIZ(leds));
  led_set(0, 1);
  clock_init(&pll_params);
#if APPL
  backup_domain_protect(0);
  clock_init_ls(0);
#endif

#if 0
  debug_uart_init(USART1_BASE, 115200, APB2_CLOCK, 0);
#endif
#if 1
  debug_uart_init(LPUART1_BASE, 115200, APB1_CLOCK, STM32_UART_LP);
#endif
}
