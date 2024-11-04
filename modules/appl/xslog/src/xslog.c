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
#include <string.h>
#include <stdlib.h>

#include "bmos_msg_queue.h"
#include "bmos_op_msg.h"
#include "bmos_task.h"
#include "common.h"
#include "io.h"
#include "shell.h"
#include "xslog.h"
#include "xtime.h"
#include "fast_log.h"

#ifdef CONFIG_SYSLOG_LINELEN
#define SYSLOG_MAX CONFIG_SYSLOG_LINELEN
#else
#define SYSLOG_MAX 64
#endif

#ifdef CONFIG_SYSLOG_NUM_LINES
#define NSYSLOG CONFIG_SYSLOG_NUM_LINES
#else
#define NSYSLOG 64
#endif

#define OP_SYSLOG_DATA 1

static bmos_queue_t *slogq;
static bmos_queue_t *slogp;

void xsyslog(int priority, const char *format, ...);
void xdsyslog(const char *format, ...);

typedef struct _syslog_entry_t {
  xtime_ms_t ts;
  unsigned char priority;
  unsigned char pad[3];
  char e[SYSLOG_MAX];
} syslog_entry_t;

#define XDSLOG_ITEMS_LEN 128
unsigned char xdslog_mask[XDSLOG_ITEMS_LEN];

static unsigned int missed_count = 0;
static int slog_idx = 0;
static syslog_entry_t slog[NSYSLOG];
static signed char slog_pri = LOG_INFO;
static signed char slog_cons_pri = -1;

void _slog(syslog_entry_t *s)
{
  unsigned int idx;

  idx = slog_idx;
  slog_idx++;
  if (slog_idx >= NSYSLOG)
    slog_idx = 0;

  slog[idx] = *s;
}

void xvslog(int priority, const char *format, va_list ap)
{
  bmos_op_msg_t *m;
  syslog_entry_t *s;

  if (!slogp)
    return;

  m = op_msg_get(slogp);
  if (!m) {
    FAST_LOG('S', "no message for syslog", 0, 0);
    missed_count++;
    return;
  }

  s = (syslog_entry_t *)BMOS_OP_MSG_GET_DATA(m);

  (void)vsnprintf(s->e, SYSLOG_MAX, format, ap);
  s->ts = xtime_ms();
  s->priority = priority;

  op_msg_put(slogq, m, OP_SYSLOG_DATA, sizeof(syslog_entry_t));
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

static void slog_task(void *arg)
{
  for (;;) {
    bmos_op_msg_t *m;
    syslog_entry_t *s;

    m = op_msg_wait(slogq);

    s = (syslog_entry_t *)BMOS_OP_MSG_GET_DATA(m);

    if ((slog_cons_pri >= 0) && (s->priority <= slog_cons_pri)) {
      xtime_ms_t t = s->ts;
      unsigned int l = strlen(s->e) - 1;
      xprintf("%5d.%03d: %s%s", t / 1000, t % 1000, s->e,
              s->e[l] == '\n' ? "" : "\n");
    }

    if ((slog_pri >= 0) && (s->priority <= slog_pri))
      _slog(s);

    op_msg_return(m);
  }
}

void xslog_init()
{
  slogq = queue_create("syslog", QUEUE_TYPE_TASK);

  slogp =
    op_msg_pool_create("slog", QUEUE_TYPE_TASK, 2, sizeof(syslog_entry_t));

  task_init(slog_task, 0, "slog", 10, 0, 256);
}

static void _slog_dump(int count)
{
  int last = slog_idx - 1;
  int first, i, idx;
  xtime_ms_t t;
  syslog_entry_t *e;

  if (last < 0)
    last = last + NSYSLOG;

  if (count > NSYSLOG)
    count = NSYSLOG;

  first = last - count + 1;
  if (first < 0)
    first += NSYSLOG;

  for (i = 0; i < count; i++) {
    int l;
    idx = i + first;
    if (idx >= NSYSLOG)
      idx -= NSYSLOG;
    e = &slog[idx];
    if (e->e[0] == 0)
      continue;
    l = strlen(e->e) - 1;
    t = e->ts;
    xprintf("%5d.%03d: %s%s", t / 1000, t % 1000, e->e,
            e->e[l] == '\n' ? "" : "\n");
  }

  if (missed_count > 0)
    xprintf("missed %d\n", missed_count);
}

static void slog_set_pri(unsigned int pri)
{
  if (pri > LOG_DEBUG)
    pri = LOG_DEBUG;

  slog_pri = pri;
}

static void slog_set_cons_pri(int pri)
{
  if (pri > LOG_DEBUG)
    pri = LOG_DEBUG;

  slog_cons_pri = pri;
}

/* *INDENT-OFF* */
static const char *pri_names[] = { "emerg",   "alert",  "crit", "err",
                                   "warning", "notice", "info", "debug" };
/* *INDENT-ON* */

static void slog_show_pri()
{
  if (slog_pri >= ARRSIZ(pri_names))
    xprintf("invalid priority %d\n", slog_pri);
  else
    xprintf("priority: %s\n", pri_names[slog_pri]);
}

static void slog_show_cons_pri()
{
  if (slog_cons_pri < 0)
    xprintf("off\n", slog_pri);
  else if (slog_cons_pri >= ARRSIZ(pri_names))
    xprintf("invalid priority %d\n", slog_cons_pri);
  else
    xprintf("priority: %s\n", pri_names[slog_cons_pri]);
}

#define XDSLOG_ENABLE 0xff
#define XDSLOG_DISABLE 0x00

static void xdslog_enable_items(const char *items, int enable)
{
  unsigned char mask;

  if (!items)
    return;

  if (enable)
    mask = XDSLOG_ENABLE;
  else
    mask = XDSLOG_DISABLE;

  if (*items == '*')
    memset(xdslog_mask, mask, sizeof(xdslog_mask));
  else {
    unsigned int c;
    for (c = *items++; c != '\0'; c = *items++) {
      if (c >= XDSLOG_ITEMS_LEN)
        continue;
      xdslog_mask[c] = mask;
    }
  }
}

static void xdslog_show_items(void)
{
  unsigned int i;

  for (i = 0; i < XDSLOG_ITEMS_LEN; i++)
    if (xdslog_mask[i])
      xprintf("%c", i);
  xprintf("\n");
}

int cmd_slog(int argc, char *argv[])
{
  int val;
  char cmd = 'v';

  if (argc > 1)
    cmd = argv[1][0];

  switch (cmd) {
  case 'v':
    val = 10;
    if (argc > 2)
      val = atoi(argv[2]);
    _slog_dump(val);
    break;
  case 'p':
    if (argc > 2) {
      val = atoi(argv[2]);
      slog_set_pri(val);
    }
    slog_show_pri();
    break;
  case 'c':
    if (argc > 2) {
      val = atoi(argv[2]);
      slog_set_cons_pri(val);
    }
    slog_show_cons_pri();
  case 'e':
    if (argc <= 2)
      return -1;
    xdslog_enable_items(argv[2], 1);
    break;
  case 'd':
    if (argc <= 2)
      return -1;
    xdslog_enable_items(argv[2], 0);
    break;
  case 'i':
    xdslog_show_items();
    break;
  }

  return 0;
}

SHELL_CMD(slog, cmd_slog);
