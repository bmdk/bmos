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

#include "common.h"
#include "cortexm.h"
#include "debug_ser.h"
#include "hal_board.h"
#include "hal_gpio.h"
#include "hal_int.h"
#include "hal_time.h"
#include "io.h"
#include "stm32_flash.h"
#include "stm32_hal.h"
#include "xmodem.h"
#include "xtime.h"

#define FLASH_BASE 0x08000000

#if STM32_H7XX
#define APP_START 128
#define APP_LEN 128
#elif STM32_F4XX
#define APP_START 64
#define APP_LEN 64
#elif STM32_F030
#define APP_START 2
#define APP_LEN 30
#elif STM32_F072
#define APP_START 16
#define APP_LEN 32
#elif STM32_F0XX || STM32_G030 || STM32_F103DEB
#define APP_START 10
#define APP_LEN 22
#else
#define APP_START 32
#define APP_LEN 64
#endif

#define APP_BASE (FLASH_BASE + APP_START * 1024)

typedef struct {
  unsigned char count;
  unsigned char size;
} flash_block_t;

#if STM32_H7XX
#define FLASH_BLKSIZE 128
#elif STM32_G4XX || STM32_L4XX
#define FLASH_BLKSIZE 2
#elif STM32_L4R || STM32_WBXX
#define FLASH_BLKSIZE 4
#elif STM32_UXXX
#define FLASH_BLKSIZE 8
#elif STM32_F4XX
#define FLASH_BLOCKS { 4, 16 }, { 1, 64 }, { 7, 128 }, \
  { 4, 16 }, { 1, 64 }, { 7, 128 }
#elif STM32_F7XX
#define FLASH_BLKSIZE 32
#elif STM32_G0XX
#define FLASH_BLKSIZE 2
#elif STM32_F072
#define FLASH_BLKSIZE 2
#elif STM32_F1XX || STM32_F0XX
#define FLASH_BLKSIZE 1
#elif STM32_F3XX
#define FLASH_BLKSIZE 2
#else
#error define FLASH_BLKSIZE
#endif

static inline int _flash_erase(unsigned int start, unsigned int count);

static xmodem_data_t xmdat;

typedef struct {
  unsigned int addr;
  unsigned int count;
  unsigned int len;
} xmodem_bd_t;

static xmodem_bd_t bd;

static inline int _flash_erase(unsigned int start, unsigned int count);

static int xmodem_flash_erase(unsigned int addr)
{
  return _flash_erase(APP_START, APP_LEN);
}

int xmodem_block(void *block_ctx, void *data, unsigned int len)
{
  xmodem_bd_t *bd = (xmodem_bd_t *)block_ctx;

  if (bd->count == 0)
    if (xmodem_flash_erase(bd->addr) < 0)
      return -1;

  bd->count++;
  bd->len += len;

  flash_program(bd->addr, data, len);

  bd->addr += len;

  return 0;
}

#ifdef FLASH_BLOCKS
#error Unsupported
#else
static inline int _flash_erase(unsigned int start, unsigned int count)
{
  if ((start % FLASH_BLKSIZE) != 0)
    return -1;
  if ((count % FLASH_BLKSIZE) != 0)
    return -1;

  start /= FLASH_BLKSIZE;
  count /= FLASH_BLKSIZE;

  flash_erase(start, count);

  return 0;
}
#endif

static void wait_tx_done(unsigned int timeout_ms)
{
  xtime_ms_t start = xtime_ms();

  while (!debug_ser_tx_done())
    if ((xtime_ms() - start) > timeout_ms)
      break;
}

typedef void call (void);

#define APP_ENTRY(base) (call *)(*(unsigned int *)((base) + 4))

static int app_entry_valid(call *c)
{
  unsigned int entry = (unsigned int)c;

  if (entry <= APP_BASE || entry == 0xffffffffUL)
    return 0;

  return 1;
}

static void try_boot()
{
  call *app = APP_ENTRY(APP_BASE);

  if (!app_entry_valid(app))
    return;

#if 0
  INTERRUPT_OFF();
#endif
  app();
}

static int xmodem_dl(void)
{
  int err;
  unsigned int count;

  bd.addr = APP_BASE;
  bd.count = 0;
  bd.len = 0;

  xmdat.block_ctx = &bd;

  delay(1000000);

  count = 0;
  xmodem_start(&xmdat);

  for (;;) {
    int c;

    c = debug_getc();
    if (c >= 0 || (++count) >= 1000000) {
      err = xmodem_data(&xmdat, c);
      if (err <= 0)
        break;
      count = 0;
    }
  }

  if (err == 0)
    try_boot();

  return err;
}

WEAK int bl_enter(void)
{
  int c;
  xtime_ms_t start = xtime_ms();

  xputs("\nPress enter to start load\n");

  wait_tx_done(20);

  for (;;)
    if (debug_getc() < 0)
      break;

  while (xtime_ms() - start < 1500) {
    c = debug_getc();
    if (c == '\r')
      return 1;
  }

  return 0;
}

volatile xtime_ms_t systick_count;

void systick_handler(void)
{
  systick_count++;
}

int main()
{
  hal_board_init();

  for (;;) {
    xmodem_dl();
    try_boot();
  }
}
