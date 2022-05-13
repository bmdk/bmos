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
#include <stdlib.h>

#include "common.h"
#include "hal_common.h"
#include "hal_int.h"
#include "io.h"
#include "shell.h"
#include "xslog.h"

#define CONFIG_FLASH_OPTR 0
typedef struct {
  reg32_t acr;
  reg32_t keyr;
  reg32_t optkeyr;
  reg32_t sr;
  reg32_t cr;
  reg32_t ar;
  reg32_t obr;
  reg32_t wrpr;
} stm32_flash_t;

#define FLASH_CR_LOCK BIT(7)
#define FLASH_CR_STRT BIT(6)
#define FLASH_CR_PER BIT(1)
#define FLASH_CR_PG BIT(0)

#define FLASH_SR_BSY BIT(0)
#define FLASH_SR_PROGERR BIT(2)
#define FLASH_SR_EOP BIT(5)

#define FLASH_KEYR_KEY1 0x45670123
#define FLASH_KEYR_KEY2 0xcdef89ab

#define FLASH_OPTKEYR_KEY1 0x08192a3b
#define FLASH_OPTKEYR_KEY2 0x4c5d6e7f

#define FLASH ((stm32_flash_t *)0x40022000)

#define FLASH_ACR_PRFTBE BIT(4)

void stm32_flash_latency(unsigned int val)
{
  reg_set_field(&FLASH->acr, 3, 0, val);
}

void stm32_flash_cache_enable(unsigned int en)
{
  if (en)
    FLASH->acr |= FLASH_ACR_PRFTBE;
  else
    FLASH->acr &= ~FLASH_ACR_PRFTBE;
}

static void flash_unlock()
{
  FLASH->keyr = FLASH_KEYR_KEY1;
  FLASH->keyr = FLASH_KEYR_KEY2;
}

static void flash_lock()
{
#if !CONFIG_FLASH_NO_LOCK
  FLASH->cr |= FLASH_CR_LOCK;
#endif
}

static void clear_status()
{
  while (FLASH->sr & FLASH_SR_BSY)
    ;

  FLASH->sr = 0xffffffff;
}

#if STM32_F072 || STM32_F3XX
#define PAGE_SIZE 2048
#else
#define PAGE_SIZE 1024
#endif

int flash_erase(unsigned int start, unsigned int count)
{
  unsigned int page;

  clear_status();

  flash_lock();
  flash_unlock();

  for (page = start; page < start + count; page++) {

    FLASH->cr |= FLASH_CR_PER;
    FLASH->ar = 0x8000000 + page * PAGE_SIZE;
    FLASH->cr |= FLASH_CR_STRT;

    while (FLASH->sr & FLASH_SR_BSY)
      ;
  }

  FLASH->cr &= ~(FLASH_CR_PER | FLASH_CR_STRT);
  flash_lock();

  return 0;
}

int flash_program(unsigned int addr, const void *data, unsigned int len)
{
  unsigned int i;
  volatile unsigned short *a;
  const unsigned short *d = (const unsigned short *)data;

  clear_status();

#if !CONFIG_FLASH_NO_LOCK
  flash_lock();
  flash_unlock();
#endif

  len >>= 1;
  FLASH->cr |= FLASH_CR_PG;

  a = (unsigned short *)addr;

  for (i = 0; i < len; i++) {
    unsigned short data = *d++;

    *a = data;

    while (FLASH->sr & FLASH_SR_BSY)
      ;

    if (*a++ != data)
      break;
  }

  FLASH->cr &= ~FLASH_CR_PG;

  flash_lock();

#if CONFIG_FLASH_OUTPUT_ERROR
  {
    unsigned int sr = FLASH->sr & 0x1f;
    if (sr) {
      xprintf("flash error addr:%08x sr:%08x\n", addr, sr);
      return -1;
    }
  }
#endif

  return 0;
}
