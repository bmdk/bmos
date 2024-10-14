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

#include <avr/io.h>

#include "common.h"
#include "hal_avr_common.h"
#include "hal_avr_spi.h"
#include "hal_gpio.h"
#include "io.h"

#define SPCR_SPIE BIT(7)
#define SPCR_SPE BIT(6)
#define SPCR_DORD BIT(5)
#define SPCR_MSTR BIT(4)
#define SPCR_CPOL BIT(3)
#define SPCR_CPHA BIT(2)
#define SPCR_SPR1 BIT(1)
#define SPCR_SPR0 BIT(0)

#define SPSR_SPIF BIT(7)
#define SPSR_WCOL BIT(6)
#define SPSR_SPI2X BIT(0)

void spi_init(gpio_handle_t cs)
{
  /* f/4 = 2 MHz */
  SPSR = 0;
  SPCR = SPCR_CPOL | SPCR_CPHA | SPCR_MSTR | SPCR_SPE;

  gpio_set(cs, 1);
  gpio_init(cs, GPIO_OUTPUT);
}

void spi_info()
{
  xprintf("SPCR %p %02x\n", &SPCR, SPCR);
  xprintf("SPSR %p %02x\n", &SPSR, SPSR);
}

int spi_write_read_buf(gpio_handle_t cs,
                       void *wbufp, void *rbufp, unsigned int len)
{
  unsigned char *wbuf, *rbuf;
  int i;

  gpio_set(cs, 0);

  wbuf = wbufp;
  rbuf = rbufp;

  for (i = 0; i < len; i++) {
    while((SPSR & SPSR_SPIF) != 0)
      ;

    SPDR = *wbuf++;

    while((SPSR & SPSR_SPIF) == 0)
      ;

    *rbuf++ = SPDR;
  }

  gpio_set(cs, 1);

  return 0;
}
