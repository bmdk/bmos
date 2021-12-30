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
#include <stdlib.h>

#include "io.h"
#include "shell.h"
#include "xslog.h"
#include "xtime.h"

#define SYSLOG_MAX 60

void xvslog(int priority, const char *format, va_list ap)
{
  xtime_ms_t ts = xtime_ms();

  xprintf("%5d.%03d: ", ts / 1000, ts % 1000);
  xprintf(format, ap);
  xprintf("\n");
}

void xslog(int priority, const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  xvslog(priority, format, ap);
  va_end(ap);
}

void xdslog(const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  xvslog(LOG_DEBUG, format, ap);
  va_end(ap);
}

void xslog_init()
{
}
