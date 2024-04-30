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

#ifndef STM32_ETH_H
#define STM32_ETH_H

#define ETH_FLAGS_PHY_FIXED BIT(0)
#define ETH_FLAGS_PHY_SPEED_100 BIT(1)
#define ETH_FLAGS_PHY_FULL_DUPLEX BIT(2)

typedef struct {
  unsigned char hwaddr[6];
  unsigned char flags;
} hal_eth_config_t;

void hal_eth_config_get(hal_eth_config_t *config);

#if CONFIG_LWIP
#include "lwip/netif.h"
#include "bmos_sem.h"

extern void eth_input(struct netif *nif);
extern err_t eth_init(struct netif *nif);

extern bmos_sem_t *eth_wakeup;
#endif

#endif
