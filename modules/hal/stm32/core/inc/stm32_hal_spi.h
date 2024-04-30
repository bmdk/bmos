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

#ifndef STM32_HAL_SPI
#define STM32_HAL_SPI

#include <hal_gpio.h>
#include <common.h>

/* Clock Polarity Initially High */
#define STM32_SPI_FLAG_CPOL BIT(0)
/* Clock Phase - Data ready on second clock transition */
#define STM32_SPI_FLAG_CPHA BIT(1)

typedef struct {
  void *base;
  unsigned char wordlen;
  unsigned char div;
  unsigned char flags;
  gpio_handle_t cs;
} stm32_hal_spi_t;

void stm32_hal_spi_init(stm32_hal_spi_t *spi);
void stm32_hal_spi_write(stm32_hal_spi_t *spi, unsigned int data);
void stm32_hal_spi_wait_done(stm32_hal_spi_t *spi);
void stm32_hal_spi_write_buf(stm32_hal_spi_t *s, void *data, unsigned int len);
void stm32_hal_spi_write_buf2(stm32_hal_spi_t *s, void *d1, unsigned int l1,
                              void *d2, unsigned int l2);
void stm32_hal_spi_wrd_buf(stm32_hal_spi_t *s, void *wdata, unsigned int wlen,
                           void *rdata, unsigned int rlen);

/* seems to be standard on most STM32 */
#define SPI1_BASE 0x40013000
#define SPI2_BASE 0x40003800
#define SPI3_BASE 0x40003C00
#define SPI4_BASE 0x40013400
#define SPI5_BASE 0x40015000
#define SPI6_BASE 0x58001400

#endif
