#include "lwip/init.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/apps/httpd.h"
#include "netif/etharp.h"
#include "lwip/timeouts.h"
#include "stm32_eth.h"

struct netif ethif;
bmos_sem_t *eth_wakeup;

unsigned char _ipaddr[] = { 10, 80, 40, 11 };
unsigned char _netmask[] = { 255, 255, 255, 0 };
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

  for (;;) {
    int err;
    err = sem_wait_ms(eth_wakeup, 1000);
    eth_input(&ethif);

    if (err < 0)
      sys_check_timeouts();
  }
}
