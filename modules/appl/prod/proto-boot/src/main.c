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
#include "shell.h"
#include "stm32_flash.h"
#include "stm32_hal.h"
#include "xmodem.h"
#include "xtime.h"

#define FLASH_BASE 0x08000000

#if STM32_H7XX
#define APP_START 128
#define APP_LEN 128
#elif STM32_H5XX
#define APP_START 128
#define APP_LEN 128
#elif STM32_F4XX
#define APP_START 64
#define APP_LEN 64
#elif STM32_F072
#define APP_START 16
#define APP_LEN 32
#elif BOARD_F103DEB
#define APP_START 10
#define APP_LEN 22
#elif STM32_F0XX
#define APP_START 4
#define APP_LEN 12
#elif STM32_G030
#define APP_START 10
#define APP_LEN 32
#elif STM32_C0XX
#define APP_START 16
#define APP_LEN 16
#elif STM32_L0XX
#define APP_START 16
#define APP_LEN 32
#else
#define APP_START 32
#define APP_LEN 64
#endif

#define APP_BASE (FLASH_BASE + APP_START * 1024)

typedef struct {
  unsigned char count;
  unsigned char size;
} flash_block_t;

#if STM32_H5XX
#define FLASH_BLKSIZE 8
#elif STM32_H7XX
#define FLASH_BLKSIZE 128
#elif STM32_L0XX
#define FLASH_BLKSIZE 128 // FIXME Bytes
#elif STM32_C0XX || STM32_G4XX || STM32_L4XX || STM32_U0XX
#define FLASH_BLKSIZE 2
#elif STM32_L4R || STM32_WBXX
#define FLASH_BLKSIZE 4
#elif STM32_U5XX
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
#elif AT32_F4XX
#define FLASH_BLKSIZE 2
#elif STM32_F3XX
#define FLASH_BLKSIZE 2
#else
#error define FLASH_BLKSIZE
#endif

static int led_state;
static xtime_ms_t last_blink;

void blink()
{
  xtime_ms_t now;

  now = xtime_ms();
  if (xtime_diff_ms(now, last_blink) >= 500) {
    led_state ^= 1;
    led_set(0, led_state);
    last_blink = now;
  }
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

static inline int _flash_erase(unsigned int start, unsigned int count);

#define H745N_M4_FLASH_BASE 0x08100000

static int xmodem_flash_erase(unsigned int addr)
{
#if BOARD_H745N
  if (addr == H745N_M4_FLASH_BASE)
    return _flash_erase(1024, 128);
#endif

  return _flash_erase(APP_START, APP_LEN);
}

static int xmodem_block(void *block_ctx, void *data, unsigned int len)
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
static const flash_block_t flash_blocks[] = { FLASH_BLOCKS };

static inline int _find_block(unsigned int ofs)
{
  unsigned int i;
  unsigned int cofs = 0;
  unsigned int blk_cnt = 0;

  for (i = 0; i < ARRSIZ(flash_blocks); i++) {
    const flash_block_t *c = &flash_blocks[i];
    unsigned int blk_ofs;

    blk_ofs = (ofs - cofs) / c->size;
    if (blk_ofs < c->count)
      return blk_cnt + blk_ofs;

    cofs += c->count * c->size;
    blk_cnt += c->count;
  }

  return -1;
}

static inline int _flash_erase(unsigned int start, unsigned int count)
{
  unsigned int s = _find_block(start);
  unsigned int e = _find_block(start + count - 1);
  unsigned int c = e - s + 1;

  if (s < 0 || e < 0) {
    xprintf("error computing start or end block (%d,%d)\n", s, e);
    return -1;
  }

#if 0
  xprintf("flash_erase block %d-%d count %d\n", s, e, c);
#endif
  return flash_erase(s, c);
}
#else
static inline int _flash_erase(unsigned int start, unsigned int count)
{
#if STM32_L0XX
  start *= 8;
  count *= 8;
#else
  if ((start % FLASH_BLKSIZE) != 0) {
    xprintf("invalid start");
    return -1;
  }
  if ((count % FLASH_BLKSIZE) != 0) {
    xprintf("invalid count");
    return -1;
  }

  start /= FLASH_BLKSIZE;
  count /= FLASH_BLKSIZE;
#endif

  flash_erase(start, count);

  return 0;
}
#endif

static void wait_tx_done(unsigned int timeout_ms)
{
  xtime_ms_t start = xtime_ms();

  while (!debug_ser_tx_done()) {
    if ((xtime_ms() - start) > timeout_ms) {
      xprintf("TIMEOUT waiting for tx done\n");
      break;
    }
  }
}

static int cmd_xmodem(int argc, char *argv[])
{
  int err;
  xtime_ms_t start = xtime_ms();

#if BOARD_H745N
  if (argc > 1 && argv[1][0] == 'm')
    bd.addr = H745N_M4_FLASH_BASE;
  else
#endif
  {
    bd.addr = APP_BASE;
  }
  bd.count = 0;
  bd.len = 0;

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
  wait_tx_done(2000);

  if (err == 0)
    boot();
  else
    xprintf("ERROR %d\n", err);

  return 0;
}

SHELL_CMD(xmodem, cmd_xmodem);

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
  xtime_ms_t start = xtime_ms();

  xputs("\nPress return to enter bootloader\n");

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

volatile xtime_ms_t systick_count;

void systick_handler(void)
{
  systick_count++;

  blink();
}

int main()
{
  INTERRUPT_OFF();
  hal_cpu_init();
  hal_board_init();
  hal_time_init();

  led_set(0, 1);

  xputs("\nBOOT\n\n");

  systick_init();
  INTERRUPT_ON();

  try_boot();

  polled_shell();

  return 0;
}
