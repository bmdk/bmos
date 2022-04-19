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

#ifndef ENC28J60_H
#define ENC28J60_H

/* spi commands */
#define ENC_RCR(_a_) (0x00 | ((_a_) & 0x1f))
#define ENC_RBM 0x3a
#define ENC_WCR(_a_) (0x40 | ((_a_) & 0x1f))
#define ENC_WBM 0x7a
#define ENC_BFS(_a_) (0x80 | ((_a_) & 0x1f))
#define ENC_BFC(_a_) (0xa0 | ((_a_) & 0x1f))
#define ENC_SRS 0xff

/* registers in all banks */
#define ENC28J60_EIE 0x1B
#define ENC28J60_EIR 0x1C
#define ENC28J60_ESTAT 0x1D
#define ENC28J60_ECON2 0x1E
#define ENC28J60_ECON1 0x1F

#define BANK(_b_, _r_) (((_b_) << 5) | ((_r_) & 0x1f))

/* bank 1 registers */
#define ENC28J60_ERDPTL BANK(1, 0x00)
#define ENC28J60_ERDPTH BANK(1, 0x01)
#define ENC28J60_EWRPTL BANK(1, 0x02)
#define ENC28J60_EWRPTH BANK(1, 0x03)
#define ENC28J60_ETXSTL BANK(1, 0x04)
#define ENC28J60_ETXSTH BANK(1, 0x05)
#define ENC28J60_ETXNDL BANK(1, 0x06)
#define ENC28J60_ETXNDH BANK(1, 0x07)
#define ENC28J60_ERXSTL BANK(1, 0x08)
#define ENC28J60_ERXSTH BANK(1, 0x09)
#define ENC28J60_ERXNDL BANK(1, 0x0A)
#define ENC28J60_ERXNDH BANK(1, 0x0B)
#define ENC28J60_ERXRDPTL BANK(1, 0x0C)
#define ENC28J60_ERXRDPTH BANK(1, 0x0D)
#define ENC28J60_ERXWRPTL BANK(1, 0x0E)
#define ENC28J60_ERXWRPTH BANK(1, 0x0F)
#define ENC28J60_EDMASTL BANK(1, 0x10)
#define ENC28J60_EDMASTH BANK(1, 0x11)
#define ENC28J60_EDMANDL BANK(1, 0x12)
#define ENC28J60_EDMANDH BANK(1, 0x13)
#define ENC28J60_EDMADSTH BANK(1, 0x15)
#define ENC28J60_EDMACSL BANK(1, 0x16)
#define ENC28J60_EDMACSH BANK(1, 0x17)

/* bank 2 registers */
#define ENC28J60_EHT0 BANK(2, 0x00)
#define ENC28J60_EHT1 BANK(2, 0x01)
#define ENC28J60_EHT2 BANK(2, 0x02)
#define ENC28J60_EHT3 BANK(2, 0x03)
#define ENC28J60_EHT4 BANK(2, 0x04)
#define ENC28J60_EHT5 BANK(2, 0x05)
#define ENC28J60_EHT6 BANK(2, 0x06)
#define ENC28J60_EHT7 BANK(2, 0x07)
#define ENC28J60_EPMM0 BANK(2, 0x08)
#define ENC28J60_EPMM1 BANK(2, 0x09)
#define ENC28J60_EPMM2 BANK(2, 0x0A)
#define ENC28J60_EPMM3 BANK(2, 0x0B)
#define ENC28J60_EPMM4 BANK(2, 0x0C)
#define ENC28J60_EPMM5 BANK(2, 0x0D)
#define ENC28J60_EPMM6 BANK(2, 0x0E)
#define ENC28J60_EPMM7 BANK(2, 0x0F)
#define ENC28J60_EPMCSL BANK(2, 0x10)
#define ENC28J60_EPMCSH BANK(2, 0x11)
#define ENC28J60_EPMOH BANK(2, 0x15)
#define ENC28J60_ERXFCON BANK(2, 0x18)
#define ENC28J60_EPKTCNT BANK(2, 0x19)

/* bank 3 registers */
#define ENC28J60_MACON1 BANK(3, 0x00)
#define ENC28J60_MACON3 BANK(3, 0x02)
#define ENC28J60_MACON4 BANK(3, 0x03)
#define ENC28J60_MABBIPG BANK(3, 0x04)
#define ENC28J60_MAIPGL BANK(3, 0x06)
#define ENC28J60_MAIPGH BANK(3, 0x07)
#define ENC28J60_MACLCON1 BANK(3, 0x08)
#define ENC28J60_MACLCON2 BANK(3, 0x09)
#define ENC28J60_MAMXFLL BANK(3, 0x0A)
#define ENC28J60_MAMXFLH BANK(3, 0x0B)
#define ENC28J60_MICMD BANK(3, 0x12)
#define ENC28J60_MIREGADR BANK(3, 0x14)
#define ENC28J60_MIWRL BANK(3, 0x16)
#define ENC28J60_MIWRH BANK(3, 0x17)
#define ENC28J60_MIRDL BANK(3, 0x18)
#define ENC28J60_MIRDH BANK(3, 0x19)

/* bank 4 registers */
#define ENC28J60_MAADR5 BANK(4, 0x00)
#define ENC28J60_MAADR6 BANK(4, 0x01)
#define ENC28J60_MAADR3 BANK(4, 0x02)
#define ENC28J60_MAADR4 BANK(4, 0x03)
#define ENC28J60_MAADR1 BANK(4, 0x04)
#define ENC28J60_MAADR2 BANK(4, 0x05)
#define ENC28J60_EBSTSD BANK(4, 0x06)
#define ENC28J60_EBSTCON BANK(4, 0x07)
#define ENC28J60_EBSTCSL BANK(4, 0x08)
#define ENC28J60_EBSTCSH BANK(4, 0x09)
#define ENC28J60_MISTAT BANK(4, 0x0A)
#define ENC28J60_EREVID BANK(4, 0x12)
#define ENC28J60_ECOCON BANK(4, 0x15)
#define ENC28J60_EFLOCON BANK(4, 0x17)
#define ENC28J60_EPAUSL BANK(4, 0x18)
#define ENC28J60_EPAUSH BANK(4, 0x19)

/* phy registers */
#define ENC28J60_PHY_PHCON1 0x00
#define ENC28J60_PHY_PHSTAT1 0x01
#define ENC28J60_PHY_PHID1 0x02
#define ENC28J60_PHY_PHID2 0x03
#define ENC28J60_PHY_PHCON2 0x10
#define ENC28J60_PHY_PHSTAT2 0x11
#define ENC28J60_PHY_PHIE 0x12
#define ENC28J60_PHY_PHIR 0x13
#define ENC28J60_PHY_PHLCON 0x14

/* led config bits - PHLCON */
#define ENC28J60_LED_CFG_TX 1
#define ENC28J60_LED_CFG_RX 2
#define ENC28J60_LED_CFG_COLL 3
#define ENC28J60_LED_CFG_LINK 4
#define ENC28J60_LED_CFG_DUPLEX 5
#define ENC28J60_LED_CFG_TX_RX 7
#define ENC28J60_LED_CFG_ON 8
#define ENC28J60_LED_CFG_OFF 9
#define ENC28J60_LED_CFG_BLINK_FAST 10
#define ENC28J60_LED_CFG_BLINK_SLOW 11
#define ENC28J60_LED_CFG_BLINK_LINK_RX 12
#define ENC28J60_LED_CFG_BLINK_LINK_RX_TX 13
#define ENC28J60_LED_CFG_BLINK_DUPLEX_COLL 14

#endif
