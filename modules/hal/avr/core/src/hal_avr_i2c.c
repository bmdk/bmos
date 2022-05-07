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
#include "hal_avr_i2c.h"
#include "io.h"

#define TWCR_TINT BIT(7)
#define TWCR_TWEA BIT(6)
#define TWCR_STA BIT(5)
#define TWCR_STO BIT(4)
#define TWCR_TWEN BIT(2)
#define TWCR_TWIE BIT(0)

#define TWSR_START  0x08
#define TWSR_RSTART 0x10
#define TWSR_ARB_LOST 0x38

#define TWSR_WADDR_ACK 0x18
#define TWSR_WADDR_NACK 0x20
#define TWSR_WDATA_ACK 0x28
#define TWSR_WDATA_NACK 0x30

#define TWSR_RADDR_ACK 0x40
#define TWSR_RADDR_NACK 0x48
#define TWSR_RDATA_ACK 0x50
#define TWSR_RDATA_NACK 0x58

static unsigned char _get_status()
{
  return TWSR & 0xf8;
}

/* i2c frequency = CLOCK/(16 + 2 * TWBR * 4 ^ TWPS */
#define I2C_FREQ 100000 /* Hz */
#define TWPS_VAL 0
#define TWPS_4EXP 1     /* 4^TWPS_VAL */
#define I2C_BR ((CLOCK / I2C_FREQ - 16) / 2 / TWPS_4EXP)

void i2c_init()
{
  TWBR = I2C_BR;
  TWSR = (TWPS_VAL & 0x3);
}

void i2c_info()
{
  xprintf("TWBR %p %02x\n", &TWBR, TWBR);
  xprintf("TWSR %p %02x\n", &TWSR, TWSR);
  xprintf("TWDR %p %02x\n", &TWDR, TWDR);
  xprintf("TWCR %p %02x\n", &TWCR, TWCR);
}

static void _i2c_write_byte(unsigned char val)
{
  TWDR = val;
  TWCR = TWCR_TINT | TWCR_TWEN;

  while (!(TWCR & TWCR_TINT))
    ;
}

static int _i2c_start()
{
  unsigned char status;

  TWCR = TWCR_TWEN;
  TWCR = TWCR_TINT | TWCR_STA | TWCR_TWEN;

  while (!(TWCR & TWCR_TINT))
    ;

  status = _get_status();
  if (!(status == TWSR_START || status == TWSR_RSTART))
    return -1;

  return 0;
}

static void _i2c_stop()
{
  TWCR = TWCR_TINT | TWCR_STO | TWCR_TWEN;
  TWCR = 0;
}

static int _i2c_write_buf(unsigned char addr, void *buf, unsigned int len)
{
  char *bufc = (char *)buf;
  unsigned int i;
  unsigned char status;

  if (_i2c_start() < 0)
    return -1;

  addr <<= 1;
  _i2c_write_byte(addr);
  status = _get_status();
  if (status != TWSR_WADDR_ACK)
    return -1;

  for (i = 0; i < len; i++) {
    _i2c_write_byte(*bufc++);
    status = _get_status();
    if (status != TWSR_WDATA_ACK)
      return -1;
  }

  return 0;
}

int _i2c_read_buf(unsigned char addr, void *buf, unsigned int len)
{
  char *bufc = (char *)buf;
  unsigned int i, err = 0;
  unsigned char status;

  if (_i2c_start() < 0)
    return -1;

  addr <<= 1;
  addr |= 1;
  _i2c_write_byte(addr);
  status = _get_status();
  if (status != TWSR_RADDR_ACK)
    return -1;

  for (i = 0; i < len; i++) {
    unsigned int flags = TWCR_TINT | TWCR_TWEN;

    if (i != len - 1)
      flags |= TWCR_TWEA;

    TWCR = flags;

    while (!(TWCR & TWCR_TINT))
      ;

    status = _get_status();
    if (!(status == TWSR_RDATA_ACK || status == TWSR_RDATA_NACK)) {
      err = -1;
      break;
    }

    *bufc++ = TWDR;
  }

  return err;
}

int i2c_write_buf(unsigned char addr, void *buf, unsigned int len)
{
  int err = 0;

  err = _i2c_write_buf(addr, buf, len);

  _i2c_stop();

  return err;
}

int i2c_read_buf(unsigned char addr, void *buf, unsigned int len)
{
  int err = 0;

  err = _i2c_read_buf(addr, buf, len);

  _i2c_stop();

  return err;
}

int i2c_write_read_buf(unsigned char addr,
                       void *wbufp, unsigned int wbuflen,
                       void *rbufp, unsigned int rbuflen)
{
  int err;

  err = _i2c_write_buf(addr, wbufp, wbuflen);
  if (err < 0)
    goto exit;

  err = _i2c_read_buf(addr, rbufp, rbuflen);

exit:
  _i2c_stop();

  return err;
}
