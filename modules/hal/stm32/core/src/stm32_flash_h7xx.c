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

#include <string.h>

#include "common.h"
#include "hal_common.h"
#include "hal_int.h"
#include "io.h"
#include "shell.h"

#include "stm32_hal.h"

typedef struct {
  reg32_t acr;
  reg32_t keyr;
  reg32_t optkeyr;
  reg32_t cr;
  reg32_t sr;
  reg32_t ccr;
  reg32_t optcr;
  reg32_t optsr_cur;
  reg32_t optsr_prg;
  reg32_t optccr;
} stm32_flash_h7xx_t;

#define FLASH_KEYR_KEY1 0x45670123
#define FLASH_KEYR_KEY2 0xcdef89ab

#define FLASH_CR_SNB(p) (((p) & 0x7) << 8)
#define FLASH_CR_START BIT(7)
#define FLASH_CR_FW BIT(6)
#define FLASH_CR_PSIZE(s) (((s) & 0x3) << 4)
#define FLASH_CR_BER BIT(3)
#define FLASH_CR_SER BIT(2)
#define FLASH_CR_PG BIT(1)
#define FLASH_CR_LOCK BIT(0)

#define FLASH_SR_CRCEND BIT(27)
#define FLASH_SR_DBECCERR BIT(26)
#define FLASH_SR_SNECCERR BIT(25)
#define FLASH_SR_RDSERR BIT(24)
#define FLASH_SR_RDPERR BIT(23)
#define FLASH_SR_OPPERR BIT(22)
#define FLASH_SR_INCERR BIT(21)
#define FLASH_SR_STRBERR BIT(19)
#define FLASH_SR_PGSERR BIT(18)
#define FLASH_SR_WRPERR BIT(17)
#define FLASH_SR_EOP BIT(16)
#define FLASH_SR_CRC_BSY BIT(3)
#define FLASH_SR_QW BIT(2)
#define FLASH_SR_WBNE BIT(1)
#define FLASH_SR_BSY BIT(0)

typedef struct {
  const char *name;
  unsigned char pos;
} flash_err_t;

/* *INDENT-OFF* */
flash_err_t flerr[] = {
  { "CRCEND", 27 },
  { "DBECCERR", 26 },
  { "SNECCERR", 25 },
  { "RDSERR", 24 },
  { "RDPERR", 23 },
  { "OPPERR", 22 },
  { "INCERR", 21 },
  { "STRBERR", 19 },
  { "PGSERR", 18 },
  { "WRPERR", 17 },
  { "EOP", 16 }
};
/* *INDENT-ON* */

static void flash_error_decode(unsigned int sr)
{
  unsigned int i;

  for (i = 0; i < ARRSIZ(flerr); i++) {
    flash_err_t *e = &flerr[i];
    if (BIT(e->pos) & sr)
      xprintf("%s,", e->name);
  }
  xprintf("\n");
}

#define FLASH_BASE 0x52002000
#define FLASH(_idx_) ((stm32_flash_h7xx_t *)(FLASH_BASE + (_idx_) * 0x100))
#define FLASHL FLASH(0)
#define FLASHH FLASH(1)

void stm32_flash_latency(unsigned int val)
{
  reg_set_field(&FLASHL->acr, 4, 0, val);
}

/* No flash specific cache on H7 */
void stm32_flash_cache_enable(unsigned int en)
{
}

unsigned int stm32_flash_opt()
{
  return FLASHL->optsr_cur;
}

static void flash_lock(stm32_flash_h7xx_t *flash)
{
  flash->cr |= FLASH_CR_LOCK;
}

static void flash_unlock(stm32_flash_h7xx_t *flash)
{
  /* already unlocked */
  if ((flash->cr & FLASH_CR_LOCK) == 0)
    return;
  flash->keyr = FLASH_KEYR_KEY1;
  flash->keyr = FLASH_KEYR_KEY2;
}

int _flash_erase(stm32_flash_h7xx_t *flash,
                 unsigned int start, unsigned int count)
{
  unsigned int cr;
  unsigned int page;

  flash_unlock(flash);

  for (page = start; page < start + count; page++) {
    cr = flash->cr;

    cr = FLASH_CR_START | FLASH_CR_SNB(page) | FLASH_CR_SER;

    flash->cr = cr;

    while (flash->sr & FLASH_SR_QW)
      ;
  }

  flash_lock(flash);

  return 0;
}

static stm32_flash_h7xx_t *_get_flash(unsigned int addr)
{
  if (addr >= 0x08100000)
    return FLASHH;
  else
    return FLASHL;
}

static stm32_flash_h7xx_t *_get_flash_block(unsigned int block)
{
  if (block >= 8)
    return FLASHH;
  else
    return FLASHL;
}

int flash_erase(unsigned int start, unsigned int count)
{
  stm32_flash_h7xx_t *flash = _get_flash_block(start);

  _flash_erase(flash, start, count);

  return 0;
}

void flash_status(stm32_flash_h7xx_t *flash)
{
  xprintf("sr: %08x\n", flash->sr);
}

static void flash_wait_done(stm32_flash_h7xx_t *flash)
{
  while (flash->sr & (FLASH_SR_QW | FLASH_SR_BSY | FLASH_SR_WBNE))
    ;
}

static void flash_program_word(stm32_flash_h7xx_t *flash,
                               unsigned long long *a,
                               const unsigned long long *d)
{
  unsigned int i;

  flash_wait_done(flash);

  flash->ccr |= 0x0fef0000;
  flash->cr |= FLASH_CR_PG;

  for (i = 0; i < 4; i++)
    *a++ = *d++;

  flash_wait_done(flash);

  if (flash->sr & FLASH_SR_EOP)
    flash->ccr &= ~FLASH_SR_EOP;

  flash->cr &= ~FLASH_CR_PG;
}

int flash_program(unsigned int addr, const void *data, unsigned int len)
{
  const unsigned long long *d = (const unsigned long long *)data;
  unsigned long long *a;
  unsigned int i, words, remainder;
  unsigned long long buf[4];
  int err = 0;
  stm32_flash_h7xx_t *flash = _get_flash(addr);

  words = len >> 5;
  remainder = len - (words << 5);

  a = (unsigned long long *)addr;

  flash_unlock(flash);

  for (i = 0; i < words; i++) {
#if 0
    debug_printf("a: %08x d:%08x\n", a, d);
#endif
    flash_program_word(flash, a, d);
    a += 4;
    d += 4;
  }

  if (remainder) {
    memset(buf, 0xff, sizeof(buf));
    memcpy(buf, d, remainder);
    flash_program_word(flash, a, buf);
  }

  flash_lock(flash);

  return err;
}

int flash_program_test(unsigned int addr, unsigned int len)
{
  unsigned int i, *a, sr;
  int err = 0;
  stm32_flash_h7xx_t *flash = _get_flash(addr);

  flash_unlock(flash);

  len >>= 2;
  a = (unsigned int *)addr;

  flash->cr = FLASH_CR_PG;

  for (i = 0; i < len; i++) {
    unsigned int b = i & 0xff;
    unsigned int m = (b << 24) | (b << 16) | (b << 8) | b;

    *a++ = m;

    while (flash->sr & FLASH_SR_QW)
      ;

    sr = flash->sr;
    if ((sr & 0xfffe0000) != 0) {
      flash_error_decode(sr);
      err = -1;
      goto exit;
    }
  }

  flash->cr |= FLASH_CR_FW;
  while (flash->cr & FLASH_CR_FW)
    ;

exit:
  flash_lock(flash);

  xprintf("end %08x\n", (unsigned int)a);

  return err;
}


char data[4096] __attribute__((aligned(8)));

int cmd_flash(int argc, char *argv[])
{
  int err;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
#if 0
  case 'u':
    flash_unlock();
    break;
  case 'l':
    flash_lock();
    break;
#endif
  case 'e':
    flash_erase(8, 1);
    break;
  case 'p':
    for (int i = 0; i < sizeof(data); i++)
      data[i] = i & 0xff;
    //memset(data, 0x5a, sizeof(data));
    //err = flash_program(0x08100000, data, sizeof(data));
    err = flash_program(0x8100000, data, sizeof(data));
    xprintf("err %d\n", err);
    break;
  case 'q':
    err = flash_program_test(0x08100000, 256);
    xprintf("err %d\n", err);
    break;
  }

  return 0;
}

SHELL_CMD(flash, cmd_flash);
