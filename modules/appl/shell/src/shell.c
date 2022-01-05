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

#include <string.h>

#include "common.h"
#include "io.h"
#include "shell.h"

extern cmd_ent_t cmd_list_start, cmd_list_end;

#define MIN(a, b) ((a) < (b)) ? (a) : (b)

#define CMD_LIST_LEN (&cmd_list_end - &cmd_list_start)

#define ANSI_SEQ_INS "\033[1@"
#define ANSI_SEQ_DEL "\033[1P"
#define ANSI_SEQ_LEFT "\033[D"
#define ANSI_SEQ_RIGHT "\033[C"
#define ANSI_SEQ_ERASE "\033[2K"

#define MAX_ARGV 8

static int _isprint(int c)
{
  if (c >= 32 && c < 127)
    return 1;
  return 0;
}

#define CMD_PER_LINE 6

#if CONFIG_SHELL_HIST
static void history_init(sh_hist_t *hist)
{
  hist->count = 0;
}

static const char *history_get(sh_hist_t *hist, unsigned int n)
{
  if (n >= hist->count)
    return 0;

  return hist->lines[n];
}

static void history_insert(sh_hist_t *hist, const char *line)
{
  unsigned int i;

  if (strlen(line) == 0)
    return;

  if (hist->count < MAX_HIST)
    hist->count++;

  if (hist->count)
    for (i = hist->count - 1; i > 0; i--)
      strcpy(hist->lines[i], hist->lines[i - 1]);

  strcpy(hist->lines[0], line);
}
#endif

static void cmdline_init(cmdline_t *cmdline)
{
  cmdline->line[0] = '\0';
  cmdline->pos = 0;
  cmdline->len = 0;
  cmdline->escseq = 0;
  cmdline->escpos = 0;
}

#if CONFIG_SHELL_TAB_COMPLETE || SHELL_CONFIG_HIST
static void cmdline_set(cmdline_t *cmdline, const char *str)
{
  int len = strlen(str);

  strcpy(cmdline->line, str);
  cmdline->len = len;
  cmdline->pos = len;
  cmdline->escseq = 0;
  cmdline->escpos = 0;
}
#endif

static void cmdline_ins(cmdline_t *cmdline, int c)
{
  cmdline->len++;
  cmdline->line[cmdline->pos++] = c % 0xff;
  cmdline->line[cmdline->len] = '\0';
}

static void shell_history_edit(shell_t *shell)
{
#if CONFIG_SHELL_HIST
  if (shell->hist_pos >= 0) {
    const char *h = history_get(&shell->hist, shell->hist_pos);
    cmdline_set(&shell->cmdline, h);
    shell->hist_pos = -1;
  }
#endif
}

void shell_init(shell_t *shell, const char *prompt)
{
  cmdline_init(&shell->cmdline);
#if CONFIG_SHELL_HIST
  history_init(&shell->hist);
  shell->hist_pos = -1;
#endif
  shell->prompt = prompt;

  xputs(prompt);
}

int strsplit(char **argv, int max_argc, char *cmdline)
{
  char c;
  char *p = cmdline;
  unsigned argv_cnt = 0;

  while ((c = *cmdline)) {
    if (c == ' ' || c == '\t') {
      *cmdline = '\0';
      if (strlen(p) > 0) {
        argv[argv_cnt++] = p;
        if (argv_cnt == max_argc)
          return argv_cnt;
      }
      c = *++cmdline;
      while (c == ' ' || c == '\t')
        c = *++cmdline;
      p = cmdline;
    } else
      cmdline++;
  }

  if (strlen(p) > 0)
    argv[argv_cnt++] = p;

  return argv_cnt;
}

cmd_ent_t *find_command(const char *name)
{
  cmd_ent_t *ent;

  int i;

  ent = &cmd_list_start;
  for (i = 0; i < CMD_LIST_LEN; i++) {
    if (strcmp(name, ent->name) == 0)
      return ent;
    ent++;
  }

  return 0;
}

void run_command(char *cmdline)
{
  char *argv[MAX_ARGV];
  int argc;
  int rc;
  cmd_ent_t *ent;

  argc = strsplit(argv, MAX_ARGV, cmdline);

  if (argc == 0)
    return;

  ent = find_command(argv[0]);

  if (!ent)
    xprintf("unknown command '%s'\n", argv[0]);
  else {
    rc = ent->cmd(argc, argv);

    if (rc != 0) {
#if CONFIG_SHELL_HELP
      if (ent->help)
        xputs(ent->help);
      xputs("\n\n");
#endif
      xprintf("error %d\n", rc);
    }
  }
}

static void cmdline_del_char(cmdline_t *cmdline)
{
  if (cmdline->pos < cmdline->len) {
    int i;
    xputs(ANSI_SEQ_DEL);
    for (i = cmdline->pos; i < cmdline->len; i++)
      cmdline->line[i] = cmdline->line[i + 1];
    cmdline->len--;
  }
}

int matchlen(const char *a, const char *b)
{
  int count = 0;
  char ac, bc;

  for (;;) {
    ac = *a++;
    bc = *b++;
    if (ac == 0 || bc == 0 || ac != bc)
      return count;
    count++;
  }
}

#if CONFIG_SHELL_TAB_COMPLETE
typedef struct {
  const char *first;
  unsigned int max_len;
} cmd_match_t;


static int count_matching(const char *line, unsigned int len)
{
  cmd_ent_t *ent;
  unsigned int i;
  unsigned int count = 0;

  ent = &cmd_list_start;
  for (i = 0; i < CMD_LIST_LEN; i++) {
    int cmp = strncmp(line, ent->name, len);
    if (cmp < 0)
      break;
    if (cmp == 0)
      count++;
    ent++;
  }

  return count;
}

static int tab_complete(const char *line, unsigned int len, cmd_match_t *match)
{
  cmd_ent_t *ent;
  unsigned int i, l, count, oc = 0;

  count = count_matching(line, len);

  match->first = 0;
  match->max_len = 0;

  ent = &cmd_list_start;
  for (i = 0; i < CMD_LIST_LEN; i++) {
    if (!strncmp(line, ent->name, len)) {
      if (!match->first) {
        match->first = ent->name;
        if (count > 1)
          xprintf("\n");
      } else {
        l = matchlen(match->first, ent->name);
        if (match->max_len == 0)
          match->max_len = l;
        else
          match->max_len = MIN(l, match->max_len);
      }
      if (count > 1) {
        if (oc && (oc % CMD_PER_LINE) == 0)
          xprintf("\n");
        xprintf("%12s", ent->name);
        oc++;
      }
    }
    ent++;
  }

  if (count > 1)
    xprintf("\n");

  return count;
}
#endif

void shell_input(shell_t *shell, int c)
{
  cmdline_t *cmdline = &shell->cmdline;
  int i;
#if CONFIG_SHELL_HIST
  const char *h;
#endif

  if (cmdline->escseq == 1) {
    if (c == '[')
      cmdline->escseq = 2;
    else
      cmdline->escseq = 0;
  } else if (cmdline->escseq == 2) {
    if (c >= 0x40 && c <= 0x7e) {
      if (c == '~' && cmdline->escpos == 1) {
        if (cmdline->esc[0] == '3')
          cmdline_del_char(cmdline);
      } else if (cmdline->escpos == 0) {
        switch (c) {
        case 'A':
#if CONFIG_SHELL_HIST
          if (shell->hist.count > 0) {
            if (shell->hist_pos < shell->hist.count - 1)
              shell->hist_pos++;
            h = history_get(&shell->hist, shell->hist_pos);
            if (h)
              xprintf(ANSI_SEQ_ERASE "\r%s%s", shell->prompt, h);
          }
#endif
          break;
        case 'B':
#if CONFIG_SHELL_HIST
          if (shell->hist_pos > 0) {
            shell->hist_pos--;
            h = history_get(&shell->hist, shell->hist_pos);
            if (h)
              xprintf(ANSI_SEQ_ERASE "\r%s%s", shell->prompt, h);
          } else {
            shell->hist_pos = -1;
            xprintf(ANSI_SEQ_ERASE "\r%s%s", shell->prompt, cmdline->line);
          }
#endif
          break;
        case 'C':
          if (cmdline->pos < cmdline->len) {
            cmdline->pos++;
            xputs(ANSI_SEQ_RIGHT);
          }
          break;
        case 'D':
          shell_history_edit(shell);
          if (cmdline->pos > 0) {
            xputs(ANSI_SEQ_LEFT);
            cmdline->pos--;
          }
          break;
        }
      }
      cmdline->escseq = 0;
      cmdline->escpos = 0;
    } else {
      if (cmdline->escpos < MAX_ESC)
        cmdline->esc[cmdline->escpos++] = c;
      else
        cmdline->escseq = 0;
    }
  } else if (c == '\033')
    cmdline->escseq = 1;
  else if (c == '\b' || c == 0x7f) {
    shell_history_edit(shell);
    if (cmdline->pos > 0) {
      cmdline->pos--;
      xputs(ANSI_SEQ_LEFT);
      cmdline_del_char(cmdline);
    }
  } else if (c == '\t') {
#if CONFIG_SHELL_TAB_COMPLETE
    {
      int count;
      cmd_match_t match;

      count = tab_complete(cmdline->line, cmdline->len, &match);
      if (count > 0) {
        if (count == 1) {
          cmdline_set(cmdline, match.first);
          cmdline_ins(cmdline, ' ');
        } else {
          cmdline_init(cmdline);
          for (i = 0; i < match.max_len; i++)
            cmdline_ins(cmdline, match.first[i]);
        }
        xprintf(ANSI_SEQ_ERASE "\r%s%s", shell->prompt, cmdline->line);
      }
    }
#endif
  } else if (c == '\r') {
    shell_history_edit(shell);
    xputs("\r\n");
#if CONFIG_SHELL_HIST
    history_insert(&shell->hist, cmdline->line);
    shell->hist_pos = -1;
#endif
    run_command(cmdline->line);
    cmdline_init(cmdline);
    xputs(shell->prompt);
  } else if (_isprint(c)) {
    shell_history_edit(shell);
    if (cmdline->len < MAX_LINE) {
      if (cmdline->len > cmdline->pos) {
        for (i = cmdline->len + 1; i > cmdline->pos; i--)
          cmdline->line[i] = cmdline->line[i - 1];
        xputs(ANSI_SEQ_INS);
      }
      cmdline_ins(cmdline, c);
      xprintf("%c", c);
    }
  }
}

#if CONFIG_SHELL_HELP
int cmd_help(int argc, char *argv[])
{
  cmd_ent_t *ent;
  unsigned int i;

  if (argc < 2) {
    ent = &cmd_list_start;
    for (i = 0; i < CMD_LIST_LEN; i++) {
      if (i && (i % CMD_PER_LINE) == 0)
        xprintf("\n");
      xprintf("%12s", ent->name);
      ent++;
    }
  } else {
    ent = find_command(argv[1]);
    if (!ent)
      xprintf("no such command");
    else {
      const char *help = "help missing";

      if (ent->help)
        help = ent->help;

      xputs(help);
    }
  }

  xputs("\n");

  return 0;
}

SHELL_CMD_H(help, cmd_help,
            "get help for command\n\n"
            "help [command]");
#endif
