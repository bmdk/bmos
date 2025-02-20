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

#ifndef STM32_HAL_I2C_H
#define STM32_HAL_I2C_H

typedef struct {
  unsigned char presc;
  unsigned char scldel;
  unsigned char sdadel;
  unsigned char sclh;
  unsigned char scll;
} i2c_timing_t;

typedef struct {
  void *base;
  int irq;
  int irq_err;
  i2c_timing_t *timing;
  signed char dmanum;
  signed char dmachan;
  signed char dmadevid_tx;
  signed char dmadevid_rx;
  signed char dmairq;
} i2c_dev_t;

void i2c_init(i2c_dev_t *i2c);
int i2c_write_buf(i2c_dev_t *i2c, unsigned int addr,
                  const void *bufp, unsigned int buflen);

int i2c_read_buf(i2c_dev_t *i2c, unsigned int addr,
                 void *rbufp, unsigned int rbuflen);

int i2c_write_read_buf(i2c_dev_t *i2c, unsigned int addr,
                       void *wbufp, unsigned int wbuflen,
                       void *rbufp, unsigned int rbuflen);

#define I2C1_BASE 0x40005400
#define I2C2_BASE 0x40005800

#if STM32_G4XX
#define I2C3_BASE 0x40007800
#define I2C4_BASE 0x40008400
#elif STM32_U5XX
#define I2C3_BASE 0x46002800
#define I2C4_BASE 0x40008400
#else
#define I2C3_BASE 0x40005C00
#define I2C4_BASE 0x58001C00
#endif

#endif
