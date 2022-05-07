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
#include "bmos_task.h"
#include "common.h"
#include "fb.h"
#include "hal_time.h"
#include "io.h"
#include "shell.h"
#include "hal_rtc.h"

#include "ssd1306_fonts.h"

#include "stm32_hal_i2c.h"

#if STM32_F411BP
#define I2C ((stm32_i2c_t *)I2C1_BASE)
#elif STM32_G4XX
#define I2C ((stm32_i2c_t *)I2C1_BASE)
#elif STM32_H7XX
#define I2C ((stm32_i2c_t *)I2C2_BASE)
#else
#define I2C ((stm32_i2c_t *)I2C4_BASE)
#endif

#if DISP
#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR 0x22
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_CHARGEPUMP 0x8D

#define DISP_ADDR 0x3c

static int disp_cmd(unsigned char cmd)
{
  unsigned char buf[2];

  buf[0] = 0;
  buf[1] = cmd;

  return i2c_write_buf(I2C, DISP_ADDR, buf, 2);
}

static int disp_data(void *data, unsigned int len)
{
  unsigned char buf[17];
  int err;

  if (len > 16)
    len = 16;

  buf[0] = 0x40;

  memcpy(buf + 1, data, len);

  err = i2c_write_buf(I2C, DISP_ADDR, buf, len + 1);
  if (err != 0)
    return -1;

  return len;
}

static void fill(unsigned char b, unsigned int count)
{
  unsigned char data[16];
  unsigned int i, j;

  memset(data, b, 16);

  for (j = 0; j < 8; j++) {
    unsigned int c;

    disp_cmd(0xb0 + j);
    /* LOW COL */
    disp_cmd(0x02);
    /* HIGH COL 0 */
    disp_cmd(0x10);

    for (i = 0; i < 128; i += 16) {
      if (count < 16)
        c = count;
      else
        c = 16;

      count -= c;

      disp_data(data, c);

      if (count == 0)
        goto end;
    }
  }
end:
  return;
}

static void disp_init()
{
  disp_cmd(SSD1306_DISPLAYOFF);
  disp_cmd(SSD1306_MEMORYMODE);
  disp_cmd(SSD1306_SETHIGHCOLUMN);
  disp_cmd(0xB0);
  disp_cmd(0xC8);
  disp_cmd(SSD1306_SETLOWCOLUMN);
  disp_cmd(0x10);
  disp_cmd(0x40);
  disp_cmd(SSD1306_SEGREMAP | 0x01);
  disp_cmd(SSD1306_NORMALDISPLAY);
  disp_cmd(SSD1306_SETMULTIPLEX);
  disp_cmd(0x3F);
  disp_cmd(SSD1306_DISPLAYALLON_RESUME);
  disp_cmd(SSD1306_SETDISPLAYOFFSET);
  disp_cmd(0x0);
  disp_cmd(SSD1306_SETDISPLAYCLOCKDIV);
  disp_cmd(0xF0);
  disp_cmd(SSD1306_SETPRECHARGE);
  disp_cmd(0x22);
  disp_cmd(SSD1306_SETCOMPINS);
  disp_cmd(0x12);
  disp_cmd(SSD1306_SETVCOMDETECT);
  disp_cmd(0x20);
  disp_cmd(SSD1306_CHARGEPUMP);
  disp_cmd(0x14);
  disp_cmd(SSD1306_SETCONTRAST);
  disp_cmd(0x7F);
  disp_cmd(SSD1306_DISPLAYON);
}

extern const char font1[];

static fb_t *fb;

void disp_char(fb_t *fb, int x, int y, char c)
{
  const char *b = &font1[2 + c * 8];
  int i;

  for (i = 0; i < 8; i++) {
    unsigned ch = *(b + i);
    unsigned int j;

    for (j = 0; j < 8; j++) {
      unsigned int v = (ch >> (8 - 1 - j)) & 1;

      if (v)
        fb_draw(fb, x + j, y + i, 1);
    }
  }
}

void disp_char_w(fb_t *fb, int x, int y, char c)
{
  const unsigned char *b = &comic_sans_font24x32_123[4 + c * 3 * 32];
  unsigned int i, j, k;

  for (k = 0; k < 4; k++) {
    for (i = 0; i < 24; i++) {
      unsigned char ch = *(b + 24 * k + i);
      for (j = 0; j < 8; j++) {
        unsigned int v = (ch >> j) & 1;

        if (v)
          fb_draw(fb, x + i, y + k * 8 + j, 1);
      }
    }
  }
}


void disp_str(fb_t *fb, int x, int y, char *s, unsigned int slen)
{
  unsigned int k;

  for (k = 0; k < slen; k++)
    disp_char(fb, k * 8 + x, y, s[k]);
}

void fb_to_i2cdisp(fb_t *fb)
{
  unsigned int i, j;
  unsigned char *data;
  unsigned char buf[16];

  data = fb_get(fb);

  for (j = 0; j < 8; j++) {
    disp_cmd(0xb0 + j);
    /* LOW COL */
    disp_cmd(0x02);
    /* HIGH COL 0 */
    disp_cmd(0x10);

    for (i = 0; i < 128; i += 16) {
      unsigned int k, l;
      memset(buf, 0, 16);
      for (k = 0; k < 16; k++) {
        for (l = 0; l < 8; l++) {
          unsigned int a, o;
          a = ((j * 8 + l) * 128 + i + k) / 8;
          o = k & 7;
          buf[k] |= ((data[a] >> o) & 1) << l;
        }
      }
      disp_data(buf, 16);
    }
  }
}

void task_i2c_clock()
{
  rtc_time_t t, ot;
  unsigned char digits[4];
  unsigned int yo = 16;
  unsigned int xo = 8;

  fb = fb_init(128, 64, 1, 0);

  i2c_init(I2C);
  disp_init();

  memset(&ot, 0, sizeof(rtc_time_t));

  for (;;) {
    rtc_get_time(&t);

    if (t.hours != ot.hours || t.mins != ot.mins) {
      digits[0] = t.hours / 10;
      digits[1] = t.hours % 10;
      digits[2] = t.mins / 10;
      digits[3] = t.mins % 10;

      fb_clear(fb);

      disp_char_w(fb, 0 + xo, yo, 16 + digits[0]);
      disp_char_w(fb, 24 + xo, yo, 16 + digits[1]);
      disp_char_w(fb, 64 + xo, yo, 16 + digits[2]);
      disp_char_w(fb, 88 + xo, yo, 16 + digits[3]);

      fb_to_i2cdisp(fb);

      ot = t;
    }

    task_delay(2000);
  }
}
#endif

static int i2c_cmd(int argc, char *argv[])
{
  char buf[16];
  unsigned int addr, reg;
  int err, len, i;
  unsigned char wbuf;

#if DISP
  if (!fb)
    fb = fb_init(128, 64, 1, 0);
#endif

  switch (argv[1][0]) {
  case 'i':
    xprintf("init %p\n", I2C);
    i2c_init(I2C);
    break;
  case 't':
    buf[0] = 0x5a;
    err = i2c_write_buf(I2C, 0x3c, buf, 1);
    xprintf("err %d\n", err);
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
  case 'r':
    if (argc < 5)
      return -1;
    addr = strtoul(argv[2], 0, 0);
    reg = strtoul(argv[3], 0, 0);
    len = strtoul(argv[4], 0, 0);

    if (len > sizeof(buf))
      len = sizeof(buf);

    wbuf = reg & 0xff;

    err = i2c_write_read_buf(I2C, addr, &wbuf, 1, buf, len);
    if (err != 0)
      xprintf("error %d\n", err);

    for (i = 0; i < len; i++)
      xprintf("%02x", buf[i]);

    xprintf("\n");

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
#if DISP
  case 'd':
    disp_init();
    break;
  case 'f':
    if (argc < 3)
      return -1;
    if (argc > 3)
      len = atoi(argv[3]);
    else
      len = 128 * 8;
    fill(strtoul(argv[2], 0, 0), len);
    break;
  case 'c':
    if (argc < 3)
      return -1;
    disp_cmd(SSD1306_SETCONTRAST);
    disp_cmd(strtoul(argv[2], 0, 0));
    break;
  case 'x':
    /* read distance from GP2Y0E03 - needs 10k pullup or oled display */
    addr = 0x40;
    wbuf = 0x5e;

    err = i2c_write_read_buf(I2C, addr, &wbuf, 1, buf, 2);
    if (err != 0)
      xprintf("error %d\n", err);

    xprintf("dist: %d\n", (((unsigned int)buf[0] << 4) +
                           (buf[1] & 0xf)) * 1000 / 64);

    break;
  case 'q':
    fb_clear(fb);
    //disp_str(fb, 0, 48, "HELLO BRIAN", 11);
    disp_char_w(fb, 0, 0, 16);
    disp_char_w(fb, 32, 0, 17);
    disp_char_w(fb, 64, 0, 18);
    disp_char_w(fb, 96, 0, 19);
    fb_to_i2cdisp(fb);
    break;
#endif
  }

  return 0;
}

SHELL_CMD(i2c, i2c_cmd);
