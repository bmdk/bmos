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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "cortexm.h"
#include "debug_ser.h"
#include "hal_board.h"
#include "hal_gpio.h"
#include "hal_int.h"
#include "hal_time.h"
#include "io.h"
#include "shell.h"
#include "stm32_flash.h"
#include "stm32_hal.h"
#include "stm32_regs.h"
#include "xmodem.h"
#include "xtime.h"

#if STM32_H7XX
#define APP_BASE 0x08020000
#else
#define APP_BASE 0x08008000
#endif

void blink()
{
}

static void boot();

static void polled_shell(void)
{
  shell_t sh;

  shell_init(&sh, "> ");

  for (;;) {
    int c = debug_getc();
    if (c >= 0)
      shell_input(&sh, c);
  }
}

static xmodem_data_t xmdat;

typedef struct {
  unsigned int addr;
  unsigned int count;
  unsigned int len;
} xmodem_bd_t;

static xmodem_bd_t bd;

static int xmodem_block(void *block_ctx, void *data, unsigned int len)
{
  xmodem_bd_t *bd = (xmodem_bd_t *)block_ctx;

  bd->count++;
  bd->len += len;

  flash_program(bd->addr, data, len);

  bd->addr += len;

  return 0;
}

static int cmd_xmodem(int argc, char *argv[])
{
  int err;
  xtime_ms_t start = xtime_ms();

  memset(&bd, 0, sizeof(bd));
  bd.addr = APP_BASE;

#if STM32_H7XX
  flash_erase(1, 1);
#elif STM32_L4R
  flash_erase(8, 8);
#elif STM32_F429 || STM32_F411 || STM32_F401 || STM32_F4XX
  flash_erase(2, 4);
#elif STM32_F767 || STM32_F746
  flash_erase(1, 4);
#else
  flash_erase(16, 16);
#endif

  xmdat.putc = debug_putc;
  xmdat.block = xmodem_block;
  xmdat.block_ctx = &bd;

  while (xtime_ms() - start < 2000)
    ;

  start = xtime_ms();
  xmodem_start(&xmdat);

  for (;;) {
    int c;

    c = debug_getc();
    if (c < 0 && (xtime_ms() - start) > 1000) {
      err = xmodem_data(&xmdat, 0, 0);
      if (err <= 0)
        break;
      start = xtime_ms();
    } else if (c >= 0) {
      err = xmodem_data(&xmdat, &c, 1);
      if (err <= 0)
        break;
      start = xtime_ms();
    }
  }

  /* make sure we flush the last character */
  debug_putc(0);

  if (err == 0)
    boot();
  else
    xprintf("ERROR %d\n", err);

  return 0;
}

SHELL_CMD(xmodem, cmd_xmodem);

void systick_init();

typedef void call (void);

#define APP_ENTRY(base) (call *)(*(unsigned int *)((base) + 4))

static int app_entry_valid(call *c)
{
  unsigned int entry = (unsigned int)c;

  if (entry <= APP_BASE || entry == 0xffffffffUL)
    return 0;

  return 1;
}

int cmd_boot(int argc, char *argv[])
{
  call *app = APP_ENTRY(APP_BASE);

  if (!app_entry_valid(app)) {
    xprintf("application invalid\n");
    return -1;
  }

  interrupt_disable();
  app();

  return 0;
}

SHELL_CMD(boot, cmd_boot);

void boot()
{
  call *app = APP_ENTRY(APP_BASE);

  if (!app_entry_valid(app))
    return;

  interrupt_disable();
  app();
}

WEAK int bl_enter(void)
{
  int c;
  xtime_ms_t start = hal_time_us();

  xputs("\nPress return to enter bootloader\n");

  while (hal_time_us() - start < 1500000) {
    c = debug_getc();
    if (c == '\r')
      return 1;
  }

  return 0;
}

void try_boot()
{
  call *app = APP_ENTRY(APP_BASE);

  if (!app_entry_valid(app))
    return;

  if (bl_enter())
    return;

  interrupt_disable();
  app();
}

unsigned int systick_count = 0;

void systick_handler(void)
{
  systick_count++;
}

int main()
{
  interrupt_disable();
  hal_cpu_init();
  hal_board_init();
  hal_time_init();

  led_set(0, 1);

  xputs("\nBOOT\n\n");

  systick_init();
  interrupt_enable(0);

  try_boot();

  polled_shell();

  return 0;
}
