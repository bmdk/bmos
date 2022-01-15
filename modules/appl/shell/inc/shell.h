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

#ifndef SHELL_H
#define SHELL_H

#ifndef CONFIG_SHELL_HELP
#define CONFIG_SHELL_HELP 1
#endif

#ifndef CONFIG_SHELL_HIST
#define CONFIG_SHELL_HIST 1
#endif

#ifndef CONFIG_SHELL_TAB_COMPLETE
#define CONFIG_SHELL_TAB_COMPLETE 1
#endif

typedef int shell_cmd_t (int argc, char *argv[]);

#define MAX_LINE 64
#define MAX_ESC 8
#define MAX_HIST 6

#if CONFIG_SHELL_HIST
typedef struct {
  char lines[MAX_HIST][MAX_LINE + 1];
  unsigned char count;
} sh_hist_t;
#endif

typedef struct {
  char line[MAX_LINE + 1];
  int pos;
  int len;
  char escseq;
  int escpos;
  char esc[MAX_ESC];
} cmdline_t;

typedef struct {
  cmdline_t cmdline;
#if CONFIG_SHELL_HIST
  sh_hist_t hist;
  signed char hist_pos;
#endif
  const char *prompt;
} shell_t;

void shell_init(shell_t *shell, const char *prompt);
void shell_input(shell_t *shell, int c);

typedef struct {
  const char *name;
  shell_cmd_t *cmd;
#if CONFIG_SHELL_HELP
  const char *help;
#endif
} cmd_ent_t;

#if CONFIG_SHELL_HELP
#define _SHELL_CMD(_name_, _fun_, _help_) \
  static const cmd_ent_t shell_cmd_ent_ ## _name_ \
  __attribute__((section(".shell_reg." #_name_), used)) = \
  { #_name_, _fun_, _help_ }
#else
#define _SHELL_CMD(_name_, _fun_, _help_) \
  static const cmd_ent_t shell_cmd_ent_ ## _name_ \
  __attribute__((section(".shell_reg." #_name_), used)) = \
  { #_name_, _fun_ }
#endif

#define SHELL_CMD_H(_name_, _fun_, _help_) \
  _SHELL_CMD(_name_, _fun_, _help_)

#define SHELL_CMD(_name_, _fun_) _SHELL_CMD(_name_, _fun_, 0)

#endif
