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
  char e[SYSLOG_MAX];
} syslog_entry_t;

static unsigned int missed_count = 0;
static int slog_idx = 0;
static syslog_entry_t slog[NSYSLOG];
static unsigned char slog_pri = LOG_INFO;

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

  if (priority > slog_pri)
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

    _slog(s);

    op_msg_return(m);
  }
}

void xslog_init()
{
  slogq = queue_create("syslog", QUEUE_TYPE_TASK);

  slogp =
    op_msg_pool_create("slog", QUEUE_TYPE_TASK, 2, sizeof(syslog_entry_t));

  task_init(slog_task, 0, "slog", 10, 0, 1024);
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
  }

  return 0;
}

SHELL_CMD(slog, cmd_slog);
