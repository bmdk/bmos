#include <stdlib.h>

#include <lwip/err.h>
#include <lwip/raw.h>
#include <lwip/tcp.h>
#include <lwip/ip_addr.h>
#include <lwip/inet_chksum.h>

#include "bmos_op_msg.h"
#include "bmos_queue.h"
#include "bmos_syspool.h"
#include "bmos_task.h"
#include "fast_log.h"
#include "io.h"
#include "mshell.h"
#include "shell.h"
#include "xtime.h"

#define BYTE(v, n) (((unsigned int)(v) >> (n << 3)) & 0xff)

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

  rerr = tcp_output(telnet_pcb);
  if (rerr != ERR_OK)
    FAST_LOG('t', "tcp output error %d\n", rerr, 0);
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

#define PING_DELAY     1000
#define PING_DATA_SIZE 56
#define PING_ID        0xAF5A

static struct raw_pcb *ping_pcb;
static u16_t ping_seq_num;

static void
ping_prepare_echo(struct icmp_echo_hdr *iecho, u16_t len, unsigned int ping_id)
{
  size_t i;
  size_t data_len = len - sizeof(struct icmp_echo_hdr);

  ICMPH_TYPE_SET(iecho, ICMP_ECHO);
  ICMPH_CODE_SET(iecho, 0);
  iecho->chksum = 0;
  iecho->id     = lwip_htons(ping_id);
  iecho->seqno  = lwip_htons(++ping_seq_num);

  /* fill the additional data buffer with some data */
  for(i = 0; i < data_len; i++) {
    ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
  }

  iecho->chksum = inet_chksum(iecho, len);
}

static void
ping_send(struct raw_pcb *raw, const ip_addr_t *addr, unsigned int ping_id)
{
  struct pbuf *p;
  struct icmp_echo_hdr *iecho;
  size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

  p = pbuf_alloc(PBUF_IP, (u16_t)ping_size, PBUF_RAM);
  if (!p) {
    return;
  }
  if ((p->len == p->tot_len) && (p->next == NULL)) {
    iecho = (struct icmp_echo_hdr *)p->payload;

    ping_prepare_echo(iecho, (u16_t)ping_size, ping_id);

    raw_sendto(raw, p, addr);
  }
  pbuf_free(p);
}

typedef struct {
  ip_addr_t addr;
  unsigned short ping_id;
  char print;
  unsigned int count;
} ping_recv_data_t;

static ping_recv_data_t ping_recv_data;

static u8_t
ping_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *addr)
{
  ping_recv_data_t *prd = (ping_recv_data_t *)arg;

  if ((p->len == p->tot_len) && (p->next == NULL)) {
    struct ip_hdr *iphdr = (struct ip_hdr *)p->payload;
    struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr *)(iphdr + 1);

    /* only looking for echo replies */
    if (iecho->type != ICMP_ER || lwip_ntohs(iecho->id) != prd->ping_id)
      return 0;

    if (prd->print) {
      xprintf("%d bytes from %d.%d.%d.%d icmp_seq=%d\n",
          p->len - sizeof(struct ip_hdr),
          BYTE(addr->addr, 0), BYTE(addr->addr, 1),
          BYTE(addr->addr, 2), BYTE(addr->addr, 3),
          lwip_ntohs(iecho->seqno)
          );
    }

    if (addr->addr == prd->addr.addr)
        prd->count ++;
  }
  return 0;
}

#define PING_INTERVAL 1000
#define PING_REPLY_WAIT 100

static int cmd_ping(int argc, char *argv[])
{
  ip_addr_t ping_addr;
  unsigned int start_count, count = 0, print = 1;
  xtime_ms_t start = xtime_ms();

  if (argc < 2)
    return -1;

  if (!ipaddr_aton(argv[1], &ping_addr)) {
    xprintf("invalid ip address\n");
    return -1;
  }

  if (argc > 2)
    count = atoi(argv[2]);

  if (argc > 3)
    print = atoi(argv[3]);

  if (!ping_pcb) {
    ping_pcb = raw_new(IP_PROTO_ICMP);

    raw_recv(ping_pcb, ping_recv, &ping_recv_data);
    raw_bind(ping_pcb, IP_ADDR_ANY);
  }

  ping_seq_num = 0;

  ping_recv_data.ping_id = PING_ID;
  ping_recv_data.print = print;
  ping_recv_data.addr = ping_addr;

  start_count = ping_recv_data.count;

  for (;;) {
    char c;
    xtime_ms_t now;

    now = xtime_ms();

    if (count && (now > start + count * PING_INTERVAL + PING_REPLY_WAIT)) {
      xprintf("FAIL\n");
      break;
    }

    ping_send(ping_pcb, &ping_addr, PING_ID);

    c = _xgetc(PING_INTERVAL);
    if (c == '\03')
      break;

    if (count && (ping_recv_data.count - start_count >= count)) {
      xprintf("OK\n");
      break;
    }

  }

  return 0;
}

SHELL_CMD(ping, cmd_ping);
