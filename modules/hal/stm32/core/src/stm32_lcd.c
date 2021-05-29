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

#include "stm32_lcd.h"
#include "common.h"

typedef struct {
  unsigned int pad0[2];
  unsigned int sscr;
  unsigned int bpcr;
  unsigned int awcr;
  unsigned int twcr;
  unsigned int gcr;
  unsigned int pad1[2];
  unsigned int srcr;
  unsigned int bccr;
  unsigned int pad2[2];
  unsigned int ier;
  unsigned int isr;
  unsigned int icr;
  unsigned int lipcr;
  unsigned int cpsr;
  unsigned int cdsr;
  unsigned int pad3[14];
  unsigned int l1cr;
  unsigned int l1whpcr;
  unsigned int l1wvpcr;
  unsigned int l1ckcr;
  unsigned int l1pfcr;
  unsigned int l1cacr;
  unsigned int l1dccr;
  unsigned int l1bfcr;
  unsigned int pad4[2];
  unsigned int l1cfbar;
  unsigned int l1cfblr;
  unsigned int l1cfblnr;
  unsigned int pad5[3];
  unsigned int l1clutwr;
} stm32_lcd_t;

#if STM32_H7XX
volatile stm32_lcd_t *lcd = (stm32_lcd_t *)0x50001000;
#else
volatile stm32_lcd_t *lcd = (stm32_lcd_t *)0x40016800;
#endif

#define LCD_PFCR_RGB888 1
#define LCD_PFCR_RGB565 2
#define LCD_PFCR_L8 5

#define LCD_CR_LEN BIT(0)
#define LCD_CR_COLKEN BIT(1)
#define LCD_CR_CLUTEN BIT(4)

#if 0
void draw_line(unsigned int fgcol, unsigned int bgcol,
               int min, int max)
{
  int x, y;
  unsigned short col;

  for (x = 0; x < LCD_X; x++) {
    if (x > min && x <= max)
      col = fgcol;
    else
      col = bgcol;

    for (y = 0; y < LCD_Y; y++)
      framebuf[y * 480 + x] = col;
  }
}
#endif

void set_lcd_colour(unsigned int colour, unsigned int width,
                    unsigned int height)
{
  for (int i = 0; i < width * height; i++)
    framebuf[i] = colour & 0xffff;
}

void set_lcd(unsigned int start, unsigned int width, unsigned int height)
{
  for (int i = 0; i < height; i++)
    for (int j = 0; j < width; j++)
      framebuf[i * width + j] = (i + start) & 0xffff;
}

static void set_clut(unsigned int idx, unsigned int r, unsigned int g,
                     unsigned int b)
{
  lcd->l1clutwr = ((idx & 0xff) << 24) | ((r & 0xff) << 16 ) |
                  ((g & 0xff) << 8 ) | ((b & 0xff) << 0 );
}

#define WIDTH 1

void lcd_init(unsigned int width, unsigned int height)
{
  unsigned int i;

  lcd->sscr = (9 << 16) | (1);
  lcd->bpcr = (29 << 16) | (3);
  lcd->awcr = ((width + 29) << 16) | ((height + 3));
  lcd->twcr = ((width + 29 + 10) << 16) | (height + 4 + 3);
  lcd->bccr = 0x0000ff00;

  lcd->gcr = 0x00000001;

  //set_lcd_colour(0xff, width, height);
  set_lcd(130, width, height);

  lcd->l1pfcr = LCD_PFCR_L8;
  lcd->l1cacr = 0xff;
  lcd->l1dccr = 0x0;
  lcd->l1bfcr = (6 << 8) | (7);
  lcd->l1whpcr = ((width + 29) << 16) | (29 + 1);
  lcd->l1wvpcr = ((height + 3) << 16) | (3 + 1);
  lcd->l1cfbar = (unsigned int)framebuf;
  lcd->l1cfblr = ((width * WIDTH) << 16) | (width * WIDTH + 7);
  lcd->l1cfblnr = height;

  lcd->l1cr = LCD_CR_LEN;
  lcd->srcr = 1;

  for (i = 0; i < 256; i++) {
    unsigned int green = i;
    set_clut(i, 0, green, 0);
  }

  lcd->l1cr |= LCD_CR_CLUTEN;
  lcd->srcr = 1;
}
