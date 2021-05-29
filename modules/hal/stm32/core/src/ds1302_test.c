/* Copyright (c) 2019-2021 Brian Thomas Murphy
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
#include <string.h>

#include "ds1302.h"
#include "shell.h"
#include "io.h"
#include "stm32_hal.h"
#include "hal_rtc.h"

void delay_f(void)
{
  delay(1000);
}

static ds1302_t ds = {
  GPIO(0, 3),
  GPIO(2, 0),
  GPIO(2, 3),
  delay_f
};

void ds1302_get_time(rtc_time_t *t)
{
  unsigned char data[7];

  ds1302_init(&ds);

  ds1302_read_regs(&ds, data, sizeof(data));

  t->secs = (((data[0]) >> 4) & 0x7) * 10 + (data[0] & 0xf);
  t->mins = (((data[1]) >> 4) & 0xf) * 10 + (data[1] & 0xf);
  t->hours = (((data[2]) >> 4) & 0xf) * 10 + (data[2] & 0xf);
  t->day = (((data[3]) >> 4) & 0xf) * 10 + (data[3] & 0xf);
  t->month = (((data[4]) >> 4) & 0x1) * 10 + (data[4] & 0xf);
  t->dayno = data[5] & 0x7;
  t->year = (((data[6]) >> 4) & 0xf) * 10 + (data[6] & 0xf);
}

void ds1302_set_time(rtc_time_t *t)
{
  unsigned char data[8];

  ds1302_init(&ds);

  data[0] = ((t->secs / 10) << 4) | (t->secs % 10);
  data[1] = ((t->mins / 10) << 4) | (t->mins % 10);
  data[2] = ((t->hours / 10) << 4) | (t->hours % 10);
  data[3] = ((t->day / 10) << 4) | (t->day % 10);
  data[4] = ((t->month / 10) << 4) | (t->month % 10);
  data[5] = t->dayno;
  data[6] = ((t->year / 10) << 4) | (t->year % 10);
  /* it seems that when writing we need to write all 8 bytes */
  data[7] = 0;

  ds1302_write_regs(&ds, data, sizeof(data));
}

/* *INDENT-OFF* */
static const char *dayname[] = { "NONE", "MON", "TUE", "WED",
                                 "THU", "FRI", "SAT", "SUN" };
/* *INDENT-ON* */

int cmd_dsrtc(int argc, char *argv[])
{
  unsigned int secs, mins, hours, year, month, day, dayno;
  rtc_time_t t;

  if (argc > 1) {
    int count;

    memset(&t, 0, sizeof(t));

    count = sscanf(argv[1], "%2d:%2d:%2d",
                   &hours, &mins, &secs);
    if (count < 3)
      return -1;
    t.hours = (unsigned char)hours;
    t.mins = (unsigned char)mins;
    t.secs = (unsigned char)secs;

    if (argc > 2) {
      count = sscanf(argv[2], "%d", &dayno);
      if (count < 1)
        return -1;
      t.dayno = (unsigned char)dayno;
    }

    if (argc > 3) {
      count = sscanf(argv[3], "%2d-%2d-%2d",
                     &day, &month, &year);
      if (count < 3)
        return -1;
      t.day = (unsigned char)day;
      t.month = (unsigned char)month;
      t.year = (unsigned char)year;
    }

    ds1302_set_time(&t);
    return 0;
  }

  ds1302_get_time(&t);

  xprintf("%02d:%02d:%02d%s %s %02d-%02d-%02d\n", t.hours, t.mins, t.secs,
          t.pm ? " PM" : "", dayname[t.dayno], t.day, t.month, t.year);

  return 0;
}

SHELL_CMD(dsrtc, cmd_dsrtc);

void rtc_get_time(rtc_time_t *t)
{
  ds1302_get_time(t);
}

void rtc_set_time(rtc_time_t *t)
{
  ds1302_set_time(t);
}
