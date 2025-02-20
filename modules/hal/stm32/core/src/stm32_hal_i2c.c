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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "shell.h"
#include "io.h"
#include "hal_int.h"
#include "hal_time.h"
#include "hal_dma_if.h"

#include "stm32_hal_i2c.h"

typedef struct _stm32_i2c_t {
  reg32_t cr1;
  reg32_t cr2;
  reg32_t oar1;
  reg32_t oar2;
  reg32_t timingr;
  reg32_t timoutr;
  reg32_t isr;
  reg32_t icr;
  reg32_t pecr;
  reg32_t rxdr;
  reg32_t txdr;
} stm32_i2c_t;

#define I2C_CR1_PE BIT(0)
#define I2C_CR1_TXIE BIT(1)
#define I2C_CR1_RXIE BIT(2)
#define I2C_CR1_NACKIE BIT(4)
#define I2C_CR1_STOPIE BIT(5)
#define I2C_CR1_TCIE BIT(6)
#define I2C_CR1_ERRIE BIT(7)
#define I2C_CR1_ANFOFF BIT(12)
#define I2C_CR1_TXDMAEN BIT(14)
#define I2C_CR1_RXDMAEN BIT(15)
#define I2C_CR1_SBC BIT(16)
#define I2C_CR1_NOSTRETCH BIT(17)

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
#define I2C_ISR_ADDRCF BIT(3)
#define I2C_ISR_RXNE BIT(2)
#define I2C_ISR_TXIS BIT(1)
#define I2C_ISR_TXE BIT(0)

#define I2C_ICR_MSK (I2C_ISR_ADDRCF | I2C_ISR_NACKF | I2C_ISR_STOPF)

#define I2C_ISR_ERR_MSK (0x00003f00)

#define I2C_MAX_BYTES 255

#define I2C_DMA_IRQ 0

#if I2C_DMA_IRQ
static void i2c_dma_irq(void *data)
{
  i2c_dev_t *i2cdev = (i2c_dev_t *)data;

  dma_irq_ack(i2cdev->dmanum, i2cdev->dmachan);

#if 0
  debug_printf("I2C DMA ISR\n");
#endif
}
#endif

void i2c_dma_init(i2c_dev_t *i2cdev, void *buf, unsigned int len, int tx)
{
  stm32_i2c_t *i2c = (stm32_i2c_t *)i2cdev->base;
  dma_attr_t attr;
  int dmadevid;
  void *src, *dst;

  dma_en(i2cdev->dmanum, i2cdev->dmachan, 0);

  if (tx) {
    dmadevid = i2cdev->dmadevid_tx;
    src = buf;
    dst = (void *)&i2c->txdr;
  } else {
    dmadevid = i2cdev->dmadevid_rx;
    src = (void *)&i2c->rxdr;
    dst = buf;
  }

  dma_set_chan(i2cdev->dmanum, i2cdev->dmachan, dmadevid);

  attr.ssiz = DMA_SIZ_1;
  attr.dsiz = DMA_SIZ_1;
  attr.prio = 0;
  if (tx) {
    attr.dir = DMA_DIR_TO;
    attr.sinc = 1;
    attr.dinc = 0;
  } else {
    attr.dir = DMA_DIR_FROM;
    attr.sinc = 0;
    attr.dinc = 1;
  }
  attr.irq = 1;

  dma_trans(i2cdev->dmanum, i2cdev->dmachan, src, dst, len, attr);
  dma_en(i2cdev->dmanum, i2cdev->dmachan, 1);
}

typedef struct {
  unsigned char buf[I2C_MAX_BYTES];
  unsigned char read;
  unsigned char buflen;
  volatile unsigned char bufc;
  volatile signed char wait;
} i2c_irq_data_t;

static i2c_irq_data_t irq_data;

static void i2c_irq(void *p)
{
  stm32_i2c_t *i2c = (stm32_i2c_t *)p;
  unsigned int isr = i2c->isr;
  i2c_irq_data_t *id = &irq_data;

#if 0
  debug_printf("I2C ISR %x\n", isr);
#endif

  if (isr & I2C_ISR_NACKF) {
    if (!id->read)
      i2c->cr2 |= I2C_CR2_STOP;
    id->wait = -1;
#if 0
    debug_printf("NAK\n");
#endif
  }

#if 0
  if (isr & I2C_ISR_STOPF) {
    debug_printf("STP\n");
    if (id->wait > 0)
      id->wait = 0;
  }
#endif

#if 1
  if (isr & I2C_ISR_TC) {
#if 0
    debug_printf("TC\n");
#endif
    i2c->cr2 |= I2C_CR2_STOP;
    if (id->wait > 0)
      id->wait = 0;
  }
#endif

#if 0
  if (isr & I2C_ISR_RXNE) {
    unsigned char ch = i2c->rxdr;
#if 0
    debug_printf("RX %d %02x\n", id->bufc, ch);
#endif
    if (id->bufc < id->buflen)
      id->buf[id->bufc++] = ch;
    if (id->bufc >= id->buflen && id->wait > 0)
      id->wait = 0;
  }
#endif

#if 0
  /* seems to be identical with TXE? */
  if (isr & I2C_ISR_TXIS)
    debug_printf("TXIS\n");

#endif

#if 0
  if (!id->read && (isr & I2C_ISR_TXE)) {
    int rem = id->buflen - id->bufc;

    if (rem > 0)
      i2c->txdr = id->buf[id->bufc++];
  }
#endif

  i2c->icr = (isr & I2C_ICR_MSK);
}

static void i2c_err_irq(void *p)
{
  stm32_i2c_t *i2c = (stm32_i2c_t *)p;
  unsigned int isr = i2c->isr & I2C_ISR_ERR_MSK;

  debug_printf("i2c err %x\n", isr);

  i2c->icr = isr;
}

#if 0
static void i2c_restart(stm32_i2c_t *i2c)
{
  i2c->cr1 &= ~I2C_CR1_PE;

  (volatile void)i2c->cr1;

  i2c->cr1 |= I2C_CR1_PE;
}
#endif

void i2c_init(i2c_dev_t *i2cdev)
{
  stm32_i2c_t *i2c = (stm32_i2c_t *)i2cdev->base;
  i2c_timing_t *t = &i2cdev->timing[0];

  i2c->cr1 &= ~I2C_CR1_PE;

  i2c->timingr = I2C_TIMINGR_PRESC(t->presc) | \
                 I2C_TIMINGR_SCLDEL(t->scldel) | \
                 I2C_TIMINGR_SDADEL(t->sdadel) | \
                 I2C_TIMINGR_SCLH(t->sclh) | I2C_TIMINGR_SCLL(t->scll);

  if (i2cdev->irq >= 0) {
    int irq_err = i2cdev->irq_err;

    irq_register("i2c", i2c_irq, i2c, i2cdev->irq);

    if (irq_err < 0)
      irq_err = i2cdev->irq + 1;

    irq_register("i2c_err", i2c_err_irq, i2c, irq_err);

    i2c->cr1 |= I2C_CR1_ERRIE;
    i2c->cr1 |= (I2C_CR1_NACKIE | I2C_CR1_TCIE);
    i2c->cr1 |= I2C_CR1_STOPIE;
#if 0
    i2c->cr1 |= I2C_CR1_RXIE;
#endif
#if 0
    i2c->cr1 |= I2C_CR1_TXIE;
#endif
#if I2C_DMA_IRQ
    irq_register("i2c_dma", i2c_dma_irq, i2cdev, i2cdev->dmairq);
#endif
  }

  i2c->cr1 |= I2C_CR1_PE;
}

static int _i2c_write_buf_irq(i2c_dev_t *i2cdev, unsigned int addr,
                              const void *bufp, unsigned int buflen)
{
  stm32_i2c_t *i2c = (stm32_i2c_t *)i2cdev->base;
  i2c_irq_data_t *id = &irq_data;

#if 0
  debug_printf("_i2c_write_buf_irq %02x %d\n", addr, buflen);
#endif

#if 0
  while (i2c->isr & I2C_ISR_BUSY)
    ;
#endif

#if 0
  debug_printf("_i2c_write_buf_irq\n");
#endif

  if (buflen > I2C_MAX_BYTES)
    return -1;

  memcpy(id->buf, bufp, buflen);
  id->buflen = buflen;
  id->bufc = 0;
  id->wait = 1;
  id->read = 0;

  i2c_dma_init(i2cdev, id->buf, buflen, 1);
  i2c->cr1 &= ~I2C_CR1_RXDMAEN;
  i2c->cr1 |= I2C_CR1_TXDMAEN;

#if 1
  /* flush tx buffer */
#if 0
  while ((i2c->isr & I2C_ISR_TXE) == 0)
#endif
  i2c->isr = I2C_ISR_TXE | I2C_ISR_TXIS;
#endif
  i2c->cr2 = /* I2C_CR2_AUTOEND | */ I2C_CR2_NBYTES(buflen) | I2C_CR2_SADD(
    addr << 1);
  i2c->cr2 |= I2C_CR2_START;

  while (id->wait > 0)
    ;

  if (id->wait < 0)
    return -1;

  return 0;
}

static int _i2c_read_buf_irq(i2c_dev_t *i2cdev, unsigned int addr,
                             void *bufp, unsigned int buflen)
{
  stm32_i2c_t *i2c = (stm32_i2c_t *)i2cdev->base;
  i2c_irq_data_t *id = &irq_data;

#if 0
  debug_printf("_i2c_read_buf_irq %02x %d %p\n", addr, buflen, &id->buf);
#endif

  if (buflen == 0)
    return -1;

  i2c_dma_init(i2cdev, id->buf, buflen, 0);
  i2c->cr1 &= ~I2C_CR1_TXDMAEN;
  i2c->cr1 |= I2C_CR1_RXDMAEN;

  i2c->cr2 = /* I2C_CR2_AUTOEND | */ I2C_CR2_NBYTES(buflen) | I2C_CR2_RD_WRN |
             I2C_CR2_SADD(addr << 1);

  id->buflen = buflen;
  id->bufc = 0;
  id->wait = 1;
  id->read = 1;

  i2c->cr2 |= I2C_CR2_START;

  while (id->wait > 0)
    ;

#if 0
  if (id->bufc < buflen)
    return -1;
#endif

  memcpy(bufp, id->buf, buflen);

  return 0;
}

static int _i2c_write_buf(stm32_i2c_t *i2c, unsigned int addr,
                          const void *bufp, unsigned int buflen)
{
  const char *buf = bufp;

  /* flush tx buffer */
  i2c->isr = I2C_ISR_TXE;

  i2c->cr2 = I2C_CR2_AUTOEND | I2C_CR2_NBYTES(buflen) | I2C_CR2_SADD(addr << 1);
  i2c->cr2 |= I2C_CR2_START;

  i2c->icr = I2C_ISR_STOPF | I2C_ISR_NACKF;

  while (buflen) {
    unsigned int timeout = 1000000;

    for (;;) {
      unsigned int isr = i2c->isr;

      if (--timeout == 0) {
        xprintf("W TO isr %08x\n", isr);
        return -1;
      }

      if (isr & I2C_ISR_TXIS)
        break;

      if (isr & I2C_ISR_BUSY)
        continue;

      if (isr & I2C_ISR_NACKF)
        return -1;
    }

    i2c->txdr = *buf++;
    buflen--;
  }

  return 0;
}

static int _i2c_read_buf(stm32_i2c_t *i2c, unsigned int addr,
                         void *bufp, unsigned int buflen)
{
  char *buf = bufp;

  i2c->cr2 = I2C_CR2_AUTOEND | I2C_CR2_NBYTES(buflen) | I2C_CR2_RD_WRN |
             I2C_CR2_SADD(addr << 1);
  i2c->cr2 |= I2C_CR2_START;

  i2c->icr = I2C_ISR_STOPF | I2C_ISR_NACKF;

  while (buflen) {
    unsigned int timeout = 1000000;

    for (;;) {
      unsigned int isr = i2c->isr;

      if (--timeout == 0) {
        xprintf("R TO isr %08x\n", isr);
        return -1;
      }

      if (isr & I2C_ISR_RXNE)
        break;

      if (isr & I2C_ISR_BUSY)
        continue;

      if (isr & I2C_ISR_NACKF)
        return -1;
    }

    *buf++ = i2c->rxdr;
    buflen--;
  }

  return 0;
}

int i2c_write_read_buf(i2c_dev_t *i2cdev, unsigned int addr,
                       void *wbufp, unsigned int wbuflen,
                       void *rbufp, unsigned int rbuflen)
{
  stm32_i2c_t *i2c = (stm32_i2c_t *)i2cdev->base;

  if (i2cdev->irq >= 0) {
    if (_i2c_write_buf_irq(i2cdev, addr, wbufp, wbuflen) < 0)
      return -1;

    if (_i2c_read_buf_irq(i2cdev, addr, rbufp, rbuflen) < 0)
      return -1;
  } else {
    if (_i2c_write_buf(i2c, addr, wbufp, wbuflen) < 0)
      return -1;

    if (_i2c_read_buf(i2c, addr, rbufp, rbuflen) < 0)
      return -1;
  }

  return 0;
}

int i2c_read_buf(i2c_dev_t *i2cdev, unsigned int addr,
                 void *rbufp, unsigned int rbuflen)
{
  stm32_i2c_t *i2c = (stm32_i2c_t *)i2cdev->base;

  if (i2cdev->irq >= 0) {
    if (_i2c_read_buf_irq(i2cdev, addr, rbufp, rbuflen) < 0)
      return -1;
  } else if (_i2c_read_buf(i2c, addr, rbufp, rbuflen) < 0)
    return -1;

  return 0;
}

int i2c_write_buf(i2c_dev_t *i2cdev, unsigned int addr,
                  const void *bufp, unsigned int buflen)
{
  stm32_i2c_t *i2c = (stm32_i2c_t *)i2cdev->base;

  if (i2cdev->irq >= 0)
    return _i2c_write_buf_irq(i2cdev, addr, bufp, buflen);
  else
    return _i2c_write_buf(i2c, addr, bufp, buflen);
}
