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

#include "stm32_hal_spi.h"
#include "common.h"

typedef struct {
  unsigned int cr1;
  unsigned int cr2;
  unsigned int sr;
  unsigned int dr;
  unsigned int crcpr;
  unsigned int rxcrcr;
  unsigned int txcrcr;
  unsigned int i2scfgr;
  unsigned int i2spr;
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
  volatile stm32_spi_t *spi = s->base;

  spi->cr1 &= ~STM32_SPI_CR1_SPE;
  spi->cr1 = STM32_SPI_CR1_SSM | STM32_SPI_CR1_SSI | \
             STM32_SPI_CR1_BR(3) | STM32_SPI_CR1_MSTR;
  spi->cr2 = ((((unsigned int)s->wordlen - 1) & 0xf) << 8);
  spi->cr1 |= STM32_SPI_CR1_SPE;
}

void stm32_hal_spi_write(stm32_hal_spi_t *s, unsigned int data)
{
  volatile stm32_spi_t *spi = s->base;

  while ((spi->sr & STM32_SPI_SR_TXE) == 0)
    asm volatile ("nop");

  spi->dr = data & 0xffff;

#if 0
  while ((spi->sr & STM32_SPI_SR_TXE) == 0)
    asm volatile ("nop");
#endif
}

void stm32_hal_spi_wait_done(stm32_hal_spi_t *s)
{
  volatile stm32_spi_t *spi = s->base;

  while ((spi->sr & STM32_SPI_SR_BSY))
    asm volatile ("nop");
}
