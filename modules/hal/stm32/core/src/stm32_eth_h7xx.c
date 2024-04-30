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

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "hal_int.h"
#include "hal_time.h"
#include "io.h"
#include "shell.h"
#include "fast_log.h"
#include "xslog.h"
#include "stm32_eth_phy.h"
#include "stm32_eth.h"

#if CONFIG_LWIP
#include "lwip/netif.h"
#include "lwip/etharp.h"
#endif

#if STM32_H5XX
#define ETH_IRQ 106
#else
#define ETH_IRQ 61
#endif

#define ETH_BASE 0x40028000

typedef struct {
  reg32_t maccr;
  reg32_t macecr;
  reg32_t macpfr;
  reg32_t macwtr;
  reg32_t mact0r;
  reg32_t mact1r;
  reg32_t rsvd0[14];
  reg32_t macvtr;
  reg32_t macvhtr;
  reg32_t rsvd1;
  reg32_t macvir;
  reg32_t macivir;
  reg32_t rsvd2[3];
  reg32_t macqtxfcr;
  reg32_t rsvd3[7];
  reg32_t macrxfcr;
  reg32_t rsvd4[7];
  reg32_t macisr;
  reg32_t macier;
  reg32_t macrxtxsr;
  reg32_t rsvd5;
  reg32_t macpcsr;
  reg32_t macrwkpfr;
  reg32_t rsvd6[2];
  reg32_t maclcsr;
  reg32_t macltcr;
  reg32_t macletr;
  reg32_t mac1ustcr;
  reg32_t rsvd7[12];
  reg32_t macvr;
  reg32_t macdr;
  reg32_t rsvd8[2];
  reg32_t machwf1r;
  reg32_t machwf2r;
  reg32_t rsvd9[54];
  reg32_t macmdioar;
  reg32_t macmdiodr;
  reg32_t rsvd10[2];
  reg32_t macarpar;
  reg32_t rsvd11[59];
  struct {
    reg32_t hr;
    reg32_t lr;
  } maca[4];
  reg32_t rsvd12[248];
  reg32_t mmc_control;
  reg32_t mmc_rx_interrupt;
  reg32_t mmc_tx_interrupt;
  reg32_t mmc_rx_interrupt_mask;
  reg32_t mmc_tx_interrupt_mask;
  reg32_t rsvd13[14];
  reg32_t txsnglcolg;
  reg32_t txmultcolg;
  reg32_t rsvd14[5];
  reg32_t txpktg;
  reg32_t rsvd15[10];
  reg32_t rxcrcerr;
  reg32_t rxalgnerr;
  reg32_t rsvd16[10];
  reg32_t rxucastg;
  reg32_t rsvd17[9];
  reg32_t txlpiusc;
  reg32_t txlpitrc;
  reg32_t rxlpiusc;
  reg32_t rxlpitrc;
  reg32_t rsvd18[65];
  reg32_t macl3l4c0r;
  reg32_t macl4a0r;
  reg32_t rsvd19[2];
  reg32_t macl3a00r;
  reg32_t macl3a10r;
  reg32_t macl3a20r;
  reg32_t macl3a30r;
  reg32_t rsvd20[4];
  reg32_t macl3l4c1r;
  reg32_t macl4a1r;
  reg32_t rsvd21[2];
  reg32_t macl3a01r;
  reg32_t macl3a11r;
  reg32_t macl3a21r;
  reg32_t macl3a31r;
  reg32_t rsvd22[108];
  reg32_t mactscr;
  reg32_t macssir;
  reg32_t macstsr;
  reg32_t macstnr;
  reg32_t macstsur;
  reg32_t macstnur;
  reg32_t mactsar;
  reg32_t rsvd23;
  reg32_t mactssr;
  reg32_t rsvd24[3];
  reg32_t mactxtssnr;
  reg32_t mactxtsssr;
  reg32_t rsvd25[2];
  reg32_t macacr;
  reg32_t rsvd26;
  reg32_t macatsnr;
  reg32_t macatssr;
  reg32_t mactsiacr;
  reg32_t mactseacr;
  reg32_t mactsicnr;
  reg32_t mactsecnr;
  reg32_t rsvd27[4];
  reg32_t macppscr;
  reg32_t rsvd28[3];
  reg32_t macppsttsr;
  reg32_t macppsttnr;
  reg32_t macppsir;
  reg32_t macppswr;
  reg32_t rsvd29[12];
  reg32_t macpocr;
  reg32_t macspi0r;
  reg32_t macspi1r;
  reg32_t macspi2r;
  reg32_t maclmir;
  reg32_t rsvd30[11];
  reg32_t mtlomr;
  reg32_t rsvd31[7];
  reg32_t mtlisr;
  reg32_t rsvd32[55];
  reg32_t mtltxqomr;
  reg32_t mtltxqur;
  reg32_t mtltxqdr;
  reg32_t rsvd33[8];
  reg32_t mtlqicsr;
  reg32_t mtlrxqomr;
  reg32_t mtlrxqmpocr;
  reg32_t mtlrxqdr;
  reg32_t rsvd34[177];
  reg32_t dmamr;
  reg32_t dmasbmr;
  reg32_t dmaisr;
  reg32_t dmadsr;
  reg32_t rsvd35[60];
  reg32_t dmaccr;
  reg32_t dmactxcr;
  reg32_t dmacrxcr;
  reg32_t rsvd36[2];
  reg32_t dmactxdlar;
  reg32_t rsvd37;
  reg32_t dmacrxdlar;
  reg32_t dmactxdtpr;
  reg32_t rsvd38;
  reg32_t dmacrxdtpr;
  reg32_t dmactxdrlr;
  reg32_t dmacrxdrlr;
  reg32_t dmacier;
  reg32_t dmacrxiwtr;
  reg32_t rsvd39[2];
  reg32_t dmaccatxdr;
  reg32_t rsvd40;
  reg32_t dmaccarxdr;
  reg32_t rsvd41;
  reg32_t dmaccatxbr;
  reg32_t rsvd42;
  reg32_t dmaccarxbr;
  reg32_t dmacsr;
  reg32_t rsvd43[2];
  reg32_t dmacmfcr;
} stm32_eth_h7xx_t;

#define ETH ((stm32_eth_h7xx_t *)ETH_BASE)

#define ETH_DMAMR_SWR BIT(0)

#define ETH_DMSABMR_FB BIT(0)
#define ETH_DMSABMR_AAL BIT(12)
#define ETH_DMSABMR_MB BIT(14)
#define ETH_DMSABMR_RB BIT(15)

#define FIELD(w, o, v) (((v) & ((1 << (w)) - 1)) << (o))

#define ETH_TDES2_IOC BIT(31)
#define ETH_TDES2_TTSE BIT(30)
#define ETH_TDES2_B2L(v) FIELD(14, 16, v)
#define ETH_TDES2_VITR(v) (((v) & 0x3) << 14)
#define ETH_TDES2_VITR_NONE ETH_TDES2_VITR(0)
#define ETH_TDES2_VITR_REM ETH_TDES2_VITR(1)
#define ETH_TDES2_VITR_INS ETH_TDES2_VITR(2)
#define ETH_TDES2_VITR_REPL ETH_TDES2_VITR(3)
#define ETH_TDES2_B1L(v) FIELD(14, 0, v)

#define ETH_TDES3_OWN BIT(31)
#define ETH_TDES3_CTXT BIT(30)
#define ETH_TDES3_FD BIT(29)
#define ETH_TDES3_LD BIT(28)
#define ETH_TDES3_CPC(v) FIELD(2, 26, v)
#define ETH_TDES3_SAIC(v) FIELD(3, 23, v)
#define ETH_TDES3_THL(v) FIELD(4, 19, v)
#define ETH_TDES3_TSE BIT(18)
#define ETH_TDES3_CICTPL FIELD(2, 16, v) /* cksum/tcp_payload_len */
#define ETH_TDES3_TPL BIT(15)
#define ETH_TDES3_FL(v) FIELD(15, 0, v)

typedef struct {
  unsigned int des[4];
} eth_des_t;

#define ETH_RDES3_OWN BIT(31)
#define ETH_RDES3_IOC BIT(30)
#define ETH_RDES3_BUF2V BIT(25)
#define ETH_RDES3_BUF1V BIT(24)

#define ETH_DMARXCR_ST BIT(0)
#define ETH_DMATXCR_ST BIT(0)

#define ETH_DMACIER_NIE BIT(15)
#define ETH_DMACIER_AIE BIT(14)
#define ETH_DMACIER_CDEE BIT(13)
#define ETH_DMACIER_FBEE BIT(12)
#define ETH_DMACIER_ERIE BIT(11)
#define ETH_DMACIER_ETIE BIT(10)
#define ETH_DMACIER_RWTE BIT(9)
#define ETH_DMACIER_RSE BIT(8)
#define ETH_DMACIER_RBUE BIT(7)
#define ETH_DMACIER_RIE BIT(6)
#define ETH_DMACIER_TBUE BIT(2)
#define ETH_DMACIER_TXSE BIT(1)
#define ETH_DMACIER_TIE BIT(0)

#define N_RX_DES 4
#define N_TX_DES 4
#define ETH_PKT_LEN 1524
#define AETH_PKT_LEN ALIGN(ETH_PKT_LEN, 3)

#if STM32_H7XX
#define ETHSECT __attribute__((section(".eth"), aligned(8)))
#else
#define ETHSECT
#endif

static unsigned char rx_data[N_RX_DES][AETH_PKT_LEN] ETHSECT;
static eth_des_t rx_des[N_RX_DES] ETHSECT;

#if CONFIG_LWIP
static unsigned char tx_data[N_TX_DES][AETH_PKT_LEN] ETHSECT;
#endif
static eth_des_t tx_des[N_TX_DES] ETHSECT;

#define ETH_DMACSR_TI BIT(0)
#define ETH_DMACSR_TPS BIT(1)
#define ETH_DMACSR_TBU BIT(2)
#define ETH_DMACSR_RI BIT(6)
#define ETH_DMACSR_RBU BIT(7)
#define ETH_DMACSR_RPS BIT(8)
#define ETH_DMACSR_FBE BIT(12)
#define ETH_DMACSR_CDE BIT(13)
#define ETH_DMACSR_AIS BIT(14)
#define ETH_DMACSR_NIS BIT(15)

#define ETH_MACMDIOAR_CR(v) (((v) & 0xf) << 8)
#define ETH_MACMDIOAR_OP(v) (((v) & 0x3) << 2)
#define ETH_MACMDIOAR_OP_WRITE ETH_MACMDIOAR_OP(1)
#define ETH_MACMDIOAR_OP_READ_INC ETH_MACMDIOAR_OP(2)
#define ETH_MACMDIOAR_OP_READ ETH_MACMDIOAR_OP(3)
#define ETH_MACMDIOAR_C45E BIT(1)
#define ETH_MACMDIOAR_MB BIT(0)

typedef struct {
  unsigned int currx;
  unsigned int curtx;
} eth_ctx_t;

struct netif;
struct pbuf;

void poll_rx_desc(eth_ctx_t *eth_ctx, struct netif *nif)
{
  unsigned int idx = eth_ctx->currx;
  eth_des_t *d;

#if CONFIG_LWIP
  unsigned int rem;
  struct pbuf *p, *q;
  unsigned char *dat;
#endif

  d = &rx_des[idx];

  while ((d->des[3] & ETH_RDES3_OWN) == 0) {
#if CONFIG_LWIP
    rem = d->des[3] & 0x7fff;
    dat = rx_data[idx];
#endif

#if 0
    xprintf("rx %d len: %d\n", idx, rem);
#endif

#if CONFIG_LWIP
    p = pbuf_alloc(PBUF_RAW, rem, PBUF_POOL);
    if (p) {
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
        debug_printf("lwip input error");
        pbuf_free(p);
      }
    }
#endif

    d->des[0] = (unsigned int)&rx_data[idx];
    d->des[1] = 0;
    d->des[2] = 0;
    d->des[3] = ETH_RDES3_OWN | ETH_RDES3_IOC | ETH_RDES3_BUF1V;

    idx++;
    if (idx >= N_RX_DES)
      idx = 0;
    d = &rx_des[idx];
  }
  eth_ctx->currx = idx;

  ETH->dmacrxdtpr = (unsigned int)&rx_des[N_RX_DES];
}

static void eth_irq(void *data)
{
  unsigned int dmacsr;

  dmacsr = ETH->dmacsr;
  ETH->dmacsr = 0xffffffff;

#if 0
  if (dmacsr & ETH_DMACSR_TI)
    debug_printf("tx\n");
  if (dmacsr & ETH_DMACSR_TPS)
    debug_printf("tx process stopped\n");
  if (dmacsr & ETH_DMACSR_TBU)
    debug_printf("no tx buffer\n");

#endif

  if (dmacsr & ETH_DMACSR_RI) {
#if CONFIG_LWIP
    sem_post(eth_wakeup);
#endif

#if 0
    debug_printf("rx\n");
#endif
  }
#if 0
  if (dmacsr & ETH_DMACSR_RBU)
    debug_printf("no rx buffer\n");

#endif
  if (dmacsr & ETH_DMACSR_RPS)
    debug_printf("rx process stopped\n");

#if 0
  if (dmacsr & ETH_DMACSR_AIS)
    debug_printf("AIS\n");

#endif
#if 0
  if (dmacsr & ETH_DMACSR_NIS)
    debug_printf("NIS\n");

#endif
  if (dmacsr & ETH_DMACSR_CDE)
    debug_printf("CDE\n");
  if (dmacsr & ETH_DMACSR_FBE) {
    if (dmacsr & BIT(21))
      debug_printf("%s: fatal bus error: %s %s\n", "rx",
                   (dmacsr & BIT(19)) ? "read" : "write",
                   (dmacsr & BIT(20)) ? "desc" : "data");
    if (dmacsr & BIT(18))
      debug_printf("%s: fatal bus error: %s %s\n", "tx",
                   (dmacsr & BIT(16)) ? "read" : "write",
                   (dmacsr & BIT(17)) ? "desc" : "data");
  }
}

static eth_ctx_t eth_ctx;

static int hal_eth_init(unsigned int flags)
{
  unsigned int i;
  int speed;
  unsigned int maccr = 0;
  hal_time_us_t start = hal_time_us();

  /* reset the dma */
  ETH->dmamr |= ETH_DMAMR_SWR;
  while (ETH->dmamr & ETH_DMAMR_SWR) {
    if (hal_time_us() - start > 100000) {
      debug_printf("hal_eth_init: timeout waiting for reset\n");
      return -1;
    }
  }

  ETH->macmdioar = ETH_MACMDIOAR_CR(4);

  ETH->dmasbmr = BIT(0);

  for (i = 0; i < N_RX_DES; i++) {
    eth_des_t *d = &rx_des[i];

    d->des[0] = (unsigned int)&rx_data[i];
    d->des[1] = 0;
    d->des[2] = 0;
    d->des[3] = ETH_RDES3_OWN | ETH_RDES3_IOC | ETH_RDES3_BUF1V;
  }

  memset(tx_des, 0, sizeof(tx_des));

  ETH->dmacrxdlar = (unsigned int)&rx_des[0];
  ETH->dmacrxdtpr = (unsigned int)&rx_des[N_RX_DES];
  ETH->dmacrxdrlr = N_RX_DES - 1;

  ETH->dmactxdlar = (unsigned int)&tx_des[0];
  ETH->dmactxdtpr = (unsigned int)&tx_des[0];
  ETH->dmactxdrlr = N_TX_DES - 1;

  ETH->dmaccr = AETH_PKT_LEN;

  ETH->dmactxcr = (0x20U << 16);
  ETH->dmacrxcr = (0x20U << 16) | (AETH_PKT_LEN << 1);

#if 0
  ETH->dmacier =
    ETH_DMACIER_NIE | ETH_DMACIER_AIE | ETH_DMACIER_CDEE | ETH_DMACIER_FBEE |
    ETH_DMACIER_ERIE | ETH_DMACIER_ETIE | ETH_DMACIER_RWTE | ETH_DMACIER_RSE |
    ETH_DMACIER_RBUE | ETH_DMACIER_RIE | ETH_DMACIER_TBUE | ETH_DMACIER_TXSE |
    ETH_DMACIER_TIE;
#else
  ETH->dmacier =
    ETH_DMACIER_NIE | ETH_DMACIER_AIE | ETH_DMACIER_FBEE |
    ETH_DMACIER_ETIE | ETH_DMACIER_RIE | ETH_DMACIER_TIE;
#endif

  ETH->mtltxqomr = (0 << 16) | (0 << 4) | (2 << 2);
  ETH->mtlrxqomr = 0;

  /* mac address MACA */
  ETH->maca[0].hr = BIT(31) | 0x0200;
  ETH->maca[0].lr = 0x00000001;

#define ETH_MACPFR_RA BIT(31) /* receive all */
#define ETH_MACPFR_PR BIT(0)  /* promiscuous mode */

  ETH->macpfr = ETH_MACPFR_RA | ETH_MACPFR_PR;

  ETH->macqtxfcr = 0;
  ETH->macrxfcr = 0;
  ETH->macier = 1;

  if (flags & ETH_FLAGS_PHY_FIXED) {
    speed = 0;

    if (flags & ETH_FLAGS_PHY_SPEED_100)
      speed |= PHY_SPEED_100;
    if (flags & ETH_FLAGS_PHY_FULL_DUPLEX)
      speed |= PHY_FULL_DUPLEX;
    xslog(LOG_INFO, "eth 10%s:%s-duplex",
          (speed & PHY_SPEED_100) ? "0": "",
          (speed & PHY_FULL_DUPLEX) ? "full" : "half");
  } else {
    speed = phy_reset();
    if (speed < 0)
      return -1;
  }

  if (speed & PHY_SPEED_100)
    maccr |= BIT(14);

  if (speed & PHY_FULL_DUPLEX)
    maccr |= BIT(13);

  maccr |= BIT(1) | BIT(0);  /* enable tx and rx */

  ETH->maccr = maccr;

  __DSB();

  ETH->dmacrxcr |= ETH_DMARXCR_ST; /* enable rx dma */

  memset(&eth_ctx, 0, sizeof(eth_ctx_t));

  irq_register("eth", eth_irq, &eth_ctx, ETH_IRQ);

  return 0;
}

int phy_read(unsigned int phy, unsigned int reg)
{
  int count = 0;

  ETH->macmdioar = ((phy & 0x1f ) << 21) | ((reg & 0x1f) << 16) | \
                   ETH_MACMDIOAR_CR(4) | \
                   ETH_MACMDIOAR_OP_READ | \
                   ETH_MACMDIOAR_MB;

  while (ETH->macmdioar & ETH_MACMDIOAR_MB)
    count++;

  return ETH->macmdiodr & 0xffff;
}

void phy_write(unsigned int phy, unsigned int reg, unsigned int val)
{
  int count = 1000;

  ETH->macmdiodr = val & 0xffff;

  ETH->macmdioar = ((phy & 0x1f ) << 21) | ((reg & 0x1f) << 16) | \
                   ETH_MACMDIOAR_CR(4) | \
                   ETH_MACMDIOAR_OP_WRITE | \
                   ETH_MACMDIOAR_MB;

  while ((ETH->macmdioar & ETH_MACMDIOAR_MB) && count) {
    count++;
    hal_delay_us(100);
  }
  if (count == 0)
    debug_printf("timeout waiting for phy\n");
}

int cmd_eth(int argc, char *argv[])
{
  int v, i, phyaddr = 0;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'i':
    hal_eth_init(0);
    break;
  case 'r':
    if (argc >= 3)
      phyaddr = strtoul(argv[2], NULL, 0);
    xprintf("PHYADDR %d\n", phyaddr);
    for (i = 0; i < 32; i++) {
      v = phy_read(phyaddr, i);
      xprintf("%02x: %04x\n", i, v);
    }
    break;
  case 'x':
    phy_reset();
    break;
  case 'a':
    xprintf("dmaisr %08x\n", ETH->dmaisr);
    xprintf("dmacsr %08x\n", ETH->dmacsr);
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

  d = &tx_des[idx];

  if ((d->des[3] & ETH_TDES3_OWN) != 0) {
    FAST_LOG('e', "no free tx eth buffer\n", 0, 0);
    return -1;
  }

  dat = tx_data[idx];
  for (q = p; q; q = q->next) {
    unsigned int l = q->len;

    memcpy(dat, q->payload, l);

    dat += l;
    len += l;
  }

  XASSERT(len <= ETH_PKT_LEN);

#if 0
  xprintf("tx %d len %d\n", idx, len);
#endif

  d->des[0] = (unsigned int)&tx_data[idx];
  d->des[1] = 0;
  d->des[2] = ETH_TDES2_IOC | ETH_TDES2_B1L(len);
  d->des[3] = ETH_TDES3_OWN | ETH_TDES3_FD | ETH_TDES3_LD | \
              ETH_TDES3_FL(len);

  idx++;

  if (idx >= N_TX_DES)
    idx = 0;

  __DSB();

  ETH->dmactxdtpr = (unsigned int)&tx_des[idx];

  __DSB();

  eth_ctx.curtx = idx;

  ETH->dmactxcr |= ETH_DMATXCR_ST; /* enable tx dma */

  return 0;
}

static hal_eth_config_t hal_eth_config_default = {
  .hwaddr = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x01 },
  .flags  = 0
};

WEAK void hal_eth_config_get(hal_eth_config_t *config)
{
  *config = hal_eth_config_default;
}

err_t eth_init(struct netif *nif)
{
  hal_eth_config_t config;

  nif->output = etharp_output;
  nif->linkoutput = hal_eth_send;

  hal_eth_config_get(&config);

  nif->hwaddr_len = ETH_HWADDR_LEN;
  memcpy(nif->hwaddr, config.hwaddr, ETH_HWADDR_LEN);

  nif->mtu = 1500;
  nif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  nif->flags |= NETIF_FLAG_LINK_UP;

  if (hal_eth_init(config.flags) < 0)
    return -1;

  return 0;
}

void eth_input(struct netif *nif)
{
  poll_rx_desc(&eth_ctx, nif);
}
#endif
