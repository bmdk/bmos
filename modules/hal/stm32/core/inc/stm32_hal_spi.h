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

typedef struct {
  void *base;
  unsigned char wordlen;
  unsigned char div;
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

#endif
