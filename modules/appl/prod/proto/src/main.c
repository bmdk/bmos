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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmos_msg_queue.h"
#include "bmos_op_msg.h"
#include "bmos_sem.h"
#include "bmos_syspool.h"
#include "bmos_task.h"
#include "common.h"
#include "cortexm.h"
#include "debug_ser.h"
#include "fast_log.h"
#include "hal_board.h"
#include "hal_common.h"
#include "hal_gpio.h"
#include "hal_int.h"
#include "hal_time.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "stm32_exti.h"
#include "stm32_hal.h"
#include "stm32_wdog.h"
#include "xassert.h"
#include "xslog.h"
#include "xtime.h"
#include "kvlog.h"
#include "onewire.h"
#include "mshell.h"

#if BOARD_F429D || BOARD_F746D || BOARD_H735DK
#define LCD_DEMO 1
#endif

static int led_state = 0;
static xtime_ms_t last_blink = 0;
#define LONG_WAIT 6000
#define SHORT_WAIT 100
static xtime_diff_ms_t wait = LONG_WAIT;

void blink()
{
  xtime_ms_t now;

  now = xtime_ms();
  if (xtime_diff_ms(now, last_blink) >= wait) {
    led_state ^= 1;
    led_set(0, led_state);
    last_blink = now;
    if (wait == LONG_WAIT)
      wait = SHORT_WAIT;
    else
      wait = LONG_WAIT;
  }
}

#ifndef CONFIG_BUTTON_INT
#define BUTTON_INT 1
#else
#define BUTTON_INT CONFIG_BUTTON_INT
#endif

#if BUTTON_INT
#if BOARD_C031N || BOARD_U083N
#define BUTTON_EXTI 13
#define BUTTON_IRQ 7 /* EXTI4_15 */
#elif BOARD_F103N
#define BUTTON_EXTI 13
#define BUTTON_IRQ 40
#elif STM32_H7XX || STM32_F767 || STM32_L4XX || STM32_G4XX
#define BUTTON_EXTI 13
#define BUTTON_IRQ 40
#elif STM32_U5XX || STM32_H5XX
#define BUTTON_EXTI 13
#define BUTTON_IRQ 24 /* EXTI13 */
#elif BOARD_F411BP || BOARD_F401BP || BOARD_F401BP64 || BOARD_F407DEB || \
  BOARD_F407DEBM || BOARD_F4D || STM32_F1XX || AT32_F4XX
#define BUTTON_EXTI 0
#define BUTTON_IRQ 6
#elif BOARD_G030DEB
#define BUTTON_EXTI 0
#define BUTTON_IRQ 5 /* EXTI0_1 */
#elif BOARD_F103DEB
#define BUTTON_EXTI 0
#define BUTTON_IRQ 6 /* EXTI0 */
#elif BOARD_F429D || BOARD_F469D || BOARD_F746D || BOARD_L4RN || BOARD_WB55N || \
  BOARD_WB55USB
/* FIXME - check this */
#define BUTTON_EXTI 11
#define BUTTON_IRQ 40
#else
#error Error button is enabled but missing config.
#endif
#endif


#if BUTTON_INT
void button_int(void *data)
{
  stm32_exti_irq_ack(BUTTON_EXTI);
  debug_puts("X\n");
}
#endif

#if BLINK_TASK
static void blink_task(void *arg)
{
  for (;;) {
    task_delay(1000);
    led_set(1, 1);
    task_delay(1000);
    led_set(1, 0);
  }
}
#endif

void systick_hook(void)
{
#if CONFIG_TIMER_16BIT
  hal_time_us_update();
#endif
  blink();
#if WDOG_KICK
  stm32_wdog_kick();
#endif
}

bmos_queue_t *syspool;

extern uart_t debug_uart_2;

#if LCD_DEMO
void set_lcd(unsigned int start, unsigned int width, unsigned int height);

#if BOARD_F429D
#define LCD_X 240
#define LCD_Y 320
#else
#define LCD_X 480
#define LCD_Y 272
#endif

void lcd_task(void *arg)
{
  unsigned int i = 0;

  for (;;) {
    set_lcd(i++, LCD_X, LCD_Y);
    task_delay(10);
  }
}
#endif

void task_spi_clock();
void task_i2c_clock();
void task_net();
void task_led();
void task_can();
void tusb_cdc_init();
void adc_init();
void adc_init_dma();
void esp_init();

int hrtim_init();
int dacs_init();
void opamp_init_all();

int main()
{
  INTERRUPT_OFF();
  hal_cpu_init();
  hal_init();
  hal_board_init();
  hal_time_init();
  fast_log_init("");

  led_set(0, 1);

  debug_puts("\nAPPL\n\n");

  xslog_init();

  xslog(LOG_INFO, "starting");

#if CONFIG_KVLOG_ENABLE
  kv_init();
#endif

#if BUTTON_INT
  stm32_exti_irq_ack(BUTTON_EXTI);
  irq_register("ext", button_int, 0, BUTTON_IRQ);
#endif

#if BLINK_TASK
  task_init(blink_task, NULL, "blink", 2, 0, 128);
#endif

#if CONFIG_ENABLE_ADC
  adc_init();
#elif CONFIG_ENABLE_ADC_DMA
  adc_init_dma();
#endif

  mshell_init("sh1rx", 0);
#if !BOARD_H745N && !BOARD_H745NM4 && STM32_H7XX || BOARD_G474N
  mshell_add_uart(&debug_uart_2, 115200, 1, 0);
#endif
  mshell_add_uart(&debug_uart, 115200, 0, 0);

#if BOARD_F411BP || BOARD_F401BP || BOARD_F401BP64
  tusb_cdc_init();
#endif

#if ONE_WIRE
  esp_init();
#endif

#if BOARD_G474N
  hrtim_init();
  dacs_init();
  opamp_init_all();
#endif

#if LCD_DEMO
  task_init(lcd_task, NULL, "lcd", 2, 0, 512);
#endif

#if I2C_DEMO
  task_init(task_i2c_clock, NULL, "clk", 2, 0, 288);
#endif

#if BOARD_H743WA
  task_init(task_spi_clock, NULL, "spiclk", 2, 0, 256);
#endif

#if CONFIG_LWIP
  task_init(task_net, NULL, "net", 4, 0, 1280);
#endif

#if WS2811
  task_init(task_led, NULL, "lcd", 4, 0, 512);
#endif

#if STM32_G4XX || BOARD_H735DK || BOARD_H745N || BOARD_U575N || \
  BOARD_F103BP || AT32_F403BP || CONFIG_CAN_TEST
  task_init(task_can, NULL, "can", 4, 0, 256);
#endif

  syspool = op_msg_pool_create("sys", QUEUE_TYPE_TASK, SYSPOOL_COUNT,
                               SYSPOOL_SIZE);

  systick_init();

  task_start();

  xputs("OOPS\n");

  for (;;)
    ;

  polled_shell();

  return 0;
}
