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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "shell.h"
#include "io.h"
#include "hal_time.h"

#include "stm32_hal_i2c.h"

struct _stm32_i2c_t {
  reg32_t cr1;
  reg32_t cr2;
  reg32_t oar1;
  reg32_t oar2;
  reg32_t dr;
  reg32_t sr1;
  reg32_t sr2;
  reg32_t ccr;
  reg32_t trise;
  reg32_t fltr;
};

#define I2C_CR1_PE BIT(0)
#define I2C_CR1_START BIT(8)
#define I2C_CR1_STOP BIT(9)
#define I2C_CR1_ACK BIT(10)
#define I2C_CR1_SWRST BIT(15)

#define I2C_CR2_FREQ(_v_) ((_v_) & 0x3f)

#define I2C_CCR_CCR(_v_) ((_v_) & 0x7ff)
#define I2C_CCR_DUTY BIT(14)
#define I2C_CCR_FS BIT(15)

#define I2C_SR1_START BIT(0)
#define I2C_SR1_ADDR BIT(1)
#define I2C_SR1_RXNE BIT(6)
#define I2C_SR1_TXE BIT(7)

#define FREQ 48      /* MHz */
#define I2C_FREQ 100 /* kHz */

void i2c_init(stm32_i2c_t *i2c)
{
  unsigned int div;

  i2c->cr1 &= ~I2C_CR1_PE;

  div = (FREQ * 1000) / 2 / I2C_FREQ;

  i2c->ccr = I2C_CCR_CCR(div);
  i2c->cr2 = I2C_CR2_FREQ(FREQ);
  i2c->trise = FREQ + 1;

  i2c->cr1 |= I2C_CR1_PE;
}

#define TIMEOUT_US 100000

static int _i2c_write_buf(stm32_i2c_t *i2c, unsigned int addr,
                          const void *bufp, unsigned int buflen)
{
  const char *buf = bufp;
  int sr1, sr2;
  unsigned int start;

  start = hal_time_us();

  i2c->cr1 |= I2C_CR1_START;
  for (;;) {
    sr1 = i2c->sr1;
    if (sr1 & I2C_SR1_START)
      break;
    if (hal_time_us() - start > TIMEOUT_US) {
      xprintf("S TO sr1 %08x\n", sr1);
      return -1;
    }
  }

  i2c->dr = ((addr & 0x7f) << 1) | 0;

  start = hal_time_us();
  for (;;) {
    sr1 = i2c->sr1;
    if (sr1 & I2C_SR1_ADDR)
      break;
    if (hal_time_us() - start > TIMEOUT_US) {
#if 0
      xprintf("A TO sr1 %08x\n", sr1);
#endif
      return -1;
    }
  }
  sr2 = i2c->sr2;
  if (0)
    xprintf("sr1 %08x sr2 %08x\n", sr1, sr2);

  while (buflen) {
    start = hal_time_us();
    i2c->dr = *buf++;

    for (;;) {
      sr1 = i2c->sr1;

      if (sr1 & I2C_SR1_TXE)
        break;

      if (hal_time_us() - start > TIMEOUT_US) {
        xprintf("W TO sr1 %08x\n", sr1);
        return -1;
      }
    }

    buflen--;
  }

  i2c->cr1 |= I2C_CR1_STOP;

  return 0;
}

static int _i2c_read_buf(stm32_i2c_t *i2c, unsigned int addr,
                         void *bufp, unsigned int buflen)
{
  char *buf = bufp;
  int sr1, sr2;
  unsigned int start;

  start = hal_time_us();
  i2c->cr1 |= I2C_CR1_START;
  for (;;) {
    sr1 = i2c->sr1;
    if (sr1 & I2C_SR1_START)
      break;
    if (hal_time_us() - start > TIMEOUT_US) {
      xprintf("S TO sr1 %08x\n", sr1);
      return -1;
    }
  }

  i2c->dr = ((addr & 0x7f) << 1) | 1;

  start = hal_time_us();
  for (;;) {
    sr1 = i2c->sr1;
    if (sr1 & I2C_SR1_ADDR)
      break;
    if (hal_time_us() - start > TIMEOUT_US) {
      xprintf("A TO sr1 %08x\n", sr1);
      return -1;
    }
  }

  if (buflen <= 1)
    i2c->cr1 &= ~I2C_CR1_ACK;
  else
    i2c->cr1 |= I2C_CR1_ACK;

  sr2 = i2c->sr2;
  if (0)
    xprintf("sr1 %08x sr2 %08x\n", sr1, sr2);

  while (buflen) {
    start = hal_time_us();

    for (;;) {
      sr1 = i2c->sr1;

      if (sr1 & I2C_SR1_RXNE)
        break;

      if (hal_time_us() - start > TIMEOUT_US) {
        xprintf("R TO sr1 %08x\n", sr1);
        return -1;
      }
    }

    *buf++ = i2c->dr;

    buflen--;

    if (buflen == 1)
      i2c->cr1 &= ~I2C_CR1_ACK;
    else
      i2c->cr1 |= I2C_CR1_ACK;
  }

  i2c->cr1 |= I2C_CR1_STOP;

  return 0;
}


int i2c_write_read_buf(stm32_i2c_t *i2c, unsigned int addr,
                       void *wbufp, unsigned int wbuflen,
                       void *rbufp, unsigned int rbuflen)
{
  if (_i2c_write_buf(i2c, addr, wbufp, wbuflen) < 0)
    return -1;

  if (_i2c_read_buf(i2c, addr, rbufp, rbuflen) < 0)
    return -1;

  return 0;
}

int i2c_write_buf(stm32_i2c_t *i2c, unsigned int addr,
                  const void *bufp, unsigned int buflen)
{
  return _i2c_write_buf(i2c, addr, bufp, buflen);
}
