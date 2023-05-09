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

#include <fast_log.h>
#if CONFIG_FAST_LOG_ENABLE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal_int.h"
#include "hal_time.h"
#include "shell.h"
#include "io.h"

#define FAST_LOG_ITEMS_LEN 128
unsigned char fast_log_mask[FAST_LOG_ITEMS_LEN];
char fast_log_enabled;
int fast_log_stop_count = -1;

typedef struct fast_log_entry_t {
  unsigned int tus;
  const char    *fmt;
  unsigned long v1;
  unsigned long v2;
} fle_t;

#ifdef CONFIG_FAST_LOG_SHIFT
#define FAST_LOG_SHIFT (CONFIG_FAST_LOG_SHIFT)
#else
#define FAST_LOG_SHIFT 9
#endif

#define FAST_LOG_SIZE (1 << FAST_LOG_SHIFT)
#define FAST_LOG_MASK (FAST_LOG_SIZE - 1)
static unsigned int fast_log_index = 0;
static fle_t fast_log_entries[FAST_LOG_SIZE];

void fast_log(const char *fmt, unsigned long v1, unsigned long v2)
{
  unsigned int idx, saved;
  fle_t   *e;

  saved = interrupt_disable();
  if ((fast_log_stop_count > 0) && (--fast_log_stop_count == 0))
    fast_log_enable(0);

  idx = fast_log_index;
  fast_log_index = (fast_log_index + 1) & FAST_LOG_MASK;
  interrupt_enable(saved);

  e = &fast_log_entries[idx];

  e->tus = hal_time_us();
  e->fmt = fmt;
  e->v1 = v1;
  e->v2 = v2;
}

static void dputs(int debug, const char *str)
{
  if (debug)
    debug_puts(str);
  else
    xputs(str);
}

void fast_log_dump(unsigned int ent, int debug)
{
  unsigned int i;
  char line[64];
  int count;
  unsigned int last = 0, lastidx;

  if (ent > FAST_LOG_SIZE)
    ent = FAST_LOG_SIZE;

  lastidx = fast_log_index;

  for (i = 0; i < ent; i++) {
    unsigned int idx;
    fle_t *e;

    idx = (i + lastidx - ent) & FAST_LOG_MASK;
    e = &fast_log_entries[idx];

    /* skip time zero (initial entry) */
    if (e->tus == 0)
      continue;

    snprintf(line, sizeof(line), "%6u(%04u) ", e->tus, e->tus - last);
    dputs(debug, line);
    count = snprintf(line, sizeof(line), e->fmt, e->v1, e->v2);
    dputs(debug, line);
    if (line[count - 1] != '\n')
      dputs(debug, "\n");
    last = e->tus;
  }
}

#define FAST_LOG_ENABLE 0xff
#define FAST_LOG_DISABLE 0x00

static void fast_log_enable_items(const char *items, int enable)
{
  unsigned char mask;

  if (!items)
    return;

  if (enable)
    mask = FAST_LOG_ENABLE;
  else
    mask = FAST_LOG_DISABLE;

  if (*items == '*')
    memset(fast_log_mask, mask, sizeof(fast_log_mask));
  else {
    unsigned int c;
    for (c = *items++; c != '\0'; c = *items++) {
      if (c >= FAST_LOG_ITEMS_LEN)
        continue;
      fast_log_mask[c] = mask;
    }
  }
}

static void fast_log_show_items(void)
{
  unsigned int i;

  for (i = 0; i < FAST_LOG_ITEMS_LEN; i++)
    if (fast_log_mask[i])
      xprintf("%c", i);
  xprintf("\n%sabled\n", fast_log_enabled ? "en" : "dis");
}

void fast_log_init(const char *enable_items)
{
  fast_log_enable(strlen(enable_items) != 0);
  fast_log_enable_items(enable_items, 1);
}

int cmd_fast_log(int argc, char *argv[])
{
  unsigned int count = 16;
  char cmd = 's';

  if (argc > 1)
    cmd = argv[1][0];

  switch (cmd) {
  case 's':
    if (argc > 2)
      count = atoi(argv[2]);

    fast_log_dump(count, 0);
    break;
  case 'e':
    if (argc <= 2)
      return -1;
    fast_log_enable_items(argv[2], 1);
    break;
  case 'd':
    if (argc <= 2)
      return -1;
    fast_log_enable_items(argv[2], 0);
    break;
  case 'i':
    fast_log_show_items();
    break;
  case 't':
    fast_log_enable(1);
    break;
  case 'p':
    fast_log_enable(0);
    break;
  }

  return 0;
}

SHELL_CMD_H(fast_log, cmd_fast_log, "fast_log control\n\n"
            " i: show enabled items\n"
            " p: stop\n"
            " t: start\n"
            " d <item>|*: disable items\n"
            " e <item>|*: enable items\n"
            " s <count>: show entries"
            );
#endif
