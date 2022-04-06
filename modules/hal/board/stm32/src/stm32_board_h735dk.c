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
#include "stm32_lcd.h"
#include "stm32_pwr.h"
#include "stm32_pwr_h7xx.h"
#include "stm32_rcc_h7.h"
#include "stm32_timer.h"

/* H7xx memory layout:
 * ITCM  0x00000000 64K
 * DTCM  0x20000000 128K
 * AXI   0x24000000 512K
 * SRAM1 0x30000000 128K
 * SRAM2 0x30020000 128K
 * SRAM3 0x30040000 32K
 * SRAM4 0x38000000 64K
 * Backup SRAM 0x38800000 4K
 */

#define LCD_X 480
#define LCD_Y 272

#define LCD_MEM_SIZE (LCD_X * LCD_Y)

unsigned char framebuf[LCD_MEM_SIZE] __attribute__((section(".framebuf")));

typedef struct {
  unsigned char gpio;
  unsigned char pin;
  unsigned char alt;
} gpio_init_tab_t;

/* *INDENT-OFF* */
static gpio_init_tab_t lcd_gpio_init[] = {
  { 4, 0, 14 },
  { 7, 3, 14 },
  { 7, 8, 14 },
  { 7, 9, 14 },
  { 7, 10, 14 },
  { 7, 11, 14 },
  { 4, 1, 14 },
  { 4, 15, 14 },

  { 1, 1, 14 },
  { 1, 0, 14 },
  { 0, 6, 14 },
  { 4, 11, 14 },
  { 7, 15, 14 },
  { 7, 4, 9 },
  { 2, 7, 14 },
  { 3, 3, 14 },

  { 6, 14, 14 },
  { 3, 0, 14 },
  { 3, 6, 14 },
  { 0, 8, 13 },
  { 4, 12, 14 },
  { 0, 3, 14 },
  { 1, 8, 14 },
  { 1, 9, 14 },

  { 6, 7, 14 }, /* LCD_CLK */
  { 4, 13, 14 }, /* LCD_DE */
  { 2, 6, 14 }, /* LCD_HSYNC */
  { 0, 4, 14 }, /* LCD_VSYNC */
};
/* *INDENT-ON* */
/* A 0 B 1 C 2 D 3
   E 4 F 5 G 6 H 7 */


void lcd_pin_init()
{
  unsigned int i;

  /* LCD */
  enable_apb3(3);

  for (i = 0; i < ARRSIZ(lcd_gpio_init); i++) {
    gpio_init_tab_t *e = &lcd_gpio_init[i];
    gpio_init_attr(GPIO(e->gpio, e->pin), GPIO_ATTR_STM32(0, \
                                                          GPIO_SPEED_VHI,
                                                          e->alt, GPIO_ALT));
  }
}

static void pin_init()
{
  enable_ahb4(0);                     /* GPIOA */
  enable_ahb4(1);                     /* GPIOB */
  enable_ahb4(2);                     /* GPIOC */
  enable_ahb4(3);                     /* GPIOD */
  enable_ahb4(4);                     /* GPIOE */
  enable_ahb4(5);                     /* GPIOF */
  enable_ahb4(6);                     /* GPIOG */
  enable_ahb4(7);                     /* GPIOH */

  enable_ahb4(21);                    /* BDMA */

  gpio_init(GPIO(2, 13), GPIO_INPUT); /* Button */

  /* USART3 */
  enable_apb1(18);
#if 0
  gpio_init_attr(GPIO(1, 10), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(1, 11), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
#else
  gpio_init_attr(GPIO(3, 8), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(3, 9), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
#endif

  /* USART2 */
  enable_apb1(17);
  gpio_init_attr(GPIO(3, 5), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(3, 6), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));

  enable_apb4(1);  /* SYSCFG */

  enable_apb1(0);  /* TIM2 */

  enable_apb1(22); /* I2C2 */

  gpio_init_attr(GPIO(5, 0),
                 GPIO_ATTR_STM32(GPIO_FLAG_OPEN_DRAIN | GPIO_FLAG_PULL_PU,
                                 GPIO_SPEED_HIG, 4, GPIO_ALT));
  gpio_init_attr(GPIO(5, 1),
                 GPIO_ATTR_STM32(GPIO_FLAG_OPEN_DRAIN | GPIO_FLAG_PULL_PU,
                                 GPIO_SPEED_HIG, 4, GPIO_ALT));

  enable_apb4(7); /* I2C4 */
  gpio_init_attr(GPIO(5, 15),
                 GPIO_ATTR_STM32(GPIO_FLAG_OPEN_DRAIN | GPIO_FLAG_PULL_PU,
                                 GPIO_SPEED_HIG, 4, GPIO_ALT));
  gpio_init_attr(GPIO(5, 14),
                 GPIO_ATTR_STM32(GPIO_FLAG_OPEN_DRAIN | GPIO_FLAG_PULL_PU,
                                 GPIO_SPEED_HIG, 4, GPIO_ALT));


  /* Ethernet */
  enable_ahb1(15); /* ETHRXEN */
  enable_ahb1(16); /* ETHTXEN */
  enable_ahb1(17); /* ETHMACEN */

  gpio_init_attr(GPIO(0, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(0, 2), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(0, 7), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(1, 10), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(1, 11), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(1, 12), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(1, 13), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(2, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(2, 4), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(2, 5), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));

  /* FDCAN1 */
  gpio_init_attr(GPIO(7, 13), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));
  gpio_init_attr(GPIO(7, 14), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 9, GPIO_ALT));

  /* LCD backlight */
  gpio_init(GPIO(6, 15), GPIO_OUTPUT);
  gpio_set(GPIO(6, 15), 1);

  gpio_init(GPIO(3, 10), GPIO_OUTPUT);
  gpio_set(GPIO(3, 10), 0);
  gpio_set(GPIO(3, 10), 1);

  gpio_init(GPIO(7, 6), GPIO_OUTPUT);
  gpio_set(GPIO(7, 6), 1);

  lcd_pin_init();
}

#define USART2_BASE 0x40004400
#define USART3_BASE 0x40004800
#define APB2_CLOCK 100000000
#if BMOS
#if 0
uart_t debug_uart = { "debugser", (void *)USART2_BASE, APB2_CLOCK, 38 };
#else
uart_t debug_uart =
{ "debugser3", (void *)USART3_BASE, APB2_CLOCK, 39, STM32_UART_FIFO,
  "u3pool",    "u3tx" };
uart_t debug_uart_2 =
{ "debugser2", (void *)USART2_BASE, APB2_CLOCK, 38, STM32_UART_FIFO,
  "u2pool",    "u2tx" };
#endif
#endif

static const gpio_handle_t leds[] = { GPIO(2, 2), GPIO(2, 3) };
static const led_flag_t led_flags[] = { LED_FLAG_INV, LED_FLAG_INV };

static struct pll_params_t clock_params = {
  .pllsrc = RCC_C_CLK_HSE,
  .divm1  = 5,
  .divn1  = 80,
  .divp1  = 1,
  .divq1  = 5,
  .divr1  = 2
};

unsigned int hal_cpu_clock = 400000000;

void hal_board_init()
{
  pin_init();
  led_init_flags(leds, led_flags, ARRSIZ(leds));
  stm32_pwr_power(PWR_CR3_SDEN);
  clock_init(&clock_params);

  /* FDCAN */
  set_fdcansel(FDCANSEL_HSE_CK);
  enable_apb1(40);

#if APPL
  backup_domain_protect(0);
  clock_init_ls(0);

  stm32_syscfg_eth_phy(SYSCFG_ETH_PHY_RMII);

  stm32_syscfg_set_exti(2, 13);
  stm32_exti_irq_set_edge_rising(13, 1);
  stm32_exti_irq_enable(13, 1);
  stm32_exti_ev_enable(13, 1);
#if 0
  stm32_pwr_wkup_en(1, 1);
#endif

#if 1
  lcd_init(LCD_X, LCD_Y);
#endif

#endif

  debug_uart_init((void *)USART3_BASE, 115200, APB2_CLOCK, 0);
}
