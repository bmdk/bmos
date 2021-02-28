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
#include "xslog.h"

#if STM32_F767 || STM32_F746 || STM32_F429 || STM32_F411 || STM32_F401
#define FLASH_TYPE1 1
#elif STM32_L4XX || STM32_L4R || STM32_G4XX
#define FLASH_TYPE2 1
#else
#error CHECK FLASH BASE AND TYPE
#endif

#if FLASH_TYPE1
#define CONFIG_FLASH_OPTR 0
typedef struct {
  unsigned int acr;
  unsigned int keyr;
  unsigned int optkeyr;
  unsigned int sr;
  unsigned int cr;
  unsigned int flash_optcr;
  unsigned int flash_optcr1;
} stm32_flash_t;
#else
#define CONFIG_FLASH_OPTR 1
typedef struct {
  unsigned int acr;
  unsigned int pdkeyr;
  unsigned int keyr;
  unsigned int optkeyr;
  unsigned int sr;
  unsigned int cr;
  unsigned int eccr;
  unsigned int pad0;
  unsigned int optr;
  unsigned int pcrop1sr;
  unsigned int pcrop1er;
  unsigned int wrp1ar;
  unsigned int wrp1br;
} stm32_flash_t;
#endif

#define FLASH_CR_LOCK BIT(31)
#define FLASH_CR_OPTSTRT BIT(17)
#define FLASH_CR_STRT BIT(16)
#define FLASH_CR_PER BIT(1)
#define FLASH_CR_PG BIT(0)

#if FLASH_TYPE1
#define FLASH_CR_PSIZE(page) (((page) & 0x3) << 8)
#define FLASH_CR_PNB(page) (((page) & 0x1f) << 3)
#elif FLASH_TYPE2
#define FLASH_CR_OPTLOCK BIT(30)
#define FLASH_CR_PNB(page) (((page) & 0xff) << 3)
#define FLASH_CR_MER1 BIT(2)
#endif

#define FLASH_SR_BSY BIT(16)
#define FLASH_SR_PROGERR BIT(3)
#define FLASH_SR_OPERR BIT(1)
#define FLASH_SR_EOP BIT(0)

#define FLASH_KEYR_KEY1 0x45670123
#define FLASH_KEYR_KEY2 0xcdef89ab

#define FLASH_OPTKEYR_KEY1 0x08192a3b
#define FLASH_OPTKEYR_KEY2 0x4c5d6e7f

#if FLASH_TYPE1
#define FLASH ((volatile stm32_flash_t *)0x40023C00)
#elif FLASH_TYPE2
#define FLASH ((volatile stm32_flash_t *)0x40022000)
#endif

static void flash_unlock()
{
  FLASH->keyr = FLASH_KEYR_KEY1;
  FLASH->keyr = FLASH_KEYR_KEY2;
}

static void flash_lock()
{
  FLASH->cr |= FLASH_CR_LOCK;
}

static void clear_status()
{
  while (FLASH->sr & FLASH_SR_BSY)
    ;

  FLASH->sr = 0xffff;
}

int flash_erase(unsigned int start, unsigned int count)
{
  unsigned int cr;
  unsigned int page;

  clear_status();

  flash_lock();
  flash_unlock();

  for (page = start; page < start + count; page++) {
    cr = FLASH->cr;

    cr &= ~0x7ffff;
    cr |= FLASH_CR_STRT | FLASH_CR_PNB(page) | FLASH_CR_PER;

    FLASH->cr = cr;

    while (FLASH->sr & FLASH_SR_BSY)
      ;
  }

  flash_lock();

  return 0;
}

void flash_status()
{
  xprintf("sr: %08x\n", FLASH->sr);
}

int flash_program(unsigned int addr, const void *data, unsigned int len)
{
  unsigned int i;
  const unsigned int *d = (const unsigned int *)data;
  unsigned int *a;
  unsigned int sr;

  clear_status();

  flash_lock();
  flash_unlock();

  /* TODO find out why this causes a crash */
#if 0
  if ((((unsigned int)data) & 0x7) != 0)
    return -1;
#endif
#if 0
  if ((len & 0x7) != 0)
    return -1;
  if ((addr & 0x7) != 0)
    return -1;
#endif

#if FLASH_TYPE1
  len >>= 2;
  FLASH->cr = FLASH_CR_PSIZE(2);
  FLASH->cr |= FLASH_CR_PG;
#else
  len >>= 3;
  FLASH->cr &= ~0x7;
  FLASH->cr |= FLASH_CR_PG;
#endif

  a = (unsigned int *)addr;

  for (i = 0; i < len; i++) {
#if FLASH_TYPE1
    *a++ = *d++;
    asm volatile ("dsb");
#else
    *a++ = *d++;
    *a++ = *d++;
#endif

    while (FLASH->sr & FLASH_SR_BSY)
      ;
  }

  FLASH->cr &= ~FLASH_CR_PG;

  flash_lock();

  sr = FLASH->sr;
  if (sr)
    xslog(LOG_ERR, "flash error addr:%08x sr:%08x\n", addr, sr);

  return 0;
}

#if CONFIG_FLASH_OPTR
#include <stdlib.h>

#include "shell.h"
#include "io.h"

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

static int flash_cmd(int argc, char *argv[])
{
  unsigned int bit;

  if (argc < 2)
    return -1;

  bit = atoi(argv[1]);

  flash_optr_setbit(bit, 1);

  return 0;
}

SHELL_CMD(flash, flash_cmd);

#endif
