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

#ifndef IO_H
#define IO_H

#include <stdarg.h>

#define CONFIG_IO_CHECK 1

#if CONFIG_IO_CHECK
#define __CHECK_PRINTF__ __attribute__ ((format (printf, 1, 2)))
#else
#define __CHECK_PRINTF__
#endif

void xputs(const char *str);
int xprintf(const char *fmt, ...) __CHECK_PRINTF__;
int xvprintf(const char *fmt, va_list ap);

static inline int xprintf_nocheck(const char *fmt, ...)
{
  va_list ap;
  int rc;

  va_start(ap, fmt);
  rc = xvprintf(fmt, ap);
  va_end(ap);

  return rc;
}

void debug_puts(const char *str);
int debug_vprintf(const char *fmt, va_list ap);
int debug_printf(const char *fmt, ...) __CHECK_PRINTF__;

#ifdef BMOS
#include "bmos_queue.h"

void io_set_output(bmos_queue_t *queue, unsigned int op);
#endif

#endif
