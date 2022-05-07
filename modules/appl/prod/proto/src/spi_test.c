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

#include "io.h"
#include "shell.h"
#include "stm32_hal_spi.h"
#include "hal_gpio.h"
#include "hal_rtc.h"
#include "fb.h"

#include "ssd1306_fonts.h"
#include "st7735.h"

/* H7XX */
#define SPI1_BASE 0x40013000
#define SPI2_BASE 0x40003800
#define SPI3_BASE 0x40003C00
#define SPI4_BASE 0x40013400
#define SPI5_BASE 0x40015000
#define SPI6_BASE 0x58001400

#define SPI (void *)SPI1_BASE

static stm32_hal_spi_t spi = {
  .base    = (void *)SPI,
  .wordlen = 8,
  .div     = 12,
  .cs      = GPIO(3, 14)
};

extern const char font1[];

static fb_t *fb;

#if 0
static void disp_char(fb_t *fb, int x, int y, char c)
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
#endif

static void disp_char_w(fb_t *fb, int x, int y, char c, unsigned int col)
{
  const unsigned char *b = &comic_sans_font24x32_123[4 + c * 3 * 32];
  unsigned int i, j, k;

  for (k = 0; k < 4; k++) {
    for (i = 0; i < 24; i++) {
      unsigned char ch = *(b + 24 * k + i);
      for (j = 0; j < 8; j++) {
        unsigned int v = (ch >> j) & 1;

        if (v)
          fb_draw(fb, x + i, y + k * 8 + j, col);
      }
    }
  }
}

static void fb_to_spi_disp()
{
  unsigned char *data;
  unsigned int size = fb_get_size(fb);

  data = fb_get(fb);

  st7735_write(data, size);
}

void task_spi_clock()
{
  rtc_time_t t, ot;
  unsigned char digits[4];
  unsigned int xo = 24, yo = 24;

  memset(&ot, 0, sizeof(rtc_time_t));

  st7735_init();

  fb = fb_init(160, 80, 16, FB_FLAG_SWAP);

  for (;;) {
    rtc_get_time(&t);

    if (t.hours != ot.hours || t.mins != ot.mins) {
      rtc_get_time(&t);

      digits[0] = t.hours / 10;
      digits[1] = t.hours % 10;
      digits[2] = t.mins / 10;
      digits[3] = t.mins % 10;

      fb_clear(fb);

      disp_char_w(fb, 0 + xo, yo, 16 + digits[0], 0xf800);
      disp_char_w(fb, 24 + xo, yo, 16 + digits[1], 0x3f << 5);
      disp_char_w(fb, 64 + xo, yo, 16 + digits[2], 0x1f);
      disp_char_w(fb, 88 + xo, yo, 16 + digits[3], 0xffff);

      fb_to_spi_disp(fb);
      ot = t;
    }

    task_delay(2000);
  }
}

static int spi_cmd(int argc, char *argv[])
{
  unsigned char buf[] = { 0x5a, 0xa5, 0xff };
  rtc_time_t t;
  unsigned char digits[4];
  unsigned int xo = 24, yo = 24;

  switch (argv[1][0]) {
  case 'i':
    xprintf("init %p\n", spi.base);
    stm32_hal_spi_init(&spi);
    break;
  case 'w':
    xprintf("write 1\n");
    stm32_hal_spi_write(&spi, 0x5a);
    break;
  case 'l':
    xprintf("write %d\n", sizeof(buf));
    stm32_hal_spi_write_buf(&spi, buf, sizeof(buf));
    break;
  case 'd':
    xprintf("disp\n");
    st7735_init();

    if (!fb)
      fb = fb_init(160, 80, 16, FB_FLAG_SWAP);

    rtc_get_time(&t);

    digits[0] = t.hours / 10;
    digits[1] = t.hours % 10;
    digits[2] = t.mins / 10;
    digits[3] = t.mins % 10;

    fb_clear(fb);

    disp_char_w(fb, 0 + xo, yo, 16 + digits[0], 0xf800);
    disp_char_w(fb, 24 + xo, yo, 16 + digits[1], 0x3f << 5);
    disp_char_w(fb, 64 + xo, yo, 16 + digits[2], 0x1f);
    disp_char_w(fb, 88 + xo, yo, 16 + digits[3], 0xffff);

    fb_to_spi_disp(fb);
    break;
  }

  return 0;
}

SHELL_CMD(spi, spi_cmd);
