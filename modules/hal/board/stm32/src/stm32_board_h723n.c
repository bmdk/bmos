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
#include "cortexm.h"
#include "debug_ser.h"
#include "hal_board.h"
#include "hal_common.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "stm32_exti.h"
#include "stm32_hal.h"
#include "hal_board.h"
#include "stm32_hal_gpio.h"
#include "stm32_hal_uart.h"
#include "stm32_pwr.h"
#include "stm32_pwr_h7xx.h"
#include "stm32_rcc_h7.h"
#include "stm32_timer.h"

static void pin_init()
{
  enable_ahb4(0);                     /* GPIOA */
  enable_ahb4(1);                     /* GPIOB */
  enable_ahb4(2);                     /* GPIOC */
  enable_ahb4(3);                     /* GPIOD */
  enable_ahb4(4);                     /* GPIOE */
  enable_ahb4(5);                     /* GPIOF */
  enable_ahb4(6);                     /* GPIOG */

  enable_ahb4(21);                    /* BDMA */

  gpio_init(GPIO(2, 13), GPIO_INPUT); /* Button */

  /* USART3 */
  enable_apb1(18);
  gpio_init_attr(GPIO(3, 8), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(3, 9), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));

  /* USART2 */
  enable_apb1(17);
  gpio_init_attr(GPIO(3, 5), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(3, 6), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));

  enable_apb4(1);  /* SYSCFG */
  enable_apb2(0);  /* TIM1 */
  enable_apb1(0);  /* TIM2 */

  enable_apb1(22); /* I2C2 */

  gpio_init_attr(GPIO(5, 0),
                 GPIO_ATTR_STM32(GPIO_FLAG_OPEN_DRAIN | GPIO_FLAG_PULL_PU,
                                 GPIO_SPEED_HIG, 4, GPIO_ALT));
  gpio_init_attr(GPIO(5, 1),
                 GPIO_ATTR_STM32(GPIO_FLAG_OPEN_DRAIN | GPIO_FLAG_PULL_PU,
                                 GPIO_SPEED_HIG, 4, GPIO_ALT));

  /* Ethernet */
  enable_ahb1(15); /* ETHRXEN */
  enable_ahb1(16); /* ETHTXEN */
  enable_ahb1(17); /* ETHMACEN */

  gpio_init_attr(GPIO(0, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(0, 2), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(0, 7), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(1, 13), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(2, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(2, 4), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(2, 5), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(6, 11), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(6, 13), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
}

#define APB2_CLOCK 137500000
#if BMOS
uart_t debug_uart =
{ "debugser3", USART3_BASE, APB2_CLOCK, 39, STM32_UART_FIFO,
  "u3pool",    "u3tx" };
uart_t debug_uart_2 =
{ "debugser2", USART2_BASE, APB2_CLOCK, 38, STM32_UART_FIFO,
  "u2pool",    "u2tx" };
#endif

/* Green, Orange, Red */
static const gpio_handle_t leds[] = { GPIO(1, 0), GPIO(4, 1), GPIO(1, 14) };

static const struct pll_params_t clock_params = {
  .pllsrc = RCC_C_CLK_HSE,
  .divm1  = 4,
  .divn1  = 275,
  .divp1  = 1,
  .divq1  = 5,
  .divr1  = 2
};

unsigned int hal_cpu_clock = 550000000;

void hal_board_init()
{
  pin_init();
  led_init(leds, ARRSIZ(leds));
  stm32_pwr_power(PWR_CR3_LDOEN);
  clock_init(&clock_params);

#if APPL
  backup_domain_protect(0);
  clock_init_ls(0);

  stm32_syscfg_eth_phy(SYSCFG_ETH_PHY_RMII);

  stm32_syscfg_set_exti(2, 13);
  stm32_exti_irq_set_edge_rising(13, 1);
  stm32_exti_irq_enable(13, 1);
  stm32_exti_ev_enable(13, 1);
#endif

  debug_uart_init(USART3_BASE, 115200, APB2_CLOCK, 0);
}
