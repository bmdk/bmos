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
#include "stm32_hal.h"
#include "hal_uart.h"

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


#define USART2_BASE 0x40004400
#define APB2_CLOCK 120000000
#if BMOS
/* *INDENT-OFF* */
uart_t debug_uart =
{ "ser2", (void *)USART2_BASE, APB2_CLOCK, 38, STM32_UART_FIFO,
  "u2pool", "u2tx" };
/* *INDENT-ON* */
#endif

/* Orange */
static const gpio_handle_t leds[] = { GPIO(4, 1) };

unsigned int hal_cpu_clock = 240000000;

void hal_board_init()
{
  /* clock, pin and device initialization is done from the M7 code */
  led_init(leds, ARRSIZ(leds));

  debug_uart_init((void *)USART2_BASE, 115200, APB2_CLOCK, 0);
}
