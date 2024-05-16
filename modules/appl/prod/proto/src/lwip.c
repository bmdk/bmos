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
      xslog(LOG_INFO, "ip:%d.%d.%d.%d"
            "/%d.%d.%d.%d"
            " gw:%d.%d.%d.%d\n",
            BYTE(ethif.ip_addr.addr, 0),
            BYTE(ethif.ip_addr.addr, 1),
            BYTE(ethif.ip_addr.addr, 2),
            BYTE(ethif.ip_addr.addr, 3),

            BYTE(ethif.netmask.addr, 0),
            BYTE(ethif.netmask.addr, 1),
            BYTE(ethif.netmask.addr, 2),
            BYTE(ethif.netmask.addr, 3),

            BYTE(ethif.gw.addr, 0),
            BYTE(ethif.gw.addr, 1),
            BYTE(ethif.gw.addr, 2),
            BYTE(ethif.gw.addr, 3)
            );
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
    xprintf("addr:    %d.%d.%d.%d\n",
            BYTE(ethif.ip_addr.addr, 0),
            BYTE(ethif.ip_addr.addr, 1),
            BYTE(ethif.ip_addr.addr, 2),
            BYTE(ethif.ip_addr.addr, 3));
    xprintf("netmask: %d.%d.%d.%d\n",
            BYTE(ethif.netmask.addr, 0),
            BYTE(ethif.netmask.addr, 1),
            BYTE(ethif.netmask.addr, 2),
            BYTE(ethif.netmask.addr, 3));
    xprintf("gateway: %d.%d.%d.%d\n",
            BYTE(ethif.gw.addr, 0),
            BYTE(ethif.gw.addr, 1),
            BYTE(ethif.gw.addr, 2),
            BYTE(ethif.gw.addr, 3));
    break;
  }

  return 0;
}

SHELL_CMD(ip, cmd_ip);
