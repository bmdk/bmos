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
#include "hal_int.h"
#include "hal_time.h"
#include "io.h"
#include "shell.h"
#include "xslog.h"
#include "fast_log.h"

#if CONFIG_LWIP
#include "lwip/netif.h"
#include "lwip/etharp.h"
#include "stm32_eth.h"
#endif

#define ETH_BASE 0x40028000

typedef struct {
  unsigned int cr;
  unsigned int ffr;
  unsigned int hthr;
  unsigned int htlr;
  unsigned int miiar;
  unsigned int miidr;
  unsigned int fcr;
  unsigned int vlantr;
  unsigned int rsvd0[2];
  unsigned int rwuffr;
  unsigned int pmtcsr;
  unsigned int rsvd1;
  unsigned int dbgr;
  unsigned int sr;
  unsigned int imr;
  struct {
    unsigned int hr;
    unsigned int lr;
  } a[4];
} stm32_eth_mac_t;

typedef struct {
  unsigned int ccr;
  unsigned int rir;
  unsigned int tir;
  unsigned int rimr;
  unsigned int timr;
  unsigned int rsvd0[14];
  unsigned int tgfsccr;
  unsigned int tgfmsccr;
  unsigned int rsvd1[5];
  unsigned int tgfcr;
  unsigned int rsvd2[10];
  unsigned int rfcecr;
  unsigned int rfaecr;
  unsigned int rsvd3[10];
  unsigned int rgufcr;
} stm32_eth_mmc_t;

typedef struct {
  unsigned int tscr;
  unsigned int ssir;
  unsigned int tshr;
  unsigned int tslr;
  unsigned int tshur;
  unsigned int tslur;
  unsigned int tsar;
  unsigned int tthr;
  unsigned int ttlr;
  unsigned int rsvd0;
  unsigned int tssr;
} stm32_eth_ptp_t;

typedef struct {
  unsigned int bmr;
  unsigned int tpdr;
  unsigned int rpdr;
  unsigned int rdlar;
  unsigned int tdlar;
  unsigned int sr;
  unsigned int omr;
  unsigned int ier;
  unsigned int mfbocr;
  unsigned int rswtr;
  unsigned int rsvd0[8];
  unsigned int chtdr;
  unsigned int chrdr;
  unsigned int chtbar;
  unsigned int chrbar;
} stm32_eth_dma_t;

typedef struct {
  stm32_eth_mac_t mac;
  unsigned int rsvd0[40];
  stm32_eth_mmc_t mmc;
  unsigned int rsvd1[334];
  stm32_eth_ptp_t ptp;
  unsigned int rsvd2[565];
  stm32_eth_dma_t dma;
} stm32_eth_t;

#define ETH ((volatile stm32_eth_t *)ETH_BASE)

#define ETH_MACMIIAR_PA(v) (((v) & 0x1f) << 11)
#define ETH_MACMIIAR_MR(v) (((v) & 0x1f) << 6)
#define ETH_MACMIIAR_CR(v) (((v) & 0x7) << 2)
#define ETH_MACMIIAR_MW BIT(1)
#define ETH_MACMIIAR_MB BIT(0)

#define ETH_DMABMR_SR BIT(0)

#define ETH_DMAIER_NISE BIT(16)
#define ETH_DMAIER_AISE BIT(15)
#define ETH_DMAIER_ERIE BIT(14)
#define ETH_DMAIER_FBEIE BIT(13)
#define ETH_DMAIER_ETIE BIT(10)
#define ETH_DMAIER_RWTIE BIT(9)
#define ETH_DMAIER_RPSIE BIT(8)
#define ETH_DMAIER_RBUIE BIT(7)
#define ETH_DMAIER_RIE BIT(6)
#define ETH_DMAIER_TUIE BIT(5)
#define ETH_DMAIER_ROIE BIT(4)
#define ETH_DMAIER_TJTIE BIT(3)
#define ETH_DMAIER_TBUIE BIT(2)
#define ETH_DMAIER_TPSIE BIT(1)
#define ETH_DMAIER_TIE BIT(0)

#define ETH_DMAIER_ALL ( \
    ETH_DMAIER_NISE | ETH_DMAIER_AISE | ETH_DMAIER_ERIE | \
    ETH_DMAIER_FBEIE | ETH_DMAIER_ETIE | ETH_DMAIER_RWTIE | \
    ETH_DMAIER_RPSIE | ETH_DMAIER_RBUIE | ETH_DMAIER_RIE | \
    ETH_DMAIER_TUIE | ETH_DMAIER_ROIE | ETH_DMAIER_TJTIE | \
    ETH_DMAIER_TBUIE | ETH_DMAIER_TPSIE | ETH_DMAIER_TIE)

#define ETH_DMA_OMR_ST BIT(13)
#define ETH_DMA_OMR_SR BIT(1)

#define FIELD(w, o, v) (((v) & ((1 << (w)) - 1)) << (o))

#define ETH_TDES0_OWN BIT(31)
#define ETH_TDES0_IC BIT(30)
#define ETH_TDES0_LS BIT(29)
#define ETH_TDES0_FS BIT(28)
#define ETH_TDES0_DC BIT(27)
#define ETH_TDES0_DP BIT(26)
#define ETH_TDES0_TTSE BIT(25)
#define ETH_TDES0_CIC(v) FIELD(2, 22, v)
#define ETH_TDES0_TER BIT(21)
#define ETH_TDES0_TCH BIT(20)

#define ETH_TDES1_TBS2(l) (((l) & 0x1fff) << 16)
#define ETH_TDES1_TBS1(l) (((l) & 0x1fff) << 0)

#define ETH_RDES0_OWN BIT(31)
#define ETH_RDES0_AFM BIT(30)
#define ETH_RDES0_FL(r) (((r) >> 16) & 0x3fff)
#define ETH_RDES0_ES BIT(15)
#define ETH_RDES0_DE BIT(14)
#define ETH_RDES0_SAF BIT(13)
#define ETH_RDES0_LE BIT(12)
#define ETH_RDES0_OE BIT(11)
#define ETH_RDES0_VLAN BIT(10)
#define ETH_RDES0_FS BIT(9)
#define ETH_RDES0_LS BIT(8)
#define ETH_RDES0_IPHCE BIT(7)
#define ETH_RDES0_FT BIT(5)
#define ETH_RDES0_RWT BIT(4)
#define ETH_RDES0_RE BIT(3)
#define ETH_RDES0_DRE BIT(2)
#define ETH_RDES0_CE BIT(1)
#define ETH_RDES0_PCE BIT(0)
#define ETH_RDES0_ESA BIT(0)

#define ETH_RDES1_DIC BIT(31)
#define ETH_RDES1_RBS2(l) (((l) & 0x1fff) << 16)
#define ETH_RDES1_RER BIT(15)
#define ETH_RDES1_RCH BIT(14)
#define ETH_RDES1_RBS1(l) (((l) & 0x1fff) << 0)

#define ETH_DMASR_TS BIT(0)
#define ETH_DMASR_TPSS BIT(1)
#define ETH_DMASR_TBUS BIT(2)
#define ETH_DMASR_TJTS BIT(3)
#define ETH_DMASR_ROS BIT(4)
#define ETH_DMASR_TUS BIT(5)
#define ETH_DMASR_RS BIT(6)
#define ETH_DMASR_RBUS BIT(7)
#define ETH_DMASR_RPSS BIT(8)
#define ETH_DMASR_FBES BIT(13)
#define ETH_DMASR_AIS BIT(15)
#define ETH_DMASR_NIS BIT(16)

#define N_RX_DES 4
#define N_TX_DES 4
#define ETH_PKT_LEN 1524

typedef struct {
  unsigned int des[4];
} eth_des_t;

static unsigned char rx_data[N_RX_DES][ETH_PKT_LEN];
static eth_des_t rx_des[N_RX_DES];

static unsigned char tx_data[N_TX_DES][ETH_PKT_LEN];
static eth_des_t tx_des[N_TX_DES];

typedef struct {
  unsigned int currx;
  unsigned int curtx;
} eth_ctx_t;

void poll_rx_desc(eth_ctx_t *eth_ctx, struct netif *nif)
{
  unsigned int idx = eth_ctx->currx;
  eth_des_t *d;
  int count = 0;
  struct pbuf *p, *q;
  unsigned int rem;
  unsigned char *dat;

  d = &rx_des[idx];

  while ((d->des[0] & ETH_RDES0_OWN) == 0) {
    count++;

    rem = ETH_RDES0_FL(d->des[0]);
    dat = rx_data[idx];

#if 0
    xprintf("rx %d len: %d\n", idx, rem);
#endif

    p = pbuf_alloc(PBUF_RAW, rem, PBUF_POOL);
    if (!p) {
      xslog(LOG_ERR, "could not allocate rx buffer %d\n", rem);
    } else {
      err_t err;

      for (q = p; q && (rem > 0); q = q->next) {
        unsigned int cnt = q->len;

        if (rem < cnt)
          cnt = rem;

        memcpy(q->payload, dat, cnt);

        dat += cnt;
        rem -= cnt;
      }

      err = nif->input(p, nif);
      if (err != ERR_OK) {
        xslog(LOG_ERR, "lwip input error\n");
        pbuf_free(p);
      }
    }

    d->des[0] = ETH_RDES0_OWN;
    d->des[1] = ETH_RDES1_RBS2(0) | ETH_RDES1_RBS1(ETH_PKT_LEN);
    d->des[2] = (unsigned int)&rx_data[idx];
    d->des[3] = 0;

    idx++;
    if (idx >= N_RX_DES) {
      d->des[1] |= ETH_RDES1_RER;
      idx = 0;
    }
    d = &rx_des[idx];
  }
#if 0
  if (count > 1)
    xprintf("COUNT %d\n", count);
#endif
  eth_ctx->currx = idx;
}

static void eth_irq(void *data)
{
  unsigned int dmasr;

#if 0
  eth_ctx_t *eth_ctx = data;
#endif

#if 0
  xprintf("irq\n");
#endif

  dmasr = ETH->dma.sr;

  FAST_LOG('n', "eth_irq %08x\n", dmasr, 0);
  ETH->dma.sr = 0xffffffff;

#if 0
  xprintf("dmasr %08x\n", dmasr);
#endif

  if (dmasr & ETH_DMASR_RS) {
#if CONFIG_LWIP
    sem_post(eth_wakeup);
#endif
  }
#if 0
  if (dmasr & ETH_DMASR_TS)
    xprintf("tx done\n");

#endif
  if (dmasr & ETH_DMASR_TPSS)
    xslog(LOG_ERR, "tx process stopped\n");

#if 0
  if (dmasr & ETH_DMASR_TBUS)
    xprintf("no tx buffer\n");

#endif
  if (dmasr & ETH_DMASR_RBUS)
    xslog(LOG_ERR, "no rx buffer\n");
  if (dmasr & ETH_DMASR_RPSS)
    xslog(LOG_ERR, "rx process stopped\n");

#if 0
  if (dmasr & ETH_DMASR_AIS)
    xprintf("AIS\n");
  if (dmasr & ETH_DMASR_NIS)
    xprintf("NIS\n");

#endif
  if (dmasr & ETH_DMASR_FBES) {
    xslog(LOG_ERR, "%s: eth bus error: %s %s\n",
            (dmasr & BIT(23)) ? "tx" : "rx",
            (dmasr & BIT(24)) ? "read" : "write",
            (dmasr & BIT(25)) ? "desc" : "data");
  }
}

static eth_ctx_t eth_ctx;

int phy_reset(void);

int hal_eth_init()
{
  unsigned int i;
  int speed;
  eth_des_t *d;

  ETH->dma.ier = 0;

  /* reset the dma */
  ETH->dma.bmr |= ETH_DMABMR_SR;
  while (ETH->dma.bmr & ETH_DMABMR_SR)
    ;

  ETH->mac.miiar = ETH_MACMIIAR_CR(1);

  /* wait for phy autonegotiate and set negotiated speed */
  speed = phy_reset();
  if (speed < 0)
    return -1;

  if (speed & 1)
    ETH->mac.cr |= BIT(14);
  else
    ETH->mac.cr &= ~BIT(14);

  if (speed & 2)
    ETH->mac.cr |= BIT(11);
  else
    ETH->mac.cr &= ~BIT(11);

  for (i = 0; i < N_RX_DES; i++) {
    d = &rx_des[i];

    d->des[0] = ETH_RDES0_OWN;
    d->des[1] = ETH_RDES1_RBS2(0) | ETH_RDES1_RBS1(ETH_PKT_LEN);
    d->des[2] = (unsigned int)&rx_data[i];
    d->des[3] = 0;
  }

  d->des[1] |= ETH_RDES1_RER;

  eth_ctx.currx = 0;

  memset(tx_des, 0, sizeof(tx_des));

  ETH->dma.rdlar = (unsigned int)&rx_des[0];

  ETH->dma.tdlar = (unsigned int)&tx_des[0];

  ETH->dma.ier = ETH_DMAIER_ALL;

  ETH->dma.omr = ETH_DMA_OMR_SR;   /* start rx */

  ETH->mac.ffr = BIT(31) | BIT(0); /* promisc */

  ETH->mac.cr |= BIT(3) | BIT(2);  /* enable tx,rx */

  irq_register("eth", eth_irq, (void *)&eth_ctx, 61);

  return 0;
}

int phy_read(unsigned int phy, unsigned int reg)
{
  int count = 1000;

  ETH->mac.miiar = ETH_MACMIIAR_PA(phy) | ETH_MACMIIAR_MR(reg) | \
                   ETH_MACMIIAR_CR(1) | ETH_MACMIIAR_MB;

  while ((ETH->mac.miiar & ETH_MACMIIAR_MB) && count) {
    count++;
    hal_delay_us(100);
  }
  if (count == 0)
    xslog(LOG_ERR, "timeout waiting for phy\n");

  return ETH->mac.miidr & 0xffff;
}

void phy_write(unsigned int phy, unsigned int reg, unsigned int val)
{
  int count = 10;

  ETH->mac.miidr = val & 0xffff;

  ETH->mac.miiar = ETH_MACMIIAR_PA(phy) | ETH_MACMIIAR_MR(reg) | \
                   ETH_MACMIIAR_CR(1) | ETH_MACMIIAR_MB | ETH_MACMIIAR_MW;

  while ((ETH->mac.miiar & ETH_MACMIIAR_MB) && count) {
    count++;
    hal_delay_us(100);
  }
  if (count == 0)
    xslog(LOG_ERR, "timeout waiting for phy\n");
}

#if STM32_F4D
#define PHY_ADDR 0
#else
#define PHY_ADDR 1
#endif

int phy_reset(void)
{
  unsigned int count, reg;

  phy_write(PHY_ADDR, 0, BIT(15)); /* reset phy */

  hal_delay_us(100);
  while (phy_read(PHY_ADDR, 0) & BIT(15))
    ;

  hal_delay_us(100);

  // Restart and enable auto-negotiate
  phy_write(PHY_ADDR, 0, BIT(12) | BIT(9));
  count = 50;
  while ((phy_read(PHY_ADDR, 0) & BIT(9)) && count) {
    hal_delay_us(100);
    count--;
  }
  if (count == 0) {
    xslog(LOG_INFO, "phy read timeout\n");
    return -1;
  }
  count = 5000;
  while (count > 0) {
    reg = phy_read(PHY_ADDR, 31);
    if (reg & BIT(12))
      break;
    hal_delay_us(1000);
    count --;
  }
  if (count == 0) {
    xslog(LOG_INFO, "eth autonegotiate fail");
    return -1;
  }

  xslog(LOG_INFO, "eth 10%s:%s-duplex", (reg & BIT(3)) ? "0": "", (reg & BIT(4)) ? "full" : "half");
  return (reg >> 3) & 0x3;
}

int cmd_eth(int argc, char *argv[])
{
  int v, i;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'r':
    for (i = 0; i < 32; i++) {
      v = phy_read(PHY_ADDR, i);
      xprintf("%02x: %04x\n", i, v);
    }
    break;
  }
  return 0;
}

SHELL_CMD(eth, cmd_eth);

#if CONFIG_LWIP
static err_t hal_eth_send(struct netif *netif, struct pbuf *p)
{
  int idx = eth_ctx.curtx;
  eth_des_t *d;
  unsigned char *dat;
  struct pbuf *q;
  unsigned int len = 0;

  dat = tx_data[idx];
  for (q = p; q; q = q->next) {
    unsigned int l = q->len;

    memcpy(dat, q->payload, l);

    dat += l;
    len += l;
  }

  d = &tx_des[idx];

  if ((d->des[0] & ETH_TDES0_OWN) != 0) {
    xslog(LOG_ERR, "No free tx buffer %08x\n", d->des[0]);
    return -1;
  }

  d->des[0] = ETH_TDES0_OWN | ETH_TDES0_IC | ETH_TDES0_LS | \
              ETH_TDES0_FS;
  d->des[1] = ETH_TDES1_TBS2(0) | ETH_TDES1_TBS1(len);
  d->des[2] = (unsigned int)&tx_data[idx];
  d->des[3] = 0;

  idx++;

  if (idx >= N_TX_DES) {
    d->des[0] |= ETH_TDES0_TER;
    idx = 0;
  }
  eth_ctx.curtx = idx;

  ETH->dma.omr |= ETH_DMA_OMR_ST; /* start tx */
  ETH->dma.tpdr = 0;

  return 0;
}

err_t eth_init(struct netif *nif)
{
  nif->output = etharp_output;
  nif->linkoutput = hal_eth_send;

  nif->hwaddr_len = ETH_HWADDR_LEN;
  nif->hwaddr[0] = 0x00;
  nif->hwaddr[1] = 0x02;
  nif->hwaddr[2] = 0x00;
  nif->hwaddr[3] = 0x00;
  nif->hwaddr[4] = 0x00;
  nif->hwaddr[5] = 0x01;

  nif->mtu = 1500;
  nif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  nif->flags |= NETIF_FLAG_LINK_UP;

  hal_eth_init();

  return 0;
}

void eth_input(struct netif *nif)
{
  poll_rx_desc(&eth_ctx, nif);
}
#endif
