/* Copyright (c) 2019 Brian Thomas Murphy
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
#include "common.h"
#include "shell.h"
#include "io.h"
#include "hal_time.h"

#include "stm32_hal_i2c.h"

struct _stm32_i2c_t {
  unsigned int cr1;
  unsigned int cr2;
  unsigned int oar1;
  unsigned int oar2;
  unsigned int timingr;
  unsigned int timoutr;
  unsigned int isr;
  unsigned int icr;
  unsigned int pecr;
  unsigned int rxdr;
  unsigned int txdr;
};

#define I2C_CR1_PE BIT(0)

#define I2C_CR2_AUTOEND BIT(25)
#define I2C_CR2_RELOAD BIT(24)
#define I2C_CR2_NBYTES(n) (((n) & 0xff) << 16)
#define I2C_CR2_NACK BIT(15)
#define I2C_CR2_STOP BIT(14)
#define I2C_CR2_START BIT(13)
#define I2C_CR2_HEAD10R BIT(12)
#define I2C_CR2_ADD10 BIT(11)
#define I2C_CR2_RD_WRN BIT(10)
#define I2C_CR2_SADD(n) ((n) & 0x2ff)

#define I2C_TIMINGR_PRESC(n) ((0xfUL & (unsigned int)(n)) << 28)
#define I2C_TIMINGR_SCLDEL(n) ((0xfUL & (n)) << 20)
#define I2C_TIMINGR_SDADEL(n) ((0xfUL & (n)) << 16)
#define I2C_TIMINGR_SCLH(n) ((0xffUL & (n)) << 8)
#define I2C_TIMINGR_SCLL(n) ((0xffUL & (n)) << 0)

#define I2C_ISR_BUSY BIT(15)
#define I2C_ISR_TC BIT(6)
#define I2C_ISR_STOPF BIT(5)
#define I2C_ISR_NACKF BIT(4)
#define I2C_ISR_RXNE BIT(2)
#define I2C_ISR_TXIS BIT(1)
#define I2C_ISR_TXE BIT(0)

void i2c_init(volatile stm32_i2c_t *i2c)
{
  I2C->cr1 &= ~I2C_CR1_PE;

  I2C->timingr = I2C_TIMINGR_PRESC(0x1) | \
                 I2C_TIMINGR_SCLDEL(0xc) | I2C_TIMINGR_SDADEL(0x0) | \
                 I2C_TIMINGR_SCLH(0xec) | I2C_TIMINGR_SCLL(0xff);

  I2C->cr1 |= I2C_CR1_PE;
}

int i2c_write_buf(volatile stm32_i2c_t *i2c, unsigned int addr,
                  const void *bufp, unsigned int buflen)
{
  const char *buf = bufp;

  i2c->cr2 = I2C_CR2_AUTOEND | I2C_CR2_NBYTES(buflen) | I2C_CR2_SADD(addr << 1);
  i2c->cr2 |= I2C_CR2_START;

  i2c->icr |= I2C_ISR_STOPF | I2C_ISR_NACKF;

  while (buflen) {
    unsigned int timeout = 1000000;

    for (;;) {
      unsigned int isr = i2c->isr;

      if (--timeout == 0) {
        xprintf("TO isr %08x\n", isr);
        return -1;
      }

      if (isr & I2C_ISR_TXIS)
        break;

      if (isr & I2C_ISR_BUSY)
        continue;

      if (isr & I2C_ISR_NACKF) {
#if 0
        xprintf("NACK add %d isr %08x\n", addr, isr);
#endif
        return -1;
      }
    }

    i2c->txdr = *buf++;
    buflen--;
  }

  return 0;
}

static int i2c_cmd(int argc, char *argv[])
{
  char buf[16];
  unsigned int addr;
  int err, len, i;

  switch (argv[1][0]) {
  case 'i':
    xprintf("init\n");
    i2c_init(I2C);
    break;
  case 'w':
    if (argc < 4)
      return -1;
    addr = strtoul(argv[2], 0, 0);
    len = argc - 3;
    if (len > sizeof(buf))
      len = sizeof(buf);

    for (i = 0; i < len; i++)
      buf[i] = strtoul(argv[3 + i], 0, 0);

    err = i2c_write_buf(I2C, addr, buf, len);
    if (err != 0)
      xprintf("error %d\n", err);
    break;
  case 'p':
    buf[0] = 0xff;
    for (addr = 8; addr < 0x78; addr++) {
      i2c_init(I2C);
      hal_delay_us(1000);
      err = i2c_write_buf(I2C, addr, buf, 1);
      if (err == 0)
        xprintf("addr %02x\n", addr);
    }
    break;
  }

  return 0;
}

SHELL_CMD(i2c, i2c_cmd);
