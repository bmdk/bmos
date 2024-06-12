#include <lwip/err.h>
#include <lwip/raw.h>
#include <lwip/tcp.h>

#include "bmos_op_msg.h"
#include "bmos_queue.h"
#include "bmos_syspool.h"
#include "bmos_task.h"
#include "fast_log.h"
#include "io.h"
#include "mshell.h"
#include "shell.h"

static struct tcp_pcb *telnet_listening_pcb = 0;
static struct tcp_pcb *telnet_pcb = 0;
static bmos_queue_t *shell_tx;

typedef struct {
  char active;
  char data[2];
  char data_len;
} telnet_esc_t;

static telnet_esc_t telnet_esc;

static void telnet_shell_put(void *arg)
{
  bmos_op_msg_t *m;
  unsigned int len;
  unsigned char *data;
  err_t rerr;

  for (;;) {
    m = op_msg_get(shell_tx);
    if (!m)
      break;

    len = m->len;

    data = BMOS_OP_MSG_GET_DATA(m);

    if (telnet_pcb) {
      rerr = tcp_write(telnet_pcb, data, len, TCP_WRITE_FLAG_COPY);
      if (rerr != ERR_OK)
        FAST_LOG('t', "tcp write error %d, len %d\n", rerr, len);
    }

    op_msg_return(m);
  }
}

#define TELNET_WILL 251
#define TELNET_WONT 252
#define TELNET_DO   253
#define TELNET_DONT 254
#define TELNET_IAC  255

#define TELNET_OPT_ECHO 1
#define TELNET_OPT_LINEMODE 34

static err_t telnet_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  struct pbuf *q;
  bmos_op_msg_t *m = 0;
  char *d;
  unsigned int count;

  if (!p) {
    telnet_pcb = 0;
    return tcp_close(pcb);
  }

  for (q = p; q; q = q->next) {
    unsigned char *c = q->payload;
    unsigned int i;

    for (i = 0; i < q->len; i++)
    {
      unsigned char ch = *c++;

      if (telnet_esc.active) {
        telnet_esc.data[(int)telnet_esc.data_len++] = ch;
        if (telnet_esc.data_len == 1 && ch != TELNET_WILL &&
            ch != TELNET_WONT && ch != TELNET_DO && ch != TELNET_DONT) {
          telnet_esc.active = 0;
        } else if (telnet_esc.data_len == 2) {
          telnet_esc.active = 0;
        }
      } else if (!telnet_esc.active && ch == TELNET_IAC) {
        telnet_esc.active = 1;
        telnet_esc.data_len = 0;
      } else {
        if (!m) {
          m = op_msg_wait(syspool);
          d = BMOS_OP_MSG_GET_DATA(m);
          count = 0;
        }

        d[count++] = (char)ch;

        if (count >= SYSPOOL_SIZE) {
          op_msg_put(mshell_queue(), m, 2, count);
          m = 0;
        }
      }
    }
  }

  if (m) {
    op_msg_put(mshell_queue(), m, 2, count);
    m = 0;
  }

  tcp_recved(pcb, p->tot_len);

  pbuf_free(p);

  return ERR_OK;
}

static void send_telnet_opt(unsigned int cmd, unsigned int val)
{
  unsigned char data[3];
  int rerr;

  data[0] = TELNET_IAC;
  data[1] = (unsigned int)cmd;
  data[2] = (unsigned int)val;

  rerr = tcp_write(telnet_pcb, data, 3, TCP_WRITE_FLAG_COPY);
  if (rerr != ERR_OK)
    debug_printf("tcp write error %d\n", rerr);
}

static err_t telnet_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
  if (!pcb) {
    debug_printf("accept got zero pcb\n");
    return ERR_MEM;
  }

  if (telnet_pcb) {
    tcp_close(pcb);
    return ERR_ABRT;
  }

  telnet_pcb = pcb;

  telnet_esc.active = 0;
  send_telnet_opt(TELNET_WONT, TELNET_OPT_LINEMODE);
  send_telnet_opt(TELNET_WILL, TELNET_OPT_ECHO);

  (void)queue_set_put_f(shell_tx, telnet_shell_put, 0, pcb);

  pcb->keep_intvl = 1000;

  tcp_recv(pcb, telnet_recv);

  return ERR_OK;
}

int lwip_test_init()
{
  struct tcp_pcb *pcb = 0;
  err_t err = -1;

  pcb = tcp_new();
  if (!pcb)
    goto err_exit;

  err = tcp_bind(pcb, IP_ADDR_ANY, 23);
  if (err != ERR_OK)
    goto err_exit;

  pcb = tcp_listen(pcb);
  if (!pcb)
    goto err_exit;

  tcp_accept(pcb, telnet_accept);

  telnet_listening_pcb = pcb;

  shell_tx = queue_create("ethshtx", QUEUE_TYPE_DRIVER);

  mshell_add_queue(shell_tx, 2, 0);

  return 0;

err_exit:
  if (pcb)
    tcp_close(pcb);
  return -1;
}
