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
#include <string.h>

#include "common.h"
#include "debug_ser.h"
#include "debug_ser.h"
#include "hal_board.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "stm32_hal.h"
#include "hal_board.h"
#include "stm32_hal_gpio.h"
#include "stm32_hal_spi.h"
#include "stm32_pwr_f4xx.h"
#include "stm32_rcc_a.h"

typedef struct {
  gpio_handle_t gpio;
  unsigned char alt;
} gpio_init_tab_t;

void pin_init()
{
  enable_ahb1(0);  /* GPIOA */
  enable_ahb1(1);  /* GPIOB */
  enable_ahb1(2);  /* GPIOC */
  enable_ahb1(3);  /* GPIOD */
  enable_ahb1(5);  /* GPIOF */
  enable_ahb1(6);  /* GPIOG */
  enable_ahb1(10); /* GPIOK */

  /* USART 3 */
  enable_apb1(18);

  gpio_init_attr(GPIO(1, 10), GPIO_ATTR_STM32(0, \
                                              GPIO_SPEED_VHI, 7, GPIO_ALT));
  gpio_init_attr(GPIO(1, 11), GPIO_ATTR_STM32(0, \
                                              GPIO_SPEED_VHI, 7, GPIO_ALT));

  /* TIM1 */
  enable_apb2(0);

  /* TIM2 */
  enable_apb1(0);

  /* DMA 1 */
  enable_ahb1(21);

  /* DMA 2 */
  enable_ahb1(22);
}

#define USART1_BASE (void *)0x40011000
#define USART3_BASE (void *)0x40004800

#define APB1_CLOCK 45000000
#define APB2_CLOCK 90000000
#if BMOS
uart_t debug_uart = { "debugser", USART3_BASE, APB1_CLOCK, 39 };
#endif

static const gpio_handle_t leds[] = { GPIO(6, 6), GPIO(3,  4),
                                      GPIO(3, 5), GPIO(10, 3) };
static const led_flag_t led_flags[] = { LED_FLAG_INV, LED_FLAG_INV,
                                        LED_FLAG_INV, LED_FLAG_INV };

/* 8MHz crystal input
   180MHz CPU clock
   90MHz AHB1
   45MHz AHB2
 */
static const struct pll_params_t pll_params = {
  .src     = RCC_A_CLK_HSE_OSC,
  .pllp    = RCC_A_PLLP_2,
  .pllq    = 6,
  .pllr    = 6,
  .plln    = 180,
  .pllm    = 4,
  .hpre    = 0,
  .ppre1   = RCC_A_PPRE_4,
  .ppre2   = RCC_A_PPRE_2,
  .latency = 7
};

unsigned int hal_cpu_clock = 180000000;

void hal_board_init()
{
  pin_init();
  led_init_flags(leds, led_flags, ARRSIZ(leds));
  led_set(0, 0);
  led_set(1, 0);
  led_set(2, 0);
  led_set(3, 0);
  clock_init(&pll_params);
  debug_uart_init(USART3_BASE, 115200, APB1_CLOCK, 0);
}
