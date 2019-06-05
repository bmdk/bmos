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
