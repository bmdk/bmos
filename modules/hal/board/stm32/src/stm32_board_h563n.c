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
#include "hal_board.h"
#include "hal_common.h"
#include "hal_gpio.h"
#include "hal_rtc.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "stm32_exti.h"
#include "stm32_h5xx.h"
#include "stm32_hal.h"
#include "stm32_hal_gpio.h"
#include "stm32_hal_uart.h"
#include "stm32_pwr.h"
#include "stm32_pwr_h5xx.h"
#include "stm32_rcc_h5.h"
#include "stm32_timer.h"

static void pin_init()
{
  enable_ahb2(0);                     /* GPIOA */
  enable_ahb2(1);                     /* GPIOB */
  enable_ahb2(2);                     /* GPIOC */
  enable_ahb2(3);                     /* GPIOD */
  enable_ahb2(4);                     /* GPIOE */
  enable_ahb2(5);                     /* GPIOF */
  enable_ahb2(6);                     /* GPIOG */

  gpio_init(GPIO(2, 13), GPIO_INPUT); /* Button */

  /* USART3 */
  enable_apb1(18);
  gpio_init_attr(GPIO(3, 8), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(3, 9), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));

  enable_apb1(0);  /* TIM2 */
  enable_apb2(11); /* TIM1 */

  enable_apb3(1);  /* SBS */

  enable_apb3(21); /* RTC APB CLK */

#if 0
  enable_apb1(11); /* WWDG2 */

  /* TIM1_CH1 */
  gpio_init_attr(GPIO(4, 9), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 1, GPIO_ALT));
  /* TIM1_CH2 */
  gpio_init_attr(GPIO(4, 11), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 1, GPIO_ALT));
  /* TIM1_CH3 */
  gpio_init_attr(GPIO(4, 13), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 1, GPIO_ALT));

  /* TIM4 */
  enable_apb1(2);

  gpio_init_attr(GPIO(1, 7), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 2, GPIO_ALT));

  enable_apb1(22); /* I2C2 */

  gpio_init_attr(GPIO(5, 0),
                 GPIO_ATTR_STM32(GPIO_FLAG_OPEN_DRAIN | GPIO_FLAG_PULL_PU,
                                 GPIO_SPEED_HIG, 4, GPIO_ALT));
  gpio_init_attr(GPIO(5, 1),
                 GPIO_ATTR_STM32(GPIO_FLAG_OPEN_DRAIN | GPIO_FLAG_PULL_PU,
                                 GPIO_SPEED_HIG, 4, GPIO_ALT));
#endif

  /* Ethernet */
  enable_ahb1(21); /* ETHRXEN */
  enable_ahb1(20); /* ETHTXEN */
  enable_ahb1(19); /* ETHMACEN */

  gpio_init_attr(GPIO(0, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(0, 2), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(0, 7), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(1, 15), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(2, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(2, 4), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(2, 5), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(6, 11), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(6, 13), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));

  /* CAN1 PD0 RX PD1 TX */
  enable_apb1(41); /* FDCAN 1,2 Enable */
  gpio_init_attr(GPIO(3, 0), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
  gpio_init_attr(GPIO(3, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
  /* Interrupts 39 (40) */

  /* CAN2 PB12 RX */
  gpio_init_attr(GPIO(1, 12), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));

#if 0
  /* PB13 TX - on h563 nucleo PB13 is connected to a USB CC1 -
     a power delivery signal and requires some solder bridge work to be
     connected */
  gpio_init_attr(GPIO(1, 13), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
#else
  /* PB6 TX */
  gpio_init_attr(GPIO(1, 6), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
#endif
  /* Interrupts 109 (110) */
}

#define APB1_CLOCK 250000000
#if BMOS
uart_t debug_uart =
{ "debugser3", USART3_BASE, APB1_CLOCK, 60, STM32_UART_FIFO,
  "u3pool",    "u3tx" };
#endif

/* Green - PB0, Yellow - PF4, Red - PG4 */
static const gpio_handle_t leds[] = { GPIO(1, 0), GPIO(5, 4), GPIO(6, 4) };

static const struct pll_params_t clock_params = {
  .flags   = PLL_FLAG_PLLPEN | PLL_FLAG_PLLREN | PLL_FLAG_PLLQEN,
  .pllsrc  = RCC_D_CLK_HSE,
  .divm1   = 4,
  .divn1   = 250,
  .divp1   = 2,
  .divq1   = 2,
  .divr1   = 2,
  .latency = 5
};

unsigned int hal_cpu_clock = 250000000;

void hal_board_init()
{
  pin_init();
  led_init(leds, ARRSIZ(leds));

  /* should be done in pll control */
  stm32_pwr_vos(3); /* VOS 0 */

  clock_init(&clock_params);

  set_fdcansel(FDCANSEL_HSE_CK);

#if APPL
  backup_domain_protect(0);
  clock_init_ls(0);

  rtc_init(1);

  stm32_syscfg_eth_phy(SYSCFG_ETH_PHY_RMII);

  stm32_syscfg_set_exti(2, 13);
  stm32_exti_irq_set_edge_rising(13, 1);
  stm32_exti_irq_enable(13, 1);
  stm32_exti_ev_enable(13, 1);
#endif

  debug_uart_init(USART3_BASE, 115200, APB1_CLOCK, 0);
}
