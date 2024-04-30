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
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "hal_gpio.h"
#include "hal_int.h"
#include "hal_time.h"
#include "io.h"
#include "shell.h"
#include "stm32_hal_spi.h"
#include "stm32_hal.h"

#include "lwip/netif.h"
#include "lwip/etharp.h"

#include "stm32_exti.h"
#include "stm32_eth.h"

#include "enc28j60.h"

#if STM32_H7XX
#define SPI6_BASE 0x58001400
#endif

#define ECON1_TXRTS BIT(3)
#define ECON1_RXEN BIT(2)

#define ECON2_AUTOINC BIT(7)
#define ECON2_PKTDEC BIT(6)

#define MACON1_TXPAUS BIT(3)
#define MACON1_RXPAUS BIT(2)
#define MACON1_PASSALL BIT(1)
#define MACON1_MARXEN BIT(0)

#define MACON3_PAD(_v_) (((_v_) & 0x7) << 5)
#define MACON3_TXCRCEN BIT(4)
#define MACON3_FULDPX BIT(0)

#define EIE_INTIE BIT(7)
#define EIE_PKTIE BIT(6)
#define EIE_DMAIE BIT(5)
#define EIE_LINKIE BIT(4)
#define EIE_TXIE BIT(3)
#define EIE_TXERIE BIT(1)
#define EIE_RXERIE BIT(0)

#define EIR_PKTIF BIT(6)
#define EIR_DMAIF BIT(5)
#define EIR_LINKIF BIT(4)
#define EIR_TXIF BIT(3)
#define EIR_TXERIF BIT(1)
#define EIR_RXERIF BIT(0)

/* tx/rx memory start and end */
#define RXST 0x0000
#define TXST 0x1800
#define RXND (TXST - 1)

#define MAX_PKTLEN 1518

/* ENC28J60 */
typedef struct {
  unsigned char bank;
  unsigned char exti;
  unsigned char irq;
  stm32_hal_spi_t *spi;
} enc28j60_data_t;

static enc28j60_data_t enc28j60_data;

static void enc28j60_reset(void)
{
  unsigned char cmd = ENC_SRS;

  stm32_hal_spi_write_buf(enc28j60_data.spi, &cmd, 1);
}

/* The underscore prefixed functions do not handle bank changing so they
 * should only be used for the registers available in all banks or when
 * we know what we are doing.
 */
static void _enc28j60_wr(unsigned int reg, unsigned int v)
{
  unsigned char buf[2];

  buf[0] = ENC_WCR(reg);
  buf[1] = v & 0xff;
  stm32_hal_spi_write_buf(enc28j60_data.spi, buf, 2);
}

static unsigned int _enc28j60_rd(unsigned int reg)
{
  unsigned char cmd, val;

  cmd = ENC_RCR(reg);
  stm32_hal_spi_wrd_buf(enc28j60_data.spi, &cmd, 1, &val, 1);

  return (unsigned int)val;
}

static void _enc28j60_bit_set(unsigned int reg, unsigned int mask)
{
  unsigned char buf[2];

  buf[0] = ENC_BFS(reg);
  buf[1] = mask & 0xff;
  stm32_hal_spi_write_buf(enc28j60_data.spi, buf, 2);
}

static void _enc28j60_bit_clear(unsigned int reg, unsigned int mask)
{
  unsigned char buf[2];

  buf[0] = ENC_BFC(reg);
  buf[1] = mask & 0xff;
  stm32_hal_spi_write_buf(enc28j60_data.spi, buf, 2);
}

static void _enc28j60_set_clear(unsigned int reg, unsigned int clear,
                                unsigned int set)
{
  unsigned int v;

  v = _enc28j60_rd(reg);
  v &= ~clear;
  v |= set;
  _enc28j60_wr(reg, v);
}

static void enc28j60_set_bank(unsigned int bank)
{
  bank &= 3;
  if (bank == enc28j60_data.bank)
    return;

  _enc28j60_set_clear(ENC28J60_ECON1, 0x3, bank);

  enc28j60_data.bank = bank;
}

static void enc28j60_wr(unsigned int reg, unsigned int v)
{
  unsigned int bank = (reg >> 5) & 0x7;

  if (bank != 0)
    enc28j60_set_bank(bank - 1);

  _enc28j60_wr(reg, v);
}

/* write a 16 bit value to two sucessive registers - low register first */
static void enc28j60_wr16(unsigned int reg, unsigned int v)
{
  enc28j60_wr(reg, v & 0xff);
  enc28j60_wr(reg + 1, (v >> 8) & 0xff);
}

unsigned int enc28j60_rd(unsigned int reg)
{
  unsigned int bank = (reg >> 5) & 0x7;

  if (bank != 0)
    enc28j60_set_bank(bank - 1);

  return _enc28j60_rd(reg);
}

static stm32_hal_spi_t spi = {
  .base    = (void *)SPI1_BASE,
  .wordlen = 8,
  .div     = 12,
  .cs      = GPIO(0, 4)
};

static void enc28j60_int(void *data)
{
  enc28j60_data_t *d = (enc28j60_data_t *)data;

  stm32_exti_irq_ack(d->exti);

  sem_post(eth_wakeup);
}

/*  ERXRDPTL must never be set to an even value - erratum 14 */
static unsigned int enc28j60_erxrdptl_errata(unsigned int val)
{
  if (val & 1)
    return val;

  /* reduce even value by one allowing for region wrap - assumes
     RXST is zero as it must be for another erratum (5) */
  val -= 1;
  if (val > RXND)
    val = RXND;
  return val;
}

static void enc28j60_init(stm32_hal_spi_t *spi, unsigned char *macaddr,
                          unsigned int irq, unsigned int exti)
{
  enc28j60_data.spi = spi;
  enc28j60_data.irq = (unsigned char)irq;
  enc28j60_data.exti = (unsigned char)exti;

  enc28j60_reset();

  hal_delay_us(40);

  enc28j60_set_bank(0);

  enc28j60_wr16(ENC28J60_ERXSTL, RXST);
  enc28j60_wr16(ENC28J60_ERXRDPTL, RXND); /* errata - must be odd */
  enc28j60_wr16(ENC28J60_ERDPTL, RXST);
  enc28j60_wr16(ENC28J60_ERXNDL, RXND);

  enc28j60_wr(ENC28J60_MACON1, MACON1_TXPAUS | MACON1_RXPAUS | MACON1_MARXEN);
  enc28j60_wr(ENC28J60_MACON3, MACON3_PAD(1) | MACON3_TXCRCEN | MACON3_FULDPX);
  enc28j60_wr(ENC28J60_MABBIPG, 0x15); /* full duplex */

  enc28j60_wr16(ENC28J60_MAMXFLL, MAX_PKTLEN);

  enc28j60_wr(ENC28J60_MAADR1, macaddr[0]);
  enc28j60_wr(ENC28J60_MAADR2, macaddr[1]);
  enc28j60_wr(ENC28J60_MAADR3, macaddr[2]);
  enc28j60_wr(ENC28J60_MAADR4, macaddr[3]);
  enc28j60_wr(ENC28J60_MAADR5, macaddr[4]);
  enc28j60_wr(ENC28J60_MAADR6, macaddr[5]);

  stm32_exti_irq_ack(exti);
  irq_register("enc28j60", enc28j60_int, &enc28j60_data, irq);

  /* enable rx interrupts */
  _enc28j60_bit_set(ENC28J60_EIE, EIE_INTIE | EIE_PKTIE);
  /* enable rx */
  _enc28j60_bit_set(ENC28J60_ECON1, ECON1_RXEN);
}

#if 0
static void enc28j60_wr_phy(unsigned int reg, unsigned int v)
{
  enc28j60_wr(ENC28J60_MIREGADR, reg);
  enc28j60_wr(ENC28J60_MIWRL, v);
  enc28j60_wr(ENC28J60_MIWRH, v >> 8);
  /* MISTAT.BUSY */
}

static void enc28j60_memdump(unsigned int addr, unsigned int len)
{
  unsigned char buf[16];
  unsigned int rlen, i;
  unsigned char cmd;

  enc28j60_wr16(ENC28J60_ERDPTL, addr);

  while (len > 0) {
    if (len > sizeof(buf))
      rlen = sizeof(buf);
    else
      rlen = len;

    cmd = ENC_RBM;
    stm32_hal_spi_wrd_buf(enc28j60_data.spi, &cmd, 1, buf, rlen);

    for (i = 0; i < sizeof(buf); i++)
      xprintf("%02x ", buf[i]);
    xprintf("\n");

    len -= rlen;
  }
}

static int enc_cmd(int argc, char *argv[])
{
  unsigned int addr = 0, val = 0, bank = 0;
  unsigned char buf[64];

  switch (argv[1][0]) {
  case 'r':
    if (argc > 3) {
      bank = strtoul(argv[2], 0, 0);
      addr = strtoul(argv[3], 0, 16);
    }

    enc28j60_set_bank(bank);
    val = _enc28j60_rd(addr);

    xprintf("v(%d,%02x): %02x\n", bank, addr, val);
    break;
  case 'l':
    if (argc > 2)
      val = strtoul(argv[2], 0, 0);

    val &= 0xf;

    enc28j60_wr_phy(ENC28J60_PHY_PHLCON, (val << 8) | (val << 4));
    break;
  case 'd':
    enc28j60_memdump(0, 64);
    break;
  }

  return 0;
}

SHELL_CMD(enc, enc_cmd);
#endif

static void poll_rx_desc(struct netif *nif)
{
  unsigned char cmd, eir;
  unsigned char buf[16];
  unsigned int next, count, rem;

#if 0
  unsigned int flags;
#endif
  struct pbuf *p, *q;

  for (;;) {
    eir = enc28j60_rd(ENC28J60_EIR);
    if ((eir & EIR_PKTIF) == 0)
      break;

    cmd = ENC_RBM;
    stm32_hal_spi_wrd_buf(enc28j60_data.spi, &cmd, 1, buf, 6);

    next = (buf[1] << 8) + buf[0];
    count = (buf[3] << 8) + buf[2];
#if 0
    flags = (buf[5] << 8) + buf[4];
#endif

    rem = count;
    p = pbuf_alloc(PBUF_RAW, rem, PBUF_POOL);
    if (p) {
      err_t err;

      for (q = p; q && (rem > 0); q = q->next) {
        unsigned int cnt = q->len;

        if (rem < cnt)
          cnt = rem;

        stm32_hal_spi_wrd_buf(enc28j60_data.spi, &cmd, 1, q->payload, cnt);

        rem -= cnt;
      }

      err = nif->input(p, nif);
      if (err != ERR_OK) {
        xprintf("lwip input error");
        pbuf_free(p);
      }
    }

#if 0
    if (count & 1)
      stm32_hal_spi_wrd_buf(enc28j60_data.spi, &cmd, 1, buf, 1);
#endif

    /* move the read pointers to the next packet */
    enc28j60_wr16(ENC28J60_ERXRDPTL, enc28j60_erxrdptl_errata(next));
    enc28j60_wr16(ENC28J60_ERDPTL, next);

    _enc28j60_bit_set(ENC28J60_ECON2, ECON2_PKTDEC);
  }
}

static err_t hal_eth_send(struct netif *netif, struct pbuf *p)
{
  unsigned int v, len;
  char cmd, val;
  struct pbuf *q;

  v = _enc28j60_rd(ENC28J60_ECON1);
  if (v & ECON1_TXRTS)
    return -1;

  /* clear tx done interrupt */
  _enc28j60_bit_clear(ENC28J60_EIR, EIR_TXIF);

  /* write packet */
  enc28j60_wr16(ENC28J60_ETXSTL, TXST);
  enc28j60_wr16(ENC28J60_EWRPTL, TXST);

  cmd = ENC_WBM;
  val = 0; /* no flag override */
  stm32_hal_spi_write_buf2(enc28j60_data.spi, &cmd, 1, &val, 1);

  len = 0;
  for (q = p; q; q = q->next) {
    unsigned int l = q->len;

    stm32_hal_spi_write_buf2(enc28j60_data.spi, &cmd, 1, q->payload, q->len);

    len += l;
  }

  enc28j60_wr16(ENC28J60_ETXNDL, TXST + len); /* points to last data byte */

  /* activate tx */
  _enc28j60_bit_set(ENC28J60_ECON1, ECON1_TXRTS);

  return 0;
}

#if BOARD_F411BP
#define ENC_IRQ 9
#define ENC_EXTI 3
#else
#error Define board interrupt
#endif

err_t eth_init(struct netif *nif)
{
  nif->output = etharp_output;
  nif->linkoutput = hal_eth_send;

  nif->hwaddr_len = ETH_HWADDR_LEN;
  nif->hwaddr[0] = 0x02;

  stm32_get_udid(&nif->hwaddr[1], 5);

  nif->mtu = 1500;
  nif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  nif->flags |= NETIF_FLAG_LINK_UP;

  stm32_hal_spi_init(&spi);
  enc28j60_init(&spi, nif->hwaddr, ENC_IRQ, ENC_EXTI);

  return 0;
}

void eth_input(struct netif *nif)
{
  poll_rx_desc(nif);
}
