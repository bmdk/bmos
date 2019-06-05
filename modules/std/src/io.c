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

#include "io.h"
#include "debug_ser.h"
#include "stdio.h"
#include "string.h"
#include "xassert.h"
#if BMOS
#include "bmos_op_msg.h"
#include "bmos_queue.h"
#include "bmos_syspool.h"
#include "bmos_task.h"
#endif

void debug_puts(const char *str)
{
  char c;

  while ((c = *str++)) {
    if (c == '\n')
      debug_putc('\r');
    debug_putc(c);
  }
}

#if BMOS
static bmos_queue_t *outq;
static unsigned short outop;

void io_set_output(bmos_queue_t *queue, unsigned int op)
{
  outq = queue;
  outop = (unsigned short)op;
}

static void _xputs(const char *str)
{
  int len = strlen(str);
  bmos_queue_t *oq;

  oq = (bmos_queue_t *)task_get_tls(0);
  if (!oq)
    oq = outq;

  while (len > 0) {
    unsigned int olen = 0, i;
    char *d;
    bmos_op_msg_t *m;

    m = op_msg_wait(syspool);
    XASSERT(m);

    d = BMOS_OP_MSG_GET_DATA(m);

    for (i = 0; (i < len) && (olen < SYSPOOL_SIZE); i++) {
      char c = *str;

      if (c == '\n') {
        if (olen == SYSPOOL_SIZE - 1)
          break;
        *d++ = '\r';
        olen++;
      }

      *d++ = c;
      olen++;
      str++;
    }

    len -= i;
    op_msg_put(oq, m, outop, olen);
  }
}
#endif

void xputs(const char *str)
{
#if BMOS
  if (!outq)
    debug_puts(str);
  else
    _xputs(str);
#else
  debug_puts(str);
#endif
}

int xvprintf(const char *fmt, va_list ap)
{
  int r;
  char buf[64];

  r = vsnprintf(buf, sizeof(buf), fmt, ap);

  xputs(buf);

  return r;
}

int xprintf(const char *fmt, ...)
{
  va_list ap;
  int r;

  va_start(ap, fmt);
  r = xvprintf(fmt, ap);
  va_end(ap);

  return r;
}

int debug_vprintf(const char *fmt, va_list ap)
{
  int r;
  char buf[64];

  r = vsnprintf(buf, sizeof(buf), fmt, ap);

  debug_puts(buf);

  return r;
}

int debug_printf(const char *fmt, ...)
{
  va_list ap;
  int r;

  va_start(ap, fmt);
  r = debug_vprintf(fmt, ap);
  va_end(ap);

  return r;
}
