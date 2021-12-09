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
#include "io.h"
#include "stm32_exti.h"
#include "stm32_hal.h"
#include "hal_board.h"
#include "stm32_hal_gpio.h"
#include "stm32_pwr.h"
#include "stm32_pwr_f7xx.h"
#include "stm32_rcc_a.h"

#if APPL
static void eth_pin_init()
{
  enable_ahb1(25); /* ETHMACEN */
  enable_ahb1(26); /* ETHMACTXEN */
  enable_ahb1(27); /* ETHMACRXEN */
  enable_ahb1(28); /* ETHMACPTPEN */

  stm32_syscfg_eth_phy(1);

  /* RMII pins */
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
#endif

static void pin_init()
{
  enable_ahb1(0); /* GPIOA */
  enable_ahb1(1); /* GPIOB */
  enable_ahb1(2); /* GPIOC */
  enable_ahb1(3); /* GPIOD */
  enable_ahb1(6); /* GPIOG */

#if 0
  /* USART2 Pull up on rx signal to handle that it's not connected */
  enable_apb1(17);
  gpio_init_attr(GPIO(3, 5),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_HIG, 7,
                                 GPIO_ALT));
  gpio_init_attr(GPIO(3, 6),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_HIG, 7,
                                 GPIO_ALT));
#endif

  /* PWR */
  enable_apb1(28);

  /* SYSCFG */
  enable_apb2(14);

  /* USART3 */
  enable_apb1(18);
#if 0
  gpio_init_attr(GPIO(1, 10),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_HIG, 7,
                                 GPIO_ALT));
  gpio_init_attr(GPIO(1, 11),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_HIG, 7,
                                 GPIO_ALT));
#else
  gpio_init_attr(GPIO(3, 8),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_HIG, 7,
                                 GPIO_ALT));
  gpio_init_attr(GPIO(3, 9),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_HIG, 7,
                                 GPIO_ALT));
#endif

  /* TIM2 */
  enable_apb1(0);

  /* CAN1 */
  enable_apb1(25);
  gpio_init_attr(GPIO(3, 0), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
  gpio_init_attr(GPIO(3, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));

  /* BUTTON */
  gpio_init_attr(GPIO(2, 13),
                 GPIO_ATTR_STM32(0, GPIO_SPEED_LOW, 0, GPIO_INPUT));

#if 1
  stm32_exti_irq_set_edge_rising(13, 1);
  stm32_exti_irq_enable(13, 1);
  stm32_exti_ev_enable(13, 1);
  stm32_syscfg_set_exti(2, 13);
#endif

#if APPL
  eth_pin_init();
#endif
}

#define USART2_BASE 0x40004400
#define USART3_BASE 0x40004800
#define APB2_CLOCK 54000000
#if BMOS
uart_t debug_uart = { "debugser", (void *)USART3_BASE, APB2_CLOCK, 39 };
#endif

static const gpio_handle_t leds[] = { GPIO(1, 14), GPIO(1, 7) };

/* OSC: 8 MHz
   CPU: 216 MHz
   APB1: 54 MHz (/4)
   APB2: 108 MHz (/2)
 */
static const struct pll_params_t pll_params = {
  .src     = RCC_A_CLK_HSE_OSC,
  .pllr    = 0,
  .pllp    = RCC_A_PLLP_2,
  .pllq    = 9,
  .plln    = 216,
  .pllm    = 4,
  .hpre    = 0,
  .ppre1   = RCC_A_PPRE_4,
  .ppre2   = RCC_A_PPRE_2,
  .latency = 5
};

unsigned int hal_cpu_clock = 216000000;

void hal_board_init()
{
  pin_init();
  clock_init(&pll_params);
#if 1
  backup_domain_protect(0);
  clock_init_ls();
#endif
  led_init(leds, ARRSIZ(leds));

  debug_uart_init((void *)USART3_BASE, 115200, APB2_CLOCK, 0);
}
