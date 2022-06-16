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
#include "xslog.h"

typedef struct {
  reg32_t acr;
  reg32_t pecr;
  reg32_t pdkeyr;
  reg32_t pekeyr;
  reg32_t prgkeyr;
  reg32_t optkeyr;
  reg32_t sr;
  reg32_t optr;
  reg32_t wrprot[2];
} stm32_flash_t;

#define FLASH_ACR_PRE_READ BIT(6)
#define FLASH_ACR_DISAB_BUF BIT(5)
#define FLASH_ACR_RUN_PD BIT(4)
#define FLASH_ACR_SLEEP_PD BIT(3)
#define FLASH_ACR_PRFTEN BIT(1)
#define FLASH_ACR_LATENCY BIT(0)

#define FLASH_PECR_NZ_DISABLE BIT(23)
#define FLASH_PECR_OBL_LAUNCH BIT(18)
#define FLASH_PECR_ERRIE BIT(17)
#define FLASH_PECR_EOPIE BIT(16)
#define FLASH_PECR_PARALLELBANK BIT(15)
#define FLASH_PECR_FPRG BIT(10)
#define FLASH_PECR_ERASE BIT(9)
#define FLASH_PECR_FIT BIT(8)
#define FLASH_PECR_DATA BIT(4)
#define FLASH_PECR_PROG BIT(3)
#define FLASH_PECR_OPTLOCK BIT(2)
#define FLASH_PECR_PRGLOCK BIT(1)
#define FLASH_PECR_PELOCK BIT(0)

#define FLASH_SR_FWWERR BIT(17)
#define FLASH_SR_NOTZEROERR BIT(16)
#define FLASH_SR_RDERR BIT(13)
#define FLASH_SR_OPTVERR BIT(11)
#define FLASH_SR_SIZERR BIT(10)
#define FLASH_SR_PGAERR BIT(9)
#define FLASH_SR_WRPERR BIT(8)
#define FLASH_SR_READY BIT(3)
#define FLASH_SR_ENDHV BIT(2)
#define FLASH_SR_EOP BIT(1)
#define FLASH_SR_BSY BIT(0)

#define FLASH_PDKEYR_KEY1 0x04152637
#define FLASH_PDKEYR_KEY2 0xfafbfcfd

#define FLASH_PEKEYR_KEY1 0x89abcdef
#define FLASH_PEKEYR_KEY2 0x02030405

#define FLASH_PRGKEYR_KEY1 0x8c9daebf
#define FLASH_PRGKEYR_KEY2 0x13141516

#define FLASH_OPTKEYR_KEY1 0xfbead9c8
#define FLASH_OPTKEYR_KEY2 0x24252627

#define FLASH ((stm32_flash_t *)0x40022000)

void stm32_flash_latency(unsigned int val)
{
  reg_set_field(&FLASH->acr, 1, 0, val);
}

void stm32_flash_cache_enable(unsigned int en)
{
  if (en)
    FLASH->acr |= FLASH_ACR_PRFTEN | FLASH_ACR_PRE_READ;
  else
    FLASH->acr |= FLASH_ACR_DISAB_BUF;
}

static void flash_pecr_unlock()
{
  if ((FLASH->pecr & FLASH_PECR_PELOCK) == 0)
    return;

  FLASH->pekeyr = FLASH_PEKEYR_KEY1;
  FLASH->pekeyr = FLASH_PEKEYR_KEY2;
}

static void flash_unlock()
{
  flash_pecr_unlock();

  if ((FLASH->pecr & FLASH_PECR_PRGLOCK) == 0)
    return;

  FLASH->prgkeyr = FLASH_PRGKEYR_KEY1;
  FLASH->prgkeyr = FLASH_PRGKEYR_KEY2;
}

static void flash_lock()
{
#if !CONFIG_FLASH_NO_LOCK
  FLASH->pecr |= FLASH_PECR_PELOCK;
#endif
}

static void clear_status()
{
  while (FLASH->sr & FLASH_SR_BSY)
    ;

  FLASH->sr = 0xffffffff;
}

#define PAGE_SIZE 128
#define FLASH_BASE 0x8000000

int flash_erase(unsigned int start, unsigned int count)
{
  unsigned int page;

  clear_status();
  flash_unlock();

  for (page = start; page < start + count; page++) {
    unsigned int *addr;

    FLASH->pecr |= FLASH_PECR_ERASE | FLASH_PECR_PROG;

    addr = (unsigned int *)(page * PAGE_SIZE + FLASH_BASE);

    *addr = 0;

    while (FLASH->sr & FLASH_SR_BSY)
      ;
  }

  flash_lock();

  return 0;
}

static RAMFUNC int _flash_program_page(unsigned int *a,
                                       const unsigned int *d,
                                       unsigned int blocks)
{
  unsigned int i, j;

  for (i = 0; i < blocks; i++) {
    unsigned int count = 100000;

    FLASH->pecr |= FLASH_PECR_PROG | FLASH_PECR_FPRG;

    for (j = 0; j < 16; j++)
      *a++ = *d++;

    while (FLASH->sr & FLASH_SR_BSY)
      if (count-- == 0)
        return -1;

    if (FLASH->sr & FLASH_SR_EOP)
      FLASH->sr = FLASH_SR_EOP;

    FLASH->pecr &= ~(FLASH_PECR_PROG | FLASH_PECR_FPRG);
  }

  return 0;
}

int flash_program_page(unsigned int addr, const void *data, unsigned int len)
{
  const unsigned int *d = (const unsigned int *)data;
  unsigned int *a = (unsigned int *)addr;
  int err;
  unsigned int saved;

  clear_status();

#if !CONFIG_FLASH_NO_LOCK
  flash_unlock();
#endif

  len >>= 6;
  saved = interrupt_disable();
  err = _flash_program_page(a, d, len);
  interrupt_enable(saved);
  if (err < 0)
    return -1;

  flash_lock();

#if CONFIG_FLASH_OUTPUT_ERROR
  {
    unsigned int sr = FLASH->sr & ~0xf;
    if (sr)
      debug_printf("flash error addr:%08x sr:%08x\n", addr, sr);
  }
#endif

  return 0;
}

#define HPAGE_MASK (BIT(6) - 1)

int flash_program(unsigned int addr, const void *data, unsigned int len)
{
  unsigned int i;
  const unsigned int *d = (const unsigned int *)data;
  unsigned int *a;

  if ((addr & HPAGE_MASK) == 0 && (len & HPAGE_MASK) == 0)
    return flash_program_page(addr, data, len);

  clear_status();

#if !CONFIG_FLASH_NO_LOCK
  flash_unlock();
#endif

  len >>= 2;
  FLASH->pecr |= FLASH_PECR_PROG;

  a = (unsigned int *)addr;

  for (i = 0; i < len; i++) {
    *a++ = *d++;

    while (FLASH->sr & FLASH_SR_BSY)
      ;
  }

  flash_lock();

#if CONFIG_FLASH_OUTPUT_ERROR
  {
    unsigned int sr = FLASH->sr & ~0xf;
    if (sr)
      debug_printf("flash error addr:%08x sr:%08x\n", addr, sr);
  }
#endif

  return 0;
}

#if 1
#include <stdlib.h>

#include "shell.h"
#include "io.h"

#if 0
static void flash_opt_unlock()
{
  flash_unlock();

  FLASH->optkeyr = FLASH_OPTKEYR_KEY1;
  FLASH->optkeyr = FLASH_OPTKEYR_KEY2;
}

void flash_optr_setbit(unsigned int bit, unsigned int en)
{
  unsigned int optr = FLASH->optr;
  unsigned int mask;

  if (bit > 31)
    return;

  while (FLASH->sr & FLASH_SR_BSY)
    ;

  flash_opt_unlock();

  mask = BIT(bit);

  if (en)
    optr |= mask;
  else
    optr &= ~mask;

  FLASH->optr = optr;

  FLASH->cr |= FLASH_CR_OPTSTRT;

  while (FLASH->sr & FLASH_SR_BSY)
    ;
}
#endif

#if FLASH_DEBUG
static char data[128] __attribute__((aligned(4)));

int cmd_flash(int argc, char *argv[])
{
  int err;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'e':
    flash_erase(128, 1);
    break;
  case 'p':
    for (int i = 0; i < sizeof(data); i++)
      data[i] = i & 0xff;
    err = flash_program(0x8004000, data, sizeof(data));
    xprintf("err %d\n", err);
    break;
  }

  return 0;
}

SHELL_CMD(flash, cmd_flash);
#endif
#endif
