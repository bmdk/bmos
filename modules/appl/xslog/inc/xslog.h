#ifndef _X_SYSLOG_H
#define _X_SYSLOG_H

#include <stdarg.h>

#define LOG_EMERG 0
#define LOG_ALERT 1
#define LOG_CRIT 2
#define LOG_ERR 3
#define LOG_WARNING 4
#define LOG_NOTICE 5
#define LOG_INFO 6
#define LOG_DEBUG 7

void xvslog(int priority, const char *format, va_list ap);
void xslog(int priority, const char *format, ...);
void xdslog(const char *format, ...);

void xslog_init();

#endif
