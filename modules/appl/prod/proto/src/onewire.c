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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "bmos_syspool.h"
#include "bmos_task.h"
#include "common.h"
#include "hal_gpio.h"
#include "hal_int_cpu.h"
#include "hal_time.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "xassert.h"
#include "xslog.h"

#include "onewire.h"

static const unsigned char ow_crc_table[256] = {
  0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
  0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
  0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
  0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
  0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,
  0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
  0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,
  0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
  0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
  0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
  0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,
  0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
  0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,
  0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
  0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
  0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
  0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,
  0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
  0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,
  0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
  0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,
  0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
  0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,
  0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
  0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,
  0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
  0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,
  0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
  0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,
  0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
  0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,
  0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35
};

static unsigned int ow_crc_update(unsigned int crc, const void *data,
                                  unsigned int data_len)
{
  const unsigned char *d = (const unsigned char *)data;
  unsigned int tbl_idx;

  while (data_len--) {
    tbl_idx = crc ^ *d;
    crc = (ow_crc_table[tbl_idx] ^ (crc >> 8)) & 0xff;
    d++;
  }
  return crc & 0xff;
}

typedef void ow_write_bit_fun_t(void *data, int bit);
typedef int ow_read_bit_fun_t(void *data);
typedef int ow_reset_fun_t(void *data);

typedef struct {
  ow_write_bit_fun_t *write_bit;
  ow_read_bit_fun_t *read_bit;
  ow_reset_fun_t *reset;
} one_wire_ops_t;

#define DEVLIST_LEN 8

typedef struct {
  one_wire_ops_t *ops;
  void *ops_data;
  unsigned char devlist[DEVLIST_LEN][OW_DEVID_LEN];
  ow_found_cb_t *found;
  ow_temp_cb_t *temp;
  unsigned int poll_interval;
} one_wire_data_t;

typedef struct {
  bmos_queue_t *rxq;
  bmos_queue_t *txq;
  uart_t *uart;
} one_wire_uart_data_t;

#define OW_UART 1
#define OW_GPIO 1

#if OW_UART
static void ow_uart_init(one_wire_uart_data_t *d, uart_t *uart)
{
  d->uart = uart;
  d->rxq = queue_create("owrx", QUEUE_TYPE_TASK);
  d->txq = uart_open(d->uart, 9600, d->rxq, 0);
}

static int _ow_uart_read(one_wire_uart_data_t *d)
{
  static bmos_op_msg_t *m;
  unsigned char *data;
  unsigned int val;

  m = op_msg_wait_ms(d->rxq, 1000);
  if (!m) {
    xslog(LOG_ERR, "uart read timeout");
    return -1;
  }

  data = BMOS_OP_MSG_GET_DATA(m);

  if (m->len == 0)
    val = -1;
  else
    val = (int)data[0];

  op_msg_return(m);

  return val;
}

static void _ow_uart_write(one_wire_uart_data_t *d, int v)
{
  unsigned char *p;
  static bmos_op_msg_t *m;

  m = op_msg_wait(syspool);
  XASSERT(m);

  p = BMOS_OP_MSG_GET_DATA(m);

  p[0] = (unsigned char)v;

  op_msg_put(d->txq, m, 0, 1);
}

static int ow_wrrd_bit(one_wire_uart_data_t *d, int bit)
{
  int ch, v;

  if (bit)
    ch = 0xff;
  else
    ch = 0x00;

  _ow_uart_write(d, ch);

  v = _ow_uart_read(d);

  if (v < 0)
    xslog(LOG_ERR, "uart read error");

  return (v == 0xff) ? 1 : 0;
}

static void ow_write_bit_uart(void *data, int bit)
{
  one_wire_uart_data_t *d = (one_wire_uart_data_t *)data;

  (void)ow_wrrd_bit(d, bit);
}

static int ow_read_bit_uart(void* data)
{
  one_wire_uart_data_t *d = (one_wire_uart_data_t *)data;

  return ow_wrrd_bit(d, 1);
}

static int ow_reset_uart(void *data)
{
  one_wire_uart_data_t *d = (one_wire_uart_data_t *)data;
  int rdata;
  unsigned char *p;
  bmos_op_msg_t *m;

  uart_set_baud(d->uart, 9600);

  m = op_msg_wait(syspool);
  XASSERT(m);

  p = BMOS_OP_MSG_GET_DATA(m);

  p[0] = 0xf0;

  op_msg_put(d->txq, m, 0, 1);

  task_delay(50);

  rdata = _ow_uart_read(d);

  uart_set_baud(d->uart, 115200);

  if (rdata < 0 || rdata == 0xf0)
    return -1;

  return 0;
}

static one_wire_ops_t ow_uart_ops = {
  ow_write_bit_uart,
  ow_read_bit_uart,
  ow_reset_uart
};
#endif

#if OW_GPIO
typedef struct {
  gpio_handle_t gpio;
} one_wire_gpio_data_t;

/* From Maxim application note 126
   1-Wire Communication Through Software */
#define OW_BT_A 6
#define OW_BT_B 64
#define OW_BT_C 60
#define OW_BT_D 10
#define OW_BT_E 9
#define OW_BT_F 55
#define OW_BT_G 0
#define OW_BT_H 480
#define OW_BT_I 70
#define OW_BT_J 410

static void ow_gpio_init(one_wire_gpio_data_t *owd, gpio_handle_t gpio)
{
  owd->gpio = gpio;

  gpio_set(owd->gpio, 0);
  gpio_init(owd->gpio, GPIO_INPUT);
}

static void ow_gpio_1(one_wire_gpio_data_t *owd)
{
  gpio_init(owd->gpio, GPIO_INPUT);
}

static void ow_gpio_0(one_wire_gpio_data_t *owd)
{
  gpio_init(owd->gpio, GPIO_OUTPUT);
}

static int ow_gpio_read(one_wire_gpio_data_t *owd)
{
  return gpio_get(owd->gpio);
}

static int ow_reset_gpio(void *data)
{
  one_wire_gpio_data_t *d = (one_wire_gpio_data_t *)data;
  int result;

  ow_gpio_0(d);
  hal_delay_us(OW_BT_H);
  ow_gpio_1(d);
  hal_delay_us(OW_BT_I);
  result = ow_gpio_read(d);
  hal_delay_us(OW_BT_J);
  if (result == 1)
    return -1;
  return 0;
}

static void ow_write_bit_gpio(void *data, int bit)
{
  one_wire_gpio_data_t *d = (one_wire_gpio_data_t *)data;
  unsigned int saved = interrupt_disable();

  if (bit) {
    ow_gpio_0(d);
    hal_delay_us(OW_BT_A);
    ow_gpio_1(d);
    hal_delay_us(OW_BT_B);
  } else {
    ow_gpio_0(d);
    hal_delay_us(OW_BT_C);
    ow_gpio_1(d);
    hal_delay_us(OW_BT_D);
  }

  interrupt_enable(saved);
}

static int ow_read_bit_gpio(void *data)
{
  one_wire_gpio_data_t *d = (one_wire_gpio_data_t *)data;
  int result;
  unsigned int saved = interrupt_disable();

  ow_gpio_0(d);
  hal_delay_us(OW_BT_A);
  ow_gpio_1(d);
  hal_delay_us(OW_BT_E);
  result = ow_gpio_read(d);
  hal_delay_us(OW_BT_F);

  interrupt_enable(saved);

  return result;
}

static one_wire_ops_t ow_gpio_ops = {
  ow_write_bit_gpio,
  ow_read_bit_gpio,
  ow_reset_gpio
};
#endif

static void ow_write_bit(one_wire_data_t *owd, int bit)
{
  owd->ops->write_bit(owd->ops_data, bit);
}

static int ow_read_bit(one_wire_data_t *owd)
{
  return owd->ops->read_bit(owd->ops_data);
}

static int ow_reset(one_wire_data_t *owd)
{
  return owd->ops->reset(owd->ops_data);
}

static void ow_write_byte(one_wire_data_t *owd, unsigned int byte)
{
  unsigned int i;

  for (i = 0; i < 8; i++) {
    ow_write_bit(owd, byte & 1);
    byte >>= 1;
  }
}

static int ow_read_byte(one_wire_data_t *owd)
{
  unsigned int byte = 0, i;

  for (i = 0; i < 8; i++) {
    byte >>= 1;
    byte |= (ow_read_bit(owd) << 7);
  }

  return byte;
}

int ow_read_bytes(one_wire_data_t *owd, void *vdata, unsigned int count)
{
  unsigned int i;
  unsigned char *cdata = (unsigned char *)vdata;

  for (i = 0; i < count; i++)
    *cdata++ = (unsigned char)ow_read_byte(owd);

  return count;
}

static void set_bit(unsigned char *devid, unsigned int idx, unsigned int bit)
{
  unsigned int byteno = idx / 8;
  unsigned int bitno = idx % 8;

  if (bit)
    devid[byteno] |= BIT(bitno);
  else
    devid[byteno] &= ~BIT(bitno);
}

static unsigned int get_bit(unsigned char *devid, unsigned int idx)
{
  unsigned int byteno = idx / 8;
  unsigned int bitno = idx % 8;

  return (devid[byteno] >> bitno) & 1;
}

static int one_wire_search(one_wire_data_t *owd, unsigned int devtype)
{
  unsigned int i, bit, cbit, count = 0, crc;
  unsigned int fixed_len = 0, last_discrepancy = 0;
  unsigned char devid[OW_DEVID_LEN];

  memset(devid, 0, sizeof(devid));

  if (devtype) {
    devid[0] = devtype;
    fixed_len = 0;
  }

  for (;;) {
    if (ow_reset(owd) < 0)
      return 0;

    /* send the search command */
    ow_write_byte(owd, 0xf0);

    for (i = 0; i < 64; i++) {
      bit = ow_read_bit(owd);
      cbit = ow_read_bit(owd);

      if (i < last_discrepancy || i < fixed_len)
        bit = get_bit(devid, i);
      else {
        /* no device */
        if (bit == 1 && bit == cbit) {
          xslog(LOG_ERR, "no device\n");
          return 0;
        } else if ( bit == 0 && bit == cbit ) {
          if (i == last_discrepancy) {
            bit = 1;
            last_discrepancy = 0;
          } else
            last_discrepancy = i;
        }
        set_bit(devid, i, bit);
      }

      ow_write_bit(owd, bit);
    }

    crc = ow_crc_update(0, devid, 7);
    if (crc != devid[7]) {
      xslog(LOG_ERR, "crc error on search\n");
      /* allow the caller to decide what to do now */
      return -1;
    }

#if OW_DEBUG
    xslog(LOG_INFO, "found one wire id: %02x%02x%02x%02x%02x%02x%02x%02x\n",
          devid[0], devid[1], devid[2], devid[3],
          devid[4], devid[5], devid[6], devid[7]);
#endif

    if (count < DEVLIST_LEN)
      memcpy(owd->devlist[count++], devid, 8);

    if (last_discrepancy == 0)
      break;
  }

  return count;
}

/* ds18b20 temperature sensor */
#define OWDEVID_DS18B20 0x28

static void one_wire_task(void *arg)
{
  one_wire_data_t *owd = (one_wire_data_t *)arg;
  unsigned char data[9] = { 0 };
  int cnt, temp, count;
  unsigned int crc, cur;

restart:
  for (;;) {
    count = one_wire_search(owd, OWDEVID_DS18B20);
    if (count > 0)
      break;
    task_delay(5000);
  }

  if (owd->found)
    owd->found(count, owd->devlist);

  for (;;) {
    for (cur = 0; cur < count; cur++) {
      unsigned int i;
      unsigned char *dev = owd->devlist[cur];

      if (ow_reset(owd) < 0)
        goto restart;

      /* select device */
      ow_write_byte(owd, 0x55);
      for (i = 0; i < 8; i++)
        ow_write_byte(owd, dev[i]);

      /* start temperature conversion */
      ow_write_byte(owd, 0x44);

      /* poll for temperature conversion ready */
      for (;;) {
        cnt = ow_read_bytes(owd, data, 1);
        if (cnt != 1) {
          xslog(LOG_ERR, "read error\n");
          goto restart;
        }
        if (data[0] == 0xff)
          break;
        task_delay(50);
      }

      /* read temperature data */
      if (ow_reset(owd) < 0)
        goto restart;

      /* select device */
      ow_write_byte(owd, 0x55);
      for (i = 0; i < 8; i++)
        ow_write_byte(owd, dev[i]);

      /* read scratch */
      ow_write_byte(owd, 0xbe);
      cnt = ow_read_bytes(owd, data, 9);
      if (cnt < 9) {
        xslog(LOG_ERR, "read error\n");
        goto restart;
      }

      crc = ow_crc_update(0, data, 8);
      if (crc != data[8])
        xslog(LOG_ERR, "crc error %02x %02x %u\n", crc, data[8], hal_time_us());
      else {
        temp = ((int)(short)(((unsigned int)data[1] << 8) + data[0])) * 1000 /
               16;

#if OW_DEBUG
        xslog(LOG_INFO, "id: %02x%02x%02x%02x%02x%02x%02x%02x temp %d.%03d",
              dev[0], dev[1], dev[2], dev[3], dev[4], dev[5], dev[6], dev[7],
              temp / 1000, temp % 1000);
#endif

        if (owd->temp)
          owd->temp(cur, temp);
      }
    }
    task_delay(owd->poll_interval);
  }
}

static void one_wire_init(ow_found_cb_t *found, ow_temp_cb_t *temp,
                          one_wire_ops_t *ops, void *ops_data,
                          unsigned int interval_ms)
{
  one_wire_data_t *owd;

  owd = malloc(sizeof(one_wire_data_t));
  XASSERT(owd);

  owd->ops = ops;
  owd->ops_data = ops_data;
  owd->found = found;
  owd->temp = temp;
  owd->poll_interval = interval_ms;

  task_init(one_wire_task, owd, "ow", 1, 0, 512);
}

#if OW_UART
void one_wire_init_uart(ow_found_cb_t *found, ow_temp_cb_t *temp, uart_t *uart,
                        unsigned int poll_interval)
{
  one_wire_uart_data_t *ow_uart_data;

  ow_uart_data = malloc(sizeof(one_wire_uart_data_t));
  XASSERT(ow_uart_data);

  ow_uart_init(ow_uart_data, uart);

  one_wire_init(found, temp, &ow_uart_ops, ow_uart_data, poll_interval);
}
#endif

#if OW_GPIO
void one_wire_init_gpio(ow_found_cb_t *found, ow_temp_cb_t *temp,
                        gpio_handle_t gpio, unsigned int poll_interval)
{
  one_wire_gpio_data_t *ow_gpio_data;

  ow_gpio_data = malloc(sizeof(one_wire_gpio_data_t));

  ow_gpio_init(ow_gpio_data, gpio);

  one_wire_init(found, temp, &ow_gpio_ops, ow_gpio_data, poll_interval);
}
#endif
