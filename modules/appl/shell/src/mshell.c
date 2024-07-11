#include <stddef.h>

#include "bmos_msg_queue.h"
#include "bmos_op_msg.h"
#include "bmos_task.h"
#include "debug_ser.h"
#include "io.h"
#include "shell.h"
#include "hal_uart.h"
#include "xassert.h"

#include "mshell.h"

#ifndef CONFIG_SHELL_SRC_COUNT
#define CONFIG_SHELL_SRC_COUNT 3
#endif
#ifndef CONFIG_SHELL_SRC_DEFAULT
#define CONFIG_SHELL_SRC_DEFAULT 0
#endif

typedef struct {
  unsigned short dest;
  bmos_queue_t *rxq;
  bmos_queue_t *txq[CONFIG_SHELL_SRC_COUNT];
  unsigned short txop[CONFIG_SHELL_SRC_COUNT];
} shell_info_t;

static shell_t shell;
static shell_info_t shell_info;

void polled_shell(void)
{
  shell_init(&shell, "> ");

  for (;;) {
    int c = debug_getc();
    if (c >= 0)
      shell_input(&shell, c);
  }
}

static void shell_info_init(shell_info_t *info, const char *name,
                            unsigned int dest)
{
  info->rxq = queue_create(name, QUEUE_TYPE_TASK);
  info->dest = dest;
}

static void set_default_output(unsigned int num)
{
  if (num == CONFIG_SHELL_SRC_DEFAULT ||
      !shell_info.txq[CONFIG_SHELL_SRC_DEFAULT]) {
    io_set_output(shell_info.txq[num], shell_info.txop[num]);
    shell_info.dest = num;
  }
}

static void shell_info_add_uart(shell_info_t *info, uart_t *u,
                                unsigned int baud, unsigned int num,
                                unsigned int txop)
{
  XASSERT(num < CONFIG_SHELL_SRC_COUNT);
  XASSERT(info);
  XASSERT(info->rxq);

  shell_info.txq[num] = uart_open(u, baud, info->rxq, num);
  shell_info.txop[num] = txop;

  set_default_output(num);
}

static void shell_info_add_queue(shell_info_t *info, bmos_queue_t *q,
                                 unsigned int num, unsigned int txop)
{
  XASSERT(num < CONFIG_SHELL_SRC_COUNT);
  XASSERT(info);
  XASSERT(info->rxq);

  shell_info.txq[num] = q;
  shell_info.txop[num] = txop;

  set_default_output(num);
}

void mshell_add_uart(uart_t *u, unsigned int baud,
                     unsigned int num, unsigned int txop)
{
  shell_info_add_uart(&shell_info, u, baud, num, txop);
}

void mshell_add_queue(bmos_queue_t *q,
                      unsigned int num, unsigned int txop)
{
  shell_info_add_queue(&shell_info, q, num, txop);
}

int _xgetc(int timeout)
{
  unsigned char *data;
  static bmos_op_msg_t *m = NULL;
  static unsigned int pos = 0;
  int c;

  if (!m) {
    m = op_msg_wait_ms(shell_info.rxq, timeout);

    if (!m)
      return -1;

    if (m->op >= CONFIG_SHELL_SRC_COUNT || m->len == 0) {
      op_msg_return(m);
      m = NULL;
      return -1;
    }

    if (m->op != shell_info.dest && shell_info.txq[m->op] != 0) {
      io_set_output(shell_info.txq[m->op], shell_info.txop[m->op]);
      shell_info.dest = m->op;
    }

    pos = 0;
  }

  data = BMOS_OP_MSG_GET_DATA(m);

  c = data[pos];

  pos++;
  if (pos >= m->len) {
    op_msg_return(m);
    pos = 0;
    m = NULL;
  }

  return c;
}

static void mshell_task(void *arg)
{
  shell_init(&shell, "> ");

  for (;;) {
    int c = _xgetc(-1);

    if (c >= 0)
      shell_input(&shell, c);
  }
}

int xgetc(void)
{
  return _xgetc(1000);
}

void xputc(int ch)
{
  xprintf("%c", ch);
}

#ifndef CONFIG_MSHELL_TASK_PRI
#define CONFIG_MSHELL_TASK_PRI 1
#endif

void mshell_init(const char *name, unsigned int dest)
{
  shell_info_init(&shell_info, name, dest);
  task_init(mshell_task, &shell_info, "shell",
            CONFIG_MSHELL_TASK_PRI, 0, 768);
}

bmos_queue_t *mshell_queue()
{
  return shell_info.rxq;
}
