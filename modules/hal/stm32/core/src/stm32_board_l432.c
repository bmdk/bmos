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

void clear_button_wkup()
{
  PWR->scr = 0x1f;
}

void ext_smps_on(int en)
{
  if (en)
    PWR->cr[3] |= BIT(13);
  else
    PWR->cr[3] &= ~BIT(13);
}

int cmd_smps(int argc, char *argv[])
{
  ext_smps_on(1);

  return 0;
}

SHELL_CMD(smps, cmd_smps);

void clock_init_high();
void clock_init_low();
void clear_button_wkup();

int cmd_lp(int argc, char *argv[])
{
#if 0
  portDISABLE_INTERRUPTS();
#endif

  clock_init_low();

  reg_set_field(&PWR->cr[0], 2, 0, 2);

  set_low_power(1);
  asm volatile ("wfi");
  set_low_power(0);

  clock_init_high();

#if 0
  portENABLE_INTERRUPTS();
#endif

  return 0;
}

SHELL_CMD(lp, cmd_lp);

void pin_init_can(int en)
{
  if (en) {
    gpio_init_attr(GPIO(0, 11),
                   GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 9, GPIO_ALT));
    gpio_init_attr(GPIO(0, 12),
                   GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 9, GPIO_ALT));
  } else {
    gpio_init(GPIO(0, 11), GPIO_INPUT);
    gpio_init(GPIO(0, 12), GPIO_INPUT);
  }
}

int cmd_canconn(int argc, char *argv[])
{
  int en;

  if (argc < 2)
    return -1;

  en = atoi(argv[1]);

  pin_init_can(en);

  return 0;
}

SHELL_CMD(canconn, cmd_canconn);

void pin_init()
{
  enable_ahb2(0); /* GPIOA */
  enable_ahb2(1); /* GPIOB */
  enable_ahb2(2); /* GPIOC */
#if STM32_L496
  enable_ahb2(3); /* GPIOD */
  enable_ahb2(4); /* GPIOE */
  enable_ahb2(6); /* GPIOG */
#endif

  gpio_init_attr(GPIO(2, 13),
                 GPIO_ATTR_STM32(0, GPIO_SPEED_LOW, 0, GPIO_INPUT));

#if STM32_L496
  /* USART2 */
  enable_apb1(17);
  gpio_init_attr(GPIO(3, 5), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(3, 6),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_HIG, 7,
                                 GPIO_ALT));

  /* LPUART1 */
  enable_apb1(32);

  gpio_init_attr(GPIO(2, 0),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_LOW, 8,
                                 GPIO_ALT));
  gpio_init_attr(GPIO(2, 1),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_LOW, 8,
                                 GPIO_ALT));

#if 0
  gpio_init_attr(GPIO(6, 7), GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 8, GPIO_ALT));
  gpio_init_attr(GPIO(6, 8), GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 8, GPIO_ALT));
  gpio_init(GPIO(2, 0), GPIO_INPUT);
  gpio_init(GPIO(2, 1), GPIO_INPUT);
#endif

  /* TIM1_CH4 */
  gpio_init_attr(GPIO(4, 14), GPIO_ATTR_STM32(0, GPIO_SPEED_LOW, 1, GPIO_ALT));
#else
  /* LPUART1 */
  enable_apb1(32);

  gpio_init_attr(GPIO(0, 2),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_LOW, 8,
                                 GPIO_ALT));
  gpio_init_attr(GPIO(0, 3),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_LOW, 8,
                                 GPIO_ALT));

  /* USART1 */
  enable_apb2(14);
  gpio_init_attr(GPIO(0, 9), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(0, 10),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_HIG, 7,
                                 GPIO_ALT));
#endif

#if STM32_L452
  gpio_init_attr(GPIO(2, 10),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PD, GPIO_SPEED_LOW, 0,
                                 GPIO_INPUT));
  gpio_init_attr(GPIO(2, 11),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PD, GPIO_SPEED_LOW, 0,
                                 GPIO_INPUT));
  gpio_init_attr(GPIO(2, 12),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PD, GPIO_SPEED_LOW, 0,
                                 GPIO_INPUT));

  gpio_init(GPIO(1, 0), GPIO_OUTPUT);
#endif

#if STM32_L432
  /* SPI1 */
  enable_apb2(12);

  gpio_init_attr(GPIO(0, 7), GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 5, GPIO_ALT));
  gpio_init_attr(GPIO(0, 5), GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 5, GPIO_ALT));
#if 0
  gpio_init_attr(GPIO(0, 4), GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 5, GPIO_ALT));
#else
  gpio_init(GPIO(0, 4), GPIO_OUTPUT);
#endif
  gpio_init(GPIO(0, 6), GPIO_OUTPUT);
#elif STM32_L496
  /* SPI1 */
  enable_apb2(12);

  gpio_init_attr(GPIO(4, 13), GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 5, GPIO_ALT));
  gpio_init_attr(GPIO(4, 15), GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 5, GPIO_ALT));
#if 0
  gpio_init_attr(GPIO(4, 12), GPIO_ATTR_STM32(0, GPIO_SPEED_VHI, 5, GPIO_ALT));
#else
  gpio_init(GPIO(4, 12), GPIO_OUTPUT);
#endif
#endif

  /* CAN1 */
  enable_apb1(25);
  pin_init_can(1);

  /* SYSCFG */
  enable_apb2(0);

  /* PWR */
  enable_apb1(28);

  /* TIM 1 */
  enable_apb2(11);

  /* TIM 2 */
  enable_apb1(0);

#if STM32_L496
  /* DMA 1 */
  enable_ahb1(0);
  /* DMA 2 */
  enable_ahb1(1);
  /* ADC */
  enable_ahb2(13);
#endif

#if 0
  /* SMPS */
  gpio_init_attr(GPIO(0, 4),
                 GPIO_ATTR_STM32(0, GPIO_SPEED_LOW, 0, GPIO_OUTPUT));
  gpio_init_attr(GPIO(0, 5),
                 GPIO_ATTR_STM32(0, GPIO_SPEED_LOW, 0, GPIO_OUTPUT));
  gpio_init_attr(GPIO(0, 6),
                 GPIO_ATTR_STM32(GPIO_FLAG_PULL_PU, GPIO_SPEED_LOW, 0,
                                 GPIO_INPUT));
  gpio_init_attr(GPIO(0, 7),
                 GPIO_ATTR_STM32(0, GPIO_SPEED_LOW, 0, GPIO_OUTPUT));

#if 0
  gpio_set(GPIO(0, 4), 1);
  gpio_set(GPIO(0, 7), 1);
#endif
#endif

  stm32_exti_irq_set_edge_rising(13, 1);
  stm32_exti_irq_enable(13, 1);
  stm32_exti_ev_enable(13, 1);
  stm32_syscfg_set_exti(2, 13);

#if 0
  EXTI->r[0].pr = 0xffffffff;

  SYSCFG->exticr[3] = (2 << 4); /* EXT13 PC13 */
  EXTI->r[0].rtsr = 0;
  EXTI->r[0].ftsr |= BIT(13);
  EXTI->r[0].emr |= BIT(13);
  EXTI->r[0].imr |= BIT(13);
#endif

#if 1
  PWR->scr = 0xffffffff;
  PWR->cr[2] = 1 << 1;
#endif
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
uart_t debug_uart = { "debug", LPUART1_BASE, APB1_CLOCK, 70, STM32_UART_LP };
#if 0
uart_t debug_uart = { "debug", USART1_BASE, APB2_CLOCK, 37 };
#endif
#endif

#define LED_CMD_TEST 0xf
#define LED_CMD_SHUTDOWN 0xc
#define LED_CMD_INTEN 0xa
#define LED_CMD_SCANLIM 0xb
#define LED_CMD_SCAN 0xb
#define LED_CMD_DECODEMODE 0x9
#define LED_CMD_NOP 0x0

#define N_LED_PANELS 12

#if STM32_L432
#define LED_CS GPIO(0, 4)
#elif STM32_L496
#define LED_CS GPIO(4, 12)
#endif

#if STM32_L432 || STM32_L496
void led_disp_cmd(unsigned int addr, unsigned int cmd, unsigned int arg)
{
  unsigned int i;

  cmd = ((cmd & 0xf) << 8) + (arg & 0xff);

  gpio_set(LED_CS, 0);

  for (i = 0; i < N_LED_PANELS - addr; i++)
    stm32_hal_spi_write(SPI1_BASE, 0x0000);
  stm32_hal_spi_write(SPI1_BASE, cmd);
  for (i = 0; i < addr; i++)
    stm32_hal_spi_write(SPI1_BASE, 0x0000);
  stm32_hal_spi_wait_done(SPI1_BASE);

  gpio_set(LED_CS, 1);
}

void led_disp_init(unsigned int nleds)
{
  unsigned int i, j;

  for (i = 0; i < nleds; i++) {
    led_disp_cmd(i, LED_CMD_TEST, 0);
    led_disp_cmd(i, LED_CMD_INTEN, 0x00);
    led_disp_cmd(i, LED_CMD_SCANLIM, 0x07);
    led_disp_cmd(i, LED_CMD_SHUTDOWN, 0x1);
    led_disp_cmd(i, LED_CMD_DECODEMODE, 0);

    /* write 0 to every pixel */
    for (j = 0; j < 8; j++)
      led_disp_cmd(i, j + 1, 0x0);
  }
}
#endif

void hal_board_init()
{
  pin_init();
#if CLOCK_HS
  clock_init();
#endif
#if 1
  backup_domain_protect(0);
  clock_init_ls();
#endif
  led_init();

  debug_uart_init(LPUART1_BASE, 115200, APB1_CLOCK, STM32_UART_LP);

#if STM32_L432 || STM32_L496
  stm32_hal_spi_init(SPI1_BASE, 16);

  gpio_set(LED_CS, 1);

  led_disp_init(N_LED_PANELS);
#endif
}
