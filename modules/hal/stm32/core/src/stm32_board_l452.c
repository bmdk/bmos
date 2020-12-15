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

#include <stdlib.h>

#include "common.h"
#include "cortexm.h"
#include "debug_ser.h"
#include "hal_common.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "stm32_can.h"
#include "stm32_hal.h"
#include "stm32_hal_gpio.h"
#include "stm32_hal_spi.h"
#include "stm32_pwr.h"
#include "stm32_regs.h"
#include "stm32_pwr_lxxx.h"
#include "stm32_exti.h"

void clock_init_high();
void clock_init_low();

void pin_init()
{
  enable_ahb2(0); /* GPIOA */
  enable_ahb2(1); /* GPIOB */
  enable_ahb2(2); /* GPIOC */

  gpio_init(GPIO(0, 0), GPIO_OUTPUT);

  gpio_init_attr(GPIO(2, 13),
                 GPIO_ATTR_STM32(0, GPIO_SPEED_LOW, 0, GPIO_INPUT));

  /* LPUART1 */
  enable_apb1(32);
  gpio_init_attr(GPIO(0, 2), GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU,
                                             GPIO_SPEED_LOW, 8, GPIO_ALT));
  gpio_init_attr(GPIO(0, 3), GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU,
                                             GPIO_SPEED_LOW, 8, GPIO_ALT));

  /* USART1 */
  enable_apb2(14);
  gpio_init_attr(GPIO(0, 9), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(0, 10),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_HIG, 7,
                                 GPIO_ALT));

  /* SYSCFG */
  enable_apb2(0);

  /* PWR */
  enable_apb1(28);

  /* TIM 1 */
  enable_apb2(11);

  /* TIM 2 */
  enable_apb1(0);

  /* DMA 1 */
  enable_ahb1(0);

  /* DMA 2 */
  enable_ahb1(1);

  stm32_exti_irq_set_edge_rising(13, 1);
  stm32_exti_irq_enable(13, 1);
  stm32_exti_ev_enable(13, 1);
  stm32_syscfg_set_exti(2, 13);

  PWR->scr = 0xffffffff;
  PWR->cr[2] = 1 << 1;
}

#define USART1_BASE (void *)0x40013800
#define USART2_BASE (void *)0x40004400
#define USART3_BASE (void *)0x40004800
#define LPUART1_BASE (void *)0x40008000

#define SPI1_BASE (void *)0x40013000

#define CLOCK_HS 1
#if CLOCK_HS
#define APB2_CLOCK 80000000
#define APB1_CLOCK 80000000
#else
#define APB1_CLOCK 4000000
#define APB2_CLOCK 4000000
#endif

#if BMOS
#if 1
uart_t debug_uart = { "debug", LPUART1_BASE, APB1_CLOCK, 70, STM32_UART_LP };
#else
uart_t debug_uart = { "debug", USART1_BASE, APB2_CLOCK, 37 };
#endif
#endif

static const gpio_handle_t leds[] = { GPIO(1, 13) };

void hal_board_init()
{
  pin_init();
#if CLOCK_HS
  clock_init();
#else
  clock_init_low();
#endif
  backup_domain_protect(0);
  clock_init_ls();
  led_init(leds, ARRSIZ(leds));

#if 1
  debug_uart_init(LPUART1_BASE, 115200, APB1_CLOCK, STM32_UART_LP);
#else
  debug_uart_init(USART1_BASE, 115200, APB2_CLOCK, 0);
#endif
}
