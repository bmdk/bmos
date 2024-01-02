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
  enable_ahb2(3); /* GPIOD */
  enable_ahb2(4); /* GPIOE */
  enable_ahb2(6); /* GPIOG */

  /* USART2 */
  enable_apb1(17);
  gpio_init_attr(GPIO(3, 5), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(3, 6), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));

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
  /* ADC */
  enable_ahb2(13);

  /* CAN1 PD0 RX PD1 TX */
  enable_apb1(25);
  gpio_init_attr(GPIO(3, 0), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
  gpio_init_attr(GPIO(3, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
  /* Interrupts TX 19 RX0 20 RX1 21 SCE 22 */

  /* CAN2 PB12 RX PB13 TX */
  enable_apb1(26);
  gpio_init_attr(GPIO(1, 12), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 10, GPIO_ALT));
  gpio_init_attr(GPIO(1, 13), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 10, GPIO_ALT));
  /* Interrupts TX 86 RX0 87 RX1 88 SCE 89 */

  /* Button */
  gpio_init_attr(GPIO(2, 13),
                 GPIO_ATTR_STM32(0, GPIO_SPEED_LOW, 0, GPIO_INPUT));

  stm32_exti_irq_set_edge_rising(13, 1);
  stm32_exti_irq_enable(13, 1);
  stm32_exti_ev_enable(13, 1);
  stm32_syscfg_set_exti(2, 13);
}

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
uart_t debug_uart = { "debug", LPUART1_BASE, APB1_CLOCK, 70, STM32_UART_LP };
#endif

static const gpio_handle_t leds[] = { GPIO(1, 14), GPIO(1, 7), GPIO(2, 7) };

static const struct pll_params_t pll_params = {
  .flags   = PLL_FLAG_PLLREN,
  .pllsrc  = RCC_PLLCFGR_PLLSRC_MSI,
  .pllm    = 1,
  .plln    = 40,
  .pllr    = PLLR_DIV_2,
  .latency = 4
};

unsigned int hal_cpu_clock = 80000000;

void hal_board_init()
{
  pin_init();
  vddio2_en(1);
#if CLOCK_HS
  clock_init(&pll_params);
#endif
#if APPL
  backup_domain_protect(0);
  clock_init_ls(0);
#endif
  led_init(leds, ARRSIZ(leds));

  debug_uart_init(LPUART1_BASE, 115200, APB1_CLOCK, STM32_UART_LP);
}
