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
#include "stm32_pwr.h"
#include "stm32_pwr_uxxx.h"
#include "stm32_rcc_d.h"
#include "stm32_regs.h"

void pin_init()
{
  enable_ahb2(0); /* GPIOA */
  enable_ahb2(1); /* GPIOB */
  enable_ahb2(2); /* GPIOC */
  enable_ahb2(3); /* GPIOC */
  enable_ahb2(6); /* GPIOG */

  gpio_init_attr(GPIO(0, 8),
                 GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 0, GPIO_ALT));

  gpio_init_attr(GPIO(2, 13),
                 GPIO_ATTR_STM32(0, GPIO_SPEED_LOW, 0, GPIO_INPUT));

#if 0
  /* LSE */
  gpio_init_attr(GPIO(2, 14),
                 GPIO_ATTR_STM32(0, GPIO_SPEED_HIGH, 1, GPIO_ALT));
  gpio_init_attr(GPIO(2, 15),
                 GPIO_ATTR_STM32(0, GPIO_SPEED_HIGH, 1, GPIO_ALT));
#endif

#if 0
  /* LPUART1 */
  enable_apb3(6);
  gpio_init_attr(GPIO(0, 2), GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU,
                                             GPIO_SPEED_LOW, 8, GPIO_ALT));
  gpio_init_attr(GPIO(0, 3), GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU,
                                             GPIO_SPEED_LOW, 8, GPIO_ALT));
#endif

  /* USART1 */
  enable_apb2(14);
  gpio_init_attr(GPIO(0, 9), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(0, 10),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_HIG, 7,
                                 GPIO_ALT));

  /* FDCAN */
  enable_apb1(41);
  /* PD0 - CAN_RX */
  gpio_init_attr(GPIO(3, 0), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
  /* PD1 - CAN_TX */
  gpio_init_attr(GPIO(3, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));

  enable_apb3(1);  /* SYSCFG */
  enable_ahb3(2);  /* PWR */
  enable_apb2(11); /* TIM 1 */
  enable_apb1(0);  /* TIM 2 */
  enable_apb3(21); /* RTC APB Clock */

  /* GPDMA */
  enable_ahb1(0);

  stm32_exti_irq_set_edge_rising(13, 1);
  stm32_exti_irq_enable(13, 1);
  stm32_exti_ev_enable(13, 1);
  /* consider a name change here */
  stm32_syscfg_set_exti(2, 13);
}

#define USART1_BASE (void *)0x40013800
#define USART2_BASE (void *)0x40004400
#define USART3_BASE (void *)0x40004800
#define LPUART1_BASE (void *)0x46002400

#define APB2_CLOCK 160000000
#define APB1_CLOCK 160000000

#if BMOS
uart_t debug_uart = { "debug", USART1_BASE, APB2_CLOCK, 61, 0 };
#endif

static const gpio_handle_t leds[] = { GPIO(2, 7), GPIO(1, 7), GPIO(6, 2) };

static const struct pll_params_t pll_params = {
  .flags  = PLL_FLAG_PLLREN | PLL_FLAG_PLLQEN,
  .pllsrc = RCC_D_CLK_MSIS,
  .divm1  = 1,
  .divn1  = 80,
  .divp1  = 2,
  .divq1  = 2,
  .divr1  = 2,
  .acr    = 4
};

unsigned int hal_cpu_clock = 160000000;

void hal_board_init()
{
  vddio2_en(1);

  pin_init();

  led_init(leds, ARRSIZ(leds));

  /* should be done in pll control */
  stm32_pwr_vos(3);
  stm32_pwr_boost(1);

  clock_init(&pll_params);

  stm32_pwr_power(STM32_PWR_POWER_SMPS);

  set_fdcansel(FDCANSEL_PLL1Q);

#if APPL
  backup_domain_protect(0);
  clock_init_ls();
#endif

  debug_uart_init(USART1_BASE, 115200, APB2_CLOCK, 0);
}
