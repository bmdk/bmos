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
#include "hal_gpio.h"
#include "io.h"
#include "stm32_hal.h"
#include "stm32_hal_gpio.h"
#include "stm32_regs.h"
#include "debug_ser.h"
#include "stm32_can.h"
#include "hal_uart.h"

void pin_init()
{
  enable_ahb2(1); /* GPIOB */
  enable_ahb2(3); /* GPIOD */

  /* USART2 Pull up on rx signal to handle that it's not connected */
  enable_apb1(17);
  gpio_init_attr(GPIO(3, 5), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(3, 6),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_HIG, 7,
                                 GPIO_ALT));

  /* reset the red led gpio from the bootloader */
  gpio_init(GPIO(1, 14), GPIO_INPUT);

  /* CAN1 */
  enable_apb1(25);
  gpio_init_attr(GPIO(3, 0), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
  gpio_init_attr(GPIO(3, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
}

#define USART2_BASE (void *)0x40004400
#define APB2_CLOCK 80000000
#if BMOS
uart_t debug_uart = { "debug", USART2_BASE, APB2_CLOCK, 38 };
#endif

void hal_board_init()
{
  pin_init();
  clock_init();
  led_init();
  debug_uart_init(USART2_BASE, 115200, APB2_CLOCK, 0);
}
