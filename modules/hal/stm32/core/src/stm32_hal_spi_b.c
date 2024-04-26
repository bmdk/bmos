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

#include "stm32_hal_spi.h"
#include "common.h"

#include "io.h"

/* H7XX */
#define SPI1_BASE 0x40013000
#define SPI2_BASE 0x40003800
#define SPI3_BASE 0x40003C00
#define SPI4_BASE 0x40013400
#define SPI5_BASE 0x40015000
#define SPI6_BASE 0x58001400

typedef struct {
  reg32_t cr1;
  reg32_t cr2;
  reg32_t cfg1;
  reg32_t cfg2;
  reg32_t ier;
  reg32_t sr;
  reg32_t ifcr;
  reg32_t autocr;
  union {
    reg8_t b;
    reg16_t h;
    reg32_t w;
  } txdr;
  unsigned int pad2[3];
  reg32_t rxdr;
  unsigned int pad3[3];
  reg32_t crcpoly;
  reg32_t txcrc;
  reg32_t rxcrc;
  reg32_t urdr;
} stm32_spi_b_t;

#define STM32_SPI_CR1_IOLOCK BIT(16)
#define STM32_SPI_CR1_SSI BIT(12)
#define STM32_SPI_CR1_HDDIR BIT(11)
#define STM32_SPI_CR1_CSUSP BIT(10)
#define STM32_SPI_CR1_CSTART BIT(9)
#define STM32_SPI_CR1_SPE BIT(0)

#define STM32_SPI_CFG1_TXDMAEN BIT(15)
#define STM32_SPI_CFG1_RXDMAEN BIT(14)
#define STM32_SPI_CFG1_MBR(_v_) (((_v_) & 0x7) << 28)
#define STM32_SPI_CFG1_FTHLV(_v_) (((_v_) & 0xf) << 5)
#define STM32_SPI_CFG1_DSIZE(_v_) ((_v_) & 0x1f)

#define STM32_SPI_CFG2_AFCNTR BIT(31)
#define STM32_SPI_CFG2_SSOM BIT(30)
#define STM32_SPI_CFG2_SSOE BIT(29)
#define STM32_SPI_CFG2_SSIOP BIT(28)
#define STM32_SPI_CFG2_SSM BIT(26)
#define STM32_SPI_CFG2_CPOL BIT(25)
#define STM32_SPI_CFG2_CPHA BIT(24)
#define STM32_SPI_CFG2_LSBFRST BIT(23)
#define STM32_SPI_CFG2_MASTER BIT(22)
#define STM32_SPI_CFG2_SP(_v_) (((_v_) & 1) << 19)
#define STM32_SPI_CFG2_COMM(_v_) (((_v_) & 3) << 17)
#define STM32_SPI_CFG2_IOSWP BIT(15)

#define STM32_SPI_SR_TXC BIT(12)
#define STM32_SPI_SR_SUSP BIT(11)
#define STM32_SPI_SR_TSERF BIT(10)
#define STM32_SPI_SR_TXTF BIT(4)
#define STM32_SPI_SR_EOT BIT(3)
#define STM32_SPI_SR_DXP BIT(2)
#define STM32_SPI_SR_TXP BIT(1)
#define STM32_SPI_SR_RXP BIT(0)

/* rounded up log2 */
static int _xlog2(unsigned int div)
{
  int i;

  if (div == 1)
    return 0;

  for (i = 31; i > 0; i--) {
    if (div > BIT(i - 1))
      return i;
  }

  return -1;
}

void stm32_hal_spi_init(stm32_hal_spi_t *s)
{
  stm32_spi_b_t *spi = s->base;
  int mbr = _xlog2(s->div) - 1;
  unsigned int cfg2;

  /* minimum divider is 2 = 2 ^ (0 + 1) */
  if (mbr < 0)
    mbr = 0;

  spi->cr1 &= ~STM32_SPI_CR1_SPE;
  spi->cr1 |= STM32_SPI_CR1_SSI;
  spi->cfg1 = STM32_SPI_CFG1_MBR(mbr) |
              STM32_SPI_CFG1_DSIZE((unsigned int)s->wordlen - 1);

  cfg2 = STM32_SPI_CFG2_AFCNTR | STM32_SPI_CFG2_SSM | STM32_SPI_CFG2_MASTER;

  if (s->flags & STM32_SPI_FLAG_CPOL)
    cfg2 |= STM32_SPI_CFG2_CPOL;
  if (s->flags & STM32_SPI_FLAG_CPHA)
    cfg2 |= STM32_SPI_CFG2_CPHA;

  spi->cfg2 = cfg2;
  spi->cr1 |= STM32_SPI_CR1_SPE;

  gpio_set(s->cs, 1);
  gpio_init(s->cs, GPIO_OUTPUT);
}

static void _stm32_hal_spi_write(stm32_hal_spi_t *s, unsigned int data)
{
  stm32_spi_b_t *spi = s->base;

  while ((spi->sr & STM32_SPI_SR_TXP) == 0)
    ;

  spi->txdr.b = (unsigned char)data;

  spi->cr1 |= STM32_SPI_CR1_CSTART;
}

static unsigned int _stm32_hal_spi_wrd(stm32_hal_spi_t *s, unsigned int data)
{
  stm32_spi_b_t *spi = s->base;

  /* discard old data */
  while (spi->sr & STM32_SPI_SR_RXP)
    (void)spi->rxdr;

  spi->txdr.b = (unsigned char)data;

  spi->cr1 |= STM32_SPI_CR1_CSTART;

  while ((spi->sr & STM32_SPI_SR_RXP) == 0)
    ;

  return spi->rxdr;
}

static unsigned int _stm32_hal_spi_read(stm32_hal_spi_t *s)
{
  return _stm32_hal_spi_wrd(s, 0xffff);
}

static void _stm32_hal_spi_wait_done(stm32_hal_spi_t *s)
{
  stm32_spi_b_t *spi = s->base;

  while ((spi->sr & STM32_SPI_SR_TXC) == 0)
    ;
}

void stm32_hal_spi_write(stm32_hal_spi_t *s, unsigned int data)
{
  gpio_set(s->cs, 0);

  _stm32_hal_spi_write(s, data);

  _stm32_hal_spi_wait_done(s);

  gpio_set(s->cs, 1);
}

void stm32_hal_spi_wait_done(stm32_hal_spi_t *s)
{
  _stm32_hal_spi_wait_done(s);
}

void stm32_hal_spi_write_buf(stm32_hal_spi_t *s, void *data, unsigned int len)
{
  unsigned char *cdata = (unsigned char *)data;
  unsigned int i;

  gpio_set(s->cs, 0);

  for (i = 0; i < len; i++) {
    _stm32_hal_spi_write(s, *cdata++);
    _stm32_hal_spi_wait_done(s);
  }

  gpio_set(s->cs, 1);
}

void stm32_hal_spi_wrd_buf(stm32_hal_spi_t *s, void *wdata, unsigned int wlen,
                           void *rdata, unsigned int rlen)
{
  unsigned char *cdata = (unsigned char *)wdata;
  unsigned int i;

  gpio_set(s->cs, 0);

  for (i = 0; i < wlen; i++) {
    _stm32_hal_spi_write(s, *cdata++);
    _stm32_hal_spi_wait_done(s);
  }

  cdata = (unsigned char *)rdata;
  for (i = 0; i < rlen; i++)
    *cdata++ = _stm32_hal_spi_read(s) & 0xff;

  gpio_set(s->cs, 1);
}
