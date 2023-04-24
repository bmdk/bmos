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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "shell.h"
#include "io.h"
#include "hal_rtc.h"

#if STM32_UXXX || STM32_G0XX
typedef struct {
  reg32_t tr;
  reg32_t dr;
  reg32_t ssr;
  reg32_t isr;
  reg32_t prer;
  reg32_t wutr;
  reg32_t cr;
  reg32_t privcfgr;
  reg32_t seccfgr;
  reg32_t wpr;
  reg32_t calr;
  reg32_t shiftr;
  reg32_t tstr;
  reg32_t tsdr;
  reg32_t tsssr;
  reg32_t alrmar;
  reg32_t alrmassr;
  reg32_t alrmbr;
  reg32_t alrmbssr;
  reg32_t sr;
  reg32_t msr;
  reg32_t smsr;
  reg32_t scr;
  reg32_t alrabinr;
  reg32_t alrbbinr;
} stm32_rtc_t;
#else
typedef struct {
  reg32_t tr;
  reg32_t dr;
  reg32_t cr;
  reg32_t isr;
  reg32_t prer;
  reg32_t wutr;
  reg32_t calibr;
  reg32_t alrmar;
  reg32_t alrmbr;
  reg32_t wpr;
  reg32_t ssr;
  reg32_t shiftr;
  reg32_t tstr;
  reg32_t tsdr;
  reg32_t tsssr;
  reg32_t calr;
  reg32_t tafcr;
  reg32_t alrmassr;
  reg32_t alrmbssr;
  reg32_t or;
  reg32_t bkp[32];
} stm32_rtc_t;
#endif

#if STM32_H7XX
#define RTC_BASE 0x58004000
#elif STM32_UXXX
#define RTC_BASE 0x46007800
#else
#define RTC_BASE 0x40002800
#endif

#define RTC ((stm32_rtc_t *)RTC_BASE)

#define RTC_ISR_INIT BIT(7)

#define RTC_TR_PM BIT(22)

#define RTC_ISR_INIT BIT(7)
#define RTC_ISR_INITF BIT(6)

#define RTC_CR_BKP BIT(18)
#define RTC_CR_SUB1H BIT(17)
#define RTC_CR_ADD1H BIT(16)

static inline unsigned int _get_field(
  unsigned int val, unsigned int width, unsigned int pos)
{
  unsigned int mask = (BIT(width) - 1);

  return (val >> pos) & mask;
}

/* *INDENT-OFF* */
static const char *dayname[] = { "NONE", "MON", "TUE", "WED",
                                 "THU",  "FRI", "SAT", "SUN" };
/* *INDENT-ON* */

#define TYPE_32K 0
#define TYPE_40K 1

static void set_dividers(unsigned int prediv_a, unsigned int prediv_s)
{
  if (prediv_a == 0 || prediv_s == 0)
    return;
  RTC->prer = (((prediv_a - 1) & 0x7f) << 16) | ((prediv_s - 1) & 0x7fff);
}

static void rtc_unlock(int init)
{
  RTC->wpr = 0xca;
  RTC->wpr = 0x53;

  if (init) {
    RTC->isr |= RTC_ISR_INIT;
    while ((RTC->isr & RTC_ISR_INITF) == 0)
      ;
  }
}

static void rtc_lock(int init)
{
  if (init)
    RTC->isr &= ~RTC_ISR_INIT;

  RTC->wpr = 0xff;
}

/* default dividers for 32768 Hz */
#define DIV_A_32768 128
#define DIV_S_32768 256

#if STM32_F0XX
/* and possibly others with a 40KHz internal LS clock */
#define DIV_A_INT 80
#define DIV_S_INT 500
#else
/* default 32KHz internal LS clock */
#define DIV_A_INT 128
#define DIV_S_INT 250
#endif

void rtc_init(int external)
{
  rtc_unlock(1);

  if (external)
    set_dividers(DIV_A_32768, DIV_S_32768);
  else
    set_dividers(DIV_A_INT, DIV_S_INT);

  rtc_lock(1);
}

void rtc_set_time(rtc_time_t *t)
{
  unsigned int tr, dr;

  tr = ((((t->hours / 10) << 4) + (t->hours % 10)) << 16) +
       ((((t->mins / 10) << 4) + (t->mins % 10)) << 8) +
       (((t->secs / 10) << 4) + (t->secs % 10));

  dr = ((((t->year / 10) << 4) + (t->year % 10)) << 16) +
       (t->dayno << 13) +
       ((((t->month / 10) << 4) + (t->month % 10)) << 8) +
       (((t->day / 10) << 4) + (t->day % 10));

  rtc_unlock(1);

  RTC->tr = tr;
  RTC->dr = dr;

  rtc_lock(1);
}

void rtc_get_time(rtc_time_t *t)
{
  unsigned int tr = RTC->tr;
  unsigned int dr = RTC->dr;

  t->secs = 10 * _get_field(tr, 3, 4) + _get_field(tr, 4, 0);
  t->mins = 10 * _get_field(tr, 3, 12) + _get_field(tr, 4, 8);
  t->hours = 10 * _get_field(tr, 2, 20) + _get_field(tr, 4, 16);

  t->day = 10 * _get_field(dr, 2, 4) + _get_field(dr, 4, 0);
  t->month = 10 * _get_field(dr, 1, 12) + _get_field(dr, 4, 8);
  t->dayno = _get_field(dr, 3, 13);
  t->year = 10 * _get_field(dr, 4, 20) + _get_field(dr, 4, 16);

  t->pm = (tr >> 22) & 1;
}

static int sub_cmd_rtc_set(int argc, char *argv[])
{
  int count;
  unsigned int secs, mins, hours, year, month, day, dayno;
  rtc_time_t t;

  if (argc == 0)
    return -1;

  memset(&t, 0, sizeof(t));

  count = sscanf(argv[0], "%2d:%2d:%2d",
                 &hours, &mins, &secs);
  if (count < 3)
    return -1;
  t.hours = (unsigned char)hours;
  t.mins = (unsigned char)mins;
  t.secs = (unsigned char)secs;

  if (argc > 1) {
    count = sscanf(argv[1], "%d", &dayno);
    if (count < 1)
      return -1;
    t.dayno = (unsigned char)dayno;
  }

  if (argc > 2) {
    count = sscanf(argv[2], "%2d-%2d-%2d",
                   &day, &month, &year);
    if (count < 3)
      return -1;
    t.day = (unsigned char)day;
    t.month = (unsigned char)month;
    t.year = (unsigned char)year;
  }

  rtc_set_time(&t);

  return 0;
}

static int sub_cmd_rtc_get(int argc, char *argv[])
{
  rtc_time_t t;

  rtc_get_time(&t);

  xprintf("%02d:%02d:%02d%s %s %02d-%02d-%02d\n", t.hours, t.mins, t.secs,
          t.pm ? " PM" : "", dayname[t.dayno], t.day, t.month, t.year);

  return 0;
}

static int sub_cmd_rtc_set_dst(int argc, char *argv[])
{
  int en = 0;

  if (argc >= 1)
    en = atoi(argv[0]);

  rtc_unlock(0);

  if (en)
    RTC->cr |= RTC_CR_ADD1H;
  else
    RTC->cr |= RTC_CR_SUB1H;

  RTC->cr |= RTC_CR_BKP;

  return 0;
}

int cmd_rtc(int argc, char *argv[])
{
  char cmd = 't';

  if (argc > 1)
    cmd = argv[1][0];

  switch (cmd) {
  case 's':
    return sub_cmd_rtc_set(argc - 2, argv + 2);
  case 't':
    return sub_cmd_rtc_get(argc - 2, argv + 2);
  case 'd':
    return sub_cmd_rtc_set_dst(argc - 2, argv + 2);
  }

  return 0;
}

SHELL_CMD_H(rtc, cmd_rtc,
            "set and fetch rtc parameters\n\n"
            "rtc s hh:mm:ss [daynum dd:mm:yy]: set time/date\n"
            "rtc t                           : display time/date\n"
            "rtc d <1|0>                     : enable/disable daylight savings time"
            );
