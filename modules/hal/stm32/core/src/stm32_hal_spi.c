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

#include "common.h"
#include "stm32_hal_spi.h"

typedef struct {
  reg32_t cr1;
  reg32_t cr2;
  reg32_t sr;
  reg32_t dr;
  reg32_t crcpr;
  reg32_t rxcrcr;
  reg32_t txcrcr;
  reg32_t i2scfgr;
  reg32_t i2spr;
} stm32_spi_t;

#define STM32_SPI_CR1_BIDIMODE BIT(15)
#define STM32_SPI_CR1_BIDIOE BIT(14)
#define STM32_SPI_CR1_DFF_16BIT BIT(11)
#define STM32_SPI_CR1_RXONLY BIT(10)
#define STM32_SPI_CR1_SSM BIT(9)
#define STM32_SPI_CR1_SSI BIT(8)
#define STM32_SPI_CR1_SPE BIT(6)
#define STM32_SPI_CR1_BR(v) (((v) & 0x7) << 3) /* DIV 2^v */
#define STM32_SPI_CR1_MSTR BIT(2)
#define STM32_SPI_CR1_CPOL BIT(1)
#define STM32_SPI_CR1_CPHA BIT(0)

#define STM32_SPI_SR_BSY BIT(7)
#define STM32_SPI_SR_TXE BIT(1)
#define STM32_SPI_SR_RXNE BIT(0)

void stm32_hal_spi_init(stm32_hal_spi_t *s)
{
  stm32_spi_t *spi = s->base;

  spi->cr1 &= ~STM32_SPI_CR1_SPE;
  spi->cr1 = STM32_SPI_CR1_SSM | STM32_SPI_CR1_SSI | \
             STM32_SPI_CR1_BR(3) | STM32_SPI_CR1_MSTR;
  spi->cr2 = ((((unsigned int)s->wordlen - 1) & 0xf) << 8);
  spi->cr1 |= STM32_SPI_CR1_SPE;

  gpio_set(s->cs, 1);
  gpio_init(s->cs, GPIO_OUTPUT);
}

static void _stm32_hal_spi_write(stm32_hal_spi_t *s, unsigned int data)
{
  stm32_spi_t *spi = s->base;

  /* discard any not read data */
  if (spi->sr & STM32_SPI_SR_RXNE)
    (void)spi->dr;

  while ((spi->sr & STM32_SPI_SR_TXE) == 0)
    ;

  spi->dr = data & 0xffff;
}

static void _stm32_hal_spi_wait_done(stm32_hal_spi_t *s)
{
  stm32_spi_t *spi = s->base;

  while (!(spi->sr & STM32_SPI_SR_RXNE))
    ;
}

static unsigned int _stm32_hal_spi_read(stm32_hal_spi_t *s)
{
  stm32_spi_t *spi = s->base;
  unsigned int dummy = 0xffff;

  if (spi->sr & STM32_SPI_SR_RXNE)
    (void)spi->dr;

  _stm32_hal_spi_write(s, dummy);
  _stm32_hal_spi_wait_done(s);

  return spi->dr & 0xffff;
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

void stm32_hal_spi_write_buf2(stm32_hal_spi_t *s, void *d1, unsigned int l1,
                              void *d2, unsigned int l2)
{
  unsigned char *cdata;
  unsigned int i;

  gpio_set(s->cs, 0);

  cdata = (unsigned char *)d1;
  for (i = 0; i < l1; i++) {
    _stm32_hal_spi_write(s, *cdata++);
    _stm32_hal_spi_wait_done(s);
  }

  cdata = (unsigned char *)d2;
  for (i = 0; i < l2; i++) {
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
