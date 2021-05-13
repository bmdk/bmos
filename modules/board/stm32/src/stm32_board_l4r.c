/* Copyright (c) 2019 Brian Thomas Murphy
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
#include "hal_gpio.h"
#include "hal_uart.h"
#include "io.h"
#include "stm32_hal.h"
#include "stm32_hal_gpio.h"
#include "stm32_pwr_lxxx.h"
#include "stm32_regs.h"

void pin_init()
{
  enable_ahb2(1); /* GPIOB */
  enable_ahb2(2); /* GPIOC */
  enable_ahb2(3); /* GPIOD */
  enable_ahb2(6); /* GPIOG */

  /* USART2 Pull up on rx signal to handle that it's not connected */
  enable_apb1(17);
  gpio_init_attr(GPIO(3, 5), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(3, 6),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_HIG, 7,
                                 GPIO_ALT));

  /* LPUART1 */
  enable_apb1(32);
  gpio_init_attr(GPIO(6, 7), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 8, GPIO_ALT));
  gpio_init_attr(GPIO(6, 8),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_HIG, 8,
                                 GPIO_ALT));

  /* TIM 2 */
  enable_apb1(0);

  /* PWR */
  enable_apb1(28);
}

#define USART2_BASE (void *)0x40004400
#define LPUART1_BASE (void *)0x40008000
#define APB1_CLOCK 80000000
#define APB2_CLOCK 80000000
#if BMOS
uart_t debug_uart = { "debug", LPUART1_BASE, APB1_CLOCK, 70, STM32_UART_LP };
#endif

// Red, Green, Blue
static const gpio_handle_t leds[] = { GPIO(1, 14), GPIO(2, 7), GPIO(1, 7) };

void hal_board_init()
{
  pin_init();
  vddio2_en(1);
  clock_init();
  led_init(leds, ARRSIZ(leds));

  debug_uart_init(LPUART1_BASE, 115200, APB1_CLOCK, STM32_UART_LP);
}
