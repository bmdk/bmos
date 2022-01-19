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
#include "fast_log.h"

#if CONFIG_LWIP
#include "lwip/netif.h"
#include "lwip/etharp.h"
#include "stm32_eth.h"
#endif

#define ETH_BASE 0x40028000

typedef struct {
  unsigned int maccr;
  unsigned int macecr;
  unsigned int macpfr;
  unsigned int macwtr;
  unsigned int mact0r;
  unsigned int mact1r;
  unsigned int rsvd0[14];
  unsigned int macvtr;
  unsigned int macvhtr;
  unsigned int rsvd1;
  unsigned int macvir;
  unsigned int macivir;
  unsigned int rsvd2[3];
  unsigned int macqtxfcr;
  unsigned int rsvd3[7];
  unsigned int macrxfcr;
  unsigned int rsvd4[7];
  unsigned int macisr;
  unsigned int macier;
  unsigned int macrxtxsr;
  unsigned int rsvd5;
  unsigned int macpcsr;
  unsigned int macrwkpfr;
  unsigned int rsvd6[2];
  unsigned int maclcsr;
  unsigned int macltcr;
  unsigned int macletr;
  unsigned int mac1ustcr;
  unsigned int rsvd7[12];
  unsigned int macvr;
  unsigned int macdr;
  unsigned int rsvd8[2];
  unsigned int machwf1r;
  unsigned int machwf2r;
  unsigned int rsvd9[54];
  unsigned int macmdioar;
  unsigned int macmdiodr;
  unsigned int rsvd10[2];
  unsigned int macarpar;
  unsigned int rsvd11[59];
  struct {
    unsigned int hr;
    unsigned int lr;
  } maca[4];
  unsigned int rsvd12[248];
  unsigned int mmc_control;
  unsigned int mmc_rx_interrupt;
  unsigned int mmc_tx_interrupt;
  unsigned int mmc_rx_interrupt_mask;
  unsigned int mmc_tx_interrupt_mask;
  unsigned int rsvd13[14];
  unsigned int txsnglcolg;
  unsigned int txmultcolg;
  unsigned int rsvd14[5];
  unsigned int txpktg;
  unsigned int rsvd15[10];
  unsigned int rxcrcerr;
  unsigned int rxalgnerr;
  unsigned int rsvd16[10];
  unsigned int rxucastg;
  unsigned int rsvd17[9];
  unsigned int txlpiusc;
  unsigned int txlpitrc;
  unsigned int rxlpiusc;
  unsigned int rxlpitrc;
  unsigned int rsvd18[65];
  unsigned int macl3l4c0r;
  unsigned int macl4a0r;
  unsigned int rsvd19[2];
  unsigned int macl3a00r;
  unsigned int macl3a10r;
  unsigned int macl3a20r;
  unsigned int macl3a30r;
  unsigned int rsvd20[4];
  unsigned int macl3l4c1r;
  unsigned int macl4a1r;
  unsigned int rsvd21[2];
  unsigned int macl3a01r;
  unsigned int macl3a11r;
  unsigned int macl3a21r;
  unsigned int macl3a31r;
  unsigned int rsvd22[108];
  unsigned int mactscr;
  unsigned int macssir;
  unsigned int macstsr;
  unsigned int macstnr;
  unsigned int macstsur;
  unsigned int macstnur;
  unsigned int mactsar;
  unsigned int rsvd23;
  unsigned int mactssr;
  unsigned int rsvd24[3];
  unsigned int mactxtssnr;
  unsigned int mactxtsssr;
  unsigned int rsvd25[2];
  unsigned int macacr;
  unsigned int rsvd26;
  unsigned int macatsnr;
  unsigned int macatssr;
  unsigned int mactsiacr;
  unsigned int mactseacr;
  unsigned int mactsicnr;
  unsigned int mactsecnr;
  unsigned int rsvd27[4];
  unsigned int macppscr;
  unsigned int rsvd28[3];
  unsigned int macppsttsr;
  unsigned int macppsttnr;
  unsigned int macppsir;
  unsigned int macppswr;
  unsigned int rsvd29[12];
  unsigned int macpocr;
  unsigned int macspi0r;
  unsigned int macspi1r;
  unsigned int macspi2r;
  unsigned int maclmir;
  unsigned int rsvd30[11];
  unsigned int mtlomr;
  unsigned int rsvd31[7];
  unsigned int mtlisr;
  unsigned int rsvd32[55];
  unsigned int mtltxqomr;
  unsigned int mtltxqur;
  unsigned int mtltxqdr;
  unsigned int rsvd33[8];
  unsigned int mtlqicsr;
  unsigned int mtlrxqomr;
  unsigned int mtlrxqmpocr;
  unsigned int mtlrxqdr;
  unsigned int rsvd34[177];
  unsigned int dmamr;
  unsigned int dmasbmr;
  unsigned int dmaisr;
  unsigned int dmadsr;
  unsigned int rsvd35[60];
  unsigned int dmaccr;
  unsigned int dmactxcr;
  unsigned int dmacrxcr;
  unsigned int rsvd36[2];
  unsigned int dmactxdlar;
  unsigned int rsvd37;
  unsigned int dmacrxdlar;
  unsigned int dmactxdtpr;
  unsigned int rsvd38;
  unsigned int dmacrxdtpr;
  unsigned int dmactxdrlr;
  unsigned int dmacrxdrlr;
  unsigned int dmacier;
  unsigned int dmacrxiwtr;
  unsigned int rsvd39[2];
  unsigned int dmaccatxdr;
  unsigned int rsvd40;
  unsigned int dmaccarxdr;
  unsigned int rsvd41;
  unsigned int dmaccatxbr;
  unsigned int rsvd42;
  unsigned int dmaccarxbr;
  unsigned int dmacsr;
  unsigned int rsvd43[2];
  unsigned int dmacmfcr;
} stm32_eth_h7xx_t;

#define ETH ((volatile stm32_eth_h7xx_t *)ETH_BASE)

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

#define ETHSECT __attribute__((section(".eth"), aligned(8)))

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

static int hal_eth_init()
{
  unsigned int i;

  /* reset the dma */
  ETH->dmamr |= ETH_DMAMR_SWR;
  while (ETH->dmamr & ETH_DMAMR_SWR)
    ;

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
  ETH->macier = 0;
  ETH->maccr = BIT(14) | BIT(13); /* 100Mbit Full Duplex */
  ETH->maccr |= BIT(1) | BIT(0);  /* enable tx and rx */

  __DSB();

  ETH->dmacrxcr |= ETH_DMARXCR_ST; /* enable rx dma */

  memset(&eth_ctx, 0, sizeof(eth_ctx_t));

  irq_register("eth", eth_irq, &eth_ctx, 61);

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

void phy_reset()
{
  unsigned int count;

  phy_write(0, 0, BIT(15)); /* reset phy */

  hal_delay_us(100);
  while (phy_read(0, 0) & BIT(15))
    ;

  hal_delay_us(100);

  phy_write(0, 0, BIT(13) | BIT(9) | BIT(8));
  count = 5000;
  while ((phy_read(0, 0) & BIT(9)) && count) {
    hal_delay_us(100);
    count--;
  }
  if (count == 0) {
    xprintf("timeout\n");
    return;
  }
}

int cmd_eth(int argc, char *argv[])
{
  int v, i;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'i':
    hal_eth_init();
    break;
  case 'r':
    for (i = 0; i < 32; i++) {
      v = phy_read(0, i);
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

err_t eth_init(struct netif *nif)
{
  nif->output = etharp_output;
  nif->linkoutput = hal_eth_send;

  nif->hwaddr_len = ETH_HWADDR_LEN;
  nif->hwaddr[0] = 0x02;
  nif->hwaddr[1] = 0x00;
  nif->hwaddr[2] = 0x00;
  nif->hwaddr[3] = 0x00;
  nif->hwaddr[4] = 0x00;
  nif->hwaddr[5] = 0x01;

  nif->mtu = 1500;
  nif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  nif->flags |= NETIF_FLAG_LINK_UP;

  hal_eth_init();

  return 0;
}

void eth_input(struct netif *nif)
{
  poll_rx_desc(&eth_ctx, nif);
}
#endif
