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
#include "stm32_flash.h"
#include "xslog.h"

#if STM32_F7XX || STM32_F4XX
#define FLASH_TYPE1 1
#elif STM32_L4XX || STM32_L4R || STM32_G4XX || STM32_WBXX || STM32_G0XX || \
  STM32_C0XX || STM32_U0XX
#define FLASH_TYPE2 1
#elif STM32_U5XX
#define FLASH_TYPE3 1
#elif STM32_H5XX
#define FLASH_TYPE4 1
#else
#error CHECK FLASH BASE AND TYPE
#endif

#if FLASH_TYPE1
#define CONFIG_FLASH_OPTR 0
typedef struct {
  reg32_t acr;
  reg32_t keyr;
  reg32_t optkeyr;
  reg32_t sr;
  reg32_t cr;
  reg32_t flash_optcr;
  reg32_t flash_optcr1;
} stm32_flash_t;
#elif FLASH_TYPE2
#define CONFIG_FLASH_OPTR 1
typedef struct {
  reg32_t acr;
  reg32_t pdkeyr;
  reg32_t keyr;
  reg32_t optkeyr;
  reg32_t sr;
  reg32_t cr;
  reg32_t eccr;
  reg32_t pad0;
  reg32_t optr;
  reg32_t pcrop1sr;
  reg32_t pcrop1er;
  reg32_t wrp1ar;
  reg32_t wrp1br;
} stm32_flash_t;
#elif FLASH_TYPE3
#define CONFIG_FLASH_OPTR 1
typedef struct {
  reg32_t acr;
  unsigned int pad0;
  reg32_t keyr; /* NSKEYR */
  reg32_t seckeyr;
  reg32_t optkeyr;
  unsigned int pad1;
  reg32_t pdkeyr[2];
  reg32_t sr; /* NSSR */
  reg32_t secsr;
  reg32_t cr; /* NSCR */
  reg32_t seccr;
  reg32_t eccr;
  reg32_t opsr;
  unsigned int pad2[2];
  reg32_t optr;
  /* TBD */
} stm32_flash_t;
#elif FLASH_TYPE4
typedef struct {
  reg32_t acr;
  reg32_t keyr; /* NSKEYR */
  reg32_t seckeyr;
  reg32_t optkeyr;
  reg32_t nsobkkeyr;
  reg32_t secobkkeyr;
  reg32_t opsr;
  reg32_t optcr;
  reg32_t sr; /* NSSR */
  reg32_t secsr;
  reg32_t cr; /* NSCR */
  reg32_t seccr;
  reg32_t nsccr;
  reg32_t secccr;
  reg32_t privcfgr;
  reg32_t nsobkcfgr;
  reg32_t secobkcfgr;
  reg32_t hdpextr;
  reg32_t pad0;
  reg32_t optsr_cur;
} stm32_flash_t;
#endif

#define FLASH_ACR_PRFTEN BIT(8)
#define FLASH_ACR_ICEN BIT(9)
#define FLASH_ACR_DCEN BIT(10)
#define FLASH_ACR_ICRST BIT(11)
#define FLASH_ACR_DCRST BIT(12)

#if FLASH_TYPE4
#define FLASH_CR_BKSEL BIT(31)
#define FLASH_CR_STRT BIT(5)
#define FLASH_CR_PER BIT(2)
#define FLASH_CR_PG BIT(1)
#define FLASH_CR_LOCK BIT(0)
#else
#define FLASH_CR_LOCK BIT(31)
#define FLASH_CR_OPTSTRT BIT(17)
#define FLASH_CR_STRT BIT(16)
#define FLASH_CR_PER BIT(1)
#define FLASH_CR_PG BIT(0)
#endif

#if FLASH_TYPE1
#define FLASH_CR_PSIZE(page) (((page) & 0x3) << 8)
#define FLASH_CR_PNB(page) (((page) & 0x1f) << 3)
#define MIN_WRITE_POW2 2
#elif FLASH_TYPE2 || FLASH_TYPE3
#define FLASH_CR_OPTLOCK BIT(30)
#define FLASH_CR_BKER BIT(11)
#define FLASH_CR_PNB(page) (((page) & 0xff) << 3)
#define FLASH_CR_MER1 BIT(2)
#define MIN_WRITE_POW2 3
#elif FLASH_TYPE4
#define FLASH_CR_PNB(page) (((page) & 0x7f) << 6)
#define MIN_WRITE_POW2 4
#endif

#if FLASH_TYPE4
#define FLASH_SR_BSY BIT(0)
#else
#define FLASH_SR_BSY BIT(16)
#define FLASH_SR_PROGERR BIT(3)
#define FLASH_SR_OPERR BIT(1)
#define FLASH_SR_EOP BIT(0)
#endif

#define FLASH_KEYR_KEY1 0x45670123
#define FLASH_KEYR_KEY2 0xcdef89ab

#define FLASH_OPTKEYR_KEY1 0x08192a3b
#define FLASH_OPTKEYR_KEY2 0x4c5d6e7f

#if STM32_WBXX
#define FLASH ((stm32_flash_t *)0x58004000)
#elif FLASH_TYPE1
#define FLASH ((stm32_flash_t *)0x40023C00)
#elif FLASH_TYPE2 || FLASH_TYPE3 || FLASH_TYPE4
#define FLASH ((stm32_flash_t *)0x40022000)
#endif

void stm32_flash_latency(unsigned int val)
{
  reg_set_field(&FLASH->acr, 4, 0, val);
#if STM32_H5XX
  unsigned int wrhighfreq = 2;

  switch (val) {
  case 0:
  case 1:
    wrhighfreq = 0;
    break;
  case 2:
  case 3:
    wrhighfreq = 1;
    break;
  }

  reg_set_field(&FLASH->acr, 2, 4, wrhighfreq);
#endif

  (void)FLASH->acr;
}

void stm32_flash_cache_enable(unsigned int en)
{
  if (en)
    FLASH->acr |= FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
  else
    FLASH->acr &= ~(FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN);
  (void)FLASH->acr;
}

void flash_data_cache_invalidate()
{
  FLASH->acr &= ~FLASH_ACR_DCEN;
  FLASH->acr |= FLASH_ACR_DCRST;
  FLASH->acr &= ~FLASH_ACR_DCRST;
  FLASH->acr |= FLASH_ACR_DCEN;
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

  FLASH->sr = 0xffff;
}

int flash_erase(unsigned int start, unsigned int count)
{
  unsigned int cr;
  unsigned int page;
  unsigned int flash_bank = 0;

#if FLASH_TYPE2
  if (start & FLASH_START_BANK1) {
    flash_bank |= FLASH_CR_BKER;
    start &= ~FLASH_START_BANK1;
  }
#endif

  clear_status();

  flash_lock();
  flash_unlock();

  for (page = start; page < start + count; page++) {
    cr = FLASH->cr;

    cr &= ~0x7ffff;
    cr |= FLASH_CR_STRT | FLASH_CR_PNB(page) | FLASH_CR_PER | flash_bank;

    FLASH->cr = cr;

    while (FLASH->sr & FLASH_SR_BSY)
      ;
  }

  flash_lock();

  return 0;
}

int flash_program(unsigned int addr, const void *data, unsigned int len)
{
  unsigned int i;
  const unsigned int *d = (const unsigned int *)data;
  unsigned int *a;

  if (addr & (BIT(MIN_WRITE_POW2) - 1))
    return -1;
  if (len & (BIT(MIN_WRITE_POW2) - 1))
    return -2;
  if (((unsigned int)data) & 0x3)
    return -3;

  clear_status();

#if !CONFIG_FLASH_NO_LOCK
  flash_lock();
  flash_unlock();
#endif

  len >>= MIN_WRITE_POW2;

#if FLASH_TYPE1
  FLASH->cr = FLASH_CR_PSIZE(2);
#elif FLASH_TYPE4
  FLASH->cr &= ~0x3f;
#else
  FLASH->cr &= ~0x7;
#endif
  FLASH->cr |= FLASH_CR_PG;

  a = (unsigned int *)addr;

  for (i = 0; i < len; i++) {
#if FLASH_TYPE1
    *a++ = *d++;
#elif FLASH_TYPE4
    *a++ = *d++;
    *a++ = *d++;
    *a++ = *d++;
    *a++ = *d++;
#else
    *a++ = *d++;
    *a++ = *d++;
#endif

    while (FLASH->sr & FLASH_SR_BSY)
      ;
  }

  FLASH->cr &= ~FLASH_CR_PG;

  flash_lock();

#if CONFIG_FLASH_OUTPUT_ERROR
  {
    unsigned int sr = FLASH->sr;
    if (sr)
      xprintf("flash error addr:%08x sr:%08x\n", addr, sr);
  }
#endif

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

#if 0
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

#if FLASH_TEST_CMD
static char data[256];

int cmd_flash(int argc, char *argv[])
{
  unsigned int addr, sect, count;
  int err, bank;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'e':
    if (argc < 5)
      return -1;
    sect = atoi(argv[2]);
    count = atoi(argv[3]);
    bank = atoi(argv[4]);
    if (bank != 0)
      sect |= FLASH_START_BANK1;

    flash_erase(sect, count);
    break;
  case 'p':
    if (argc < 3)
      return -1;
    addr = strtoul(argv[2], 0, 16);
    for (int i = 0; i < sizeof(data); i++)
      data[i] = i & 0xff;
    err = flash_program(addr, data, sizeof(data));
    xprintf("err %d\n", err);
    break;
  }

  return 0;
}

SHELL_CMD(flash, cmd_flash);
#endif

#endif
