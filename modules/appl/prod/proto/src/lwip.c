#include "lwip/init.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/apps/httpd.h"
#include "netif/etharp.h"
#include "lwip/timeouts.h"
#include "lwip/dhcp.h"
#include "stm32_eth.h"
#include "xslog.h"

#include "shell.h"
#include "io.h"

#define BYTE(v, n) (((unsigned int)(v) >> (n << 3)) & 0xff)

struct netif ethif;
bmos_sem_t *eth_wakeup;
static signed char has_addr;

#if 0
unsigned char _ipaddr[] = { 10, 80, 40, 11 };
unsigned char _netmask[] = { 255, 255, 255, 0 };
#else
unsigned char _ipaddr[] = { 0, 0, 0, 0 };
unsigned char _netmask[] = { 0, 0, 0, 0 };
#endif
unsigned char _gateway[] = { 0, 0, 0, 0 };

int lwip_test_init(void);

void task_net()
{
  ip4_addr_t ipaddr, netmask, gateway;

  IP4_ADDR(&ipaddr, _ipaddr[0], _ipaddr[1], _ipaddr[2], _ipaddr[3]);
  IP4_ADDR(&netmask, _netmask[0], _netmask[1], _netmask[2], _netmask[3]);
  IP4_ADDR(&gateway, _gateway[0], _gateway[1], _gateway[2], _gateway[3]);

  lwip_init();
  httpd_init();
  lwip_test_init();

  eth_wakeup = sem_create("eth_wakeup", 0);

  netif_add(&ethif, &ipaddr, &netmask, &gateway, NULL, \
            &eth_init, &ethernet_input);
  netif_set_default(&ethif);
  netif_set_up(&ethif);

  has_addr = 0;
  dhcp_start(&ethif);

  for (;;) {
    int err;
    err = sem_wait_ms(eth_wakeup, 1000);
    eth_input(&ethif);

    if (err < 0)
      sys_check_timeouts();

    if (!has_addr && dhcp_supplied_address(&ethif)) {
      xslog(LOG_INFO, "addr:%d.%d.%d.%d\n",
            BYTE(ethif.ip_addr.addr, 0),
            BYTE(ethif.ip_addr.addr, 1),
            BYTE(ethif.ip_addr.addr, 2),
            BYTE(ethif.ip_addr.addr, 3));
      has_addr = 1;
    }
  }
}

static int cmd_ip(int argc, char *argv[])
{
  int cmd = 'a';

  if (argc > 1)
    cmd = argv[1][0];

  switch (cmd) {
  case 'a':
    xprintf("addr:%d.%d.%d.%d\n",
            BYTE(ethif.ip_addr.addr, 0),
            BYTE(ethif.ip_addr.addr, 1),
            BYTE(ethif.ip_addr.addr, 2),
            BYTE(ethif.ip_addr.addr, 3));
    break;
  }

  return 0;
}

SHELL_CMD(ip, cmd_ip);
