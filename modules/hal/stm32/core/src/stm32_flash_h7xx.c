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

#include <string.h>

#include "common.h"
#include "hal_int.h"
#include "io.h"
#include "shell.h"

#include "stm32_hal.h"

typedef struct {
  unsigned int acr;
  unsigned int keyr1;
  unsigned int optkeyr;
  unsigned int cr1;
  unsigned int sr1;
  unsigned int ccr1;
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

#define FLASH ((volatile stm32_flash_h7xx_t *)0x52002000)

static void flash_lock()
{
  FLASH->cr1 |= FLASH_CR_LOCK;
}

static void flash_unlock()
{
  /* already unlocked */
  if ((FLASH->cr1 & FLASH_CR_LOCK) == 0)
    return;
  FLASH->keyr1 = FLASH_KEYR_KEY1;
  FLASH->keyr1 = FLASH_KEYR_KEY2;
}

int flash_erase(unsigned int start, unsigned int count)
{
  unsigned int cr;
  unsigned int page;

  flash_unlock();

  for (page = start; page < start + count; page++) {
    cr = FLASH->cr1;

    cr = FLASH_CR_START | FLASH_CR_SNB(page) | FLASH_CR_SER;

    FLASH->cr1 = cr;

    while (FLASH->sr1 & FLASH_SR_QW)
      ;
  }

  flash_lock();

  return 0;
}

void flash_status()
{
  xprintf("sr1: %08x\n", FLASH->sr1);
}

static void flash_wait_done(void)
{
  while (FLASH->sr1 & (FLASH_SR_QW | FLASH_SR_BSY | FLASH_SR_WBNE))
    ;
}

static void flash_program_word(unsigned long long *a,
                               const unsigned long long *d)
{
  unsigned int i;

  flash_wait_done();

  FLASH->ccr1 |= 0x0fef0000;
  FLASH->cr1 |= FLASH_CR_PG;

  for (i = 0; i < 4; i++)
    *a++ = *d++;

  flash_wait_done();

  if (FLASH->sr1 & FLASH_SR_EOP)
    FLASH->ccr1 &= ~FLASH_SR_EOP;

  FLASH->cr1 &= ~FLASH_CR_PG;
}

int flash_program(unsigned int addr, const void *data, unsigned int len)
{
  const unsigned long long *d = (const unsigned long long *)data;
  unsigned long long *a;
  unsigned int i, words, remainder;
  unsigned long long buf[4];
  int err = 0;

  words = len >> 5;
  remainder = len - (words << 5);

  a = (unsigned long long *)addr;

  flash_unlock();

  for (i = 0; i < words; i++) {
    flash_program_word(a, d);
    a += 4;
    d += 4;
  }

  if (remainder) {
    memset(buf, 0xff, sizeof(buf));
    memcpy(buf, d, remainder);
    flash_program_word(a, buf);
  }

  flash_lock();

  return err;
}

int flash_program_test(unsigned int addr, unsigned int len)
{
  unsigned int i, *a, sr;
  int err = 0;

  flash_unlock();

  len >>= 2;
  a = (unsigned int *)addr;

  FLASH->cr1 = FLASH_CR_PG;

  for (i = 0; i < len; i++) {
    unsigned int b = i & 0xff;
    unsigned int m = (b << 24) | (b << 16) | (b << 8) | b;

    *a++ = m;

    while (FLASH->sr1 & FLASH_SR_QW)
      ;

    sr = FLASH->sr1;
    if ((sr & 0xfffe0000) != 0) {
      flash_error_decode(sr);
      err = -1;
      goto exit;
    }
  }

  FLASH->cr1 |= FLASH_CR_FW;
  while (FLASH->cr1 & FLASH_CR_FW)
    ;

exit:
  flash_lock();

  xprintf("end %08x\n", (unsigned int)a);

  return err;
}


char data[4096];

int cmd_flash(int argc, char *argv[])
{
  int err;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'u':
    flash_unlock();
    break;
  case 'l':
    flash_lock();
    break;
  case 'e':
    flash_erase(1, 1);
    break;
  case 'p':
    for (int i = 0; i < sizeof(data); i++)
      data[i] = i & 0xff;
    //memset(data, 0x5a, sizeof(data));
    err = flash_program(0x08020000, data, sizeof(data));
    xprintf("err %d\n", err);
    break;
  case 'q':
    err = flash_program_test(0x08020000, 256);
    xprintf("err %d\n", err);
    break;
  }

  return 0;
}

SHELL_CMD(flash, cmd_flash);
