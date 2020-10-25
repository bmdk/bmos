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
#include <string.h>

#include "common.h"
#include "debug_ser.h"
#include "debug_ser.h"
#include "hal_board.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "stm32_can.h"
#include "stm32_hal.h"
#include "stm32_hal_gpio.h"
#include "stm32_pwr.h"
#include "stm32_pwr_fxxx.h"
#include "stm32_regs.h"

void clear_button_int()
{
  EXTI->pr = BIT(11);
}

volatile stm32_lcd_t *lcd = (stm32_lcd_t *)0x40016800;

#define LCD_MEM_SIZE (480 * 272)

unsigned char framebuf[LCD_MEM_SIZE];

#define LCD_X 480
#define LCD_Y 272

#define LCD_PFCR_RGB888 1
#define LCD_PFCR_RGB565 2
#define LCD_PFCR_L8 5

#define LCD_CR_LEN BIT(0)
#define LCD_CR_COLKEN BIT(1)
#define LCD_CR_CLUTEN BIT(4)

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

void set_lcd_colour(unsigned int colour)
{
  for (int i = 0; i < LCD_MEM_SIZE; i++)
    framebuf[i] = colour & 0xffff;
}

static void set_clut(unsigned int idx, unsigned int r, unsigned int g,
                     unsigned int b)
{
  lcd->l1clutwr = ((idx & 0xff) << 24) | ((r & 0xff) << 16 ) |
                  ((g & 0xff) << 8 ) | ((b & 0xff) << 0 );
}

void lcd_init(unsigned int x, unsigned int y)
{
  unsigned int i;

  lcd->sscr = (9 << 16) | (1);
  lcd->bpcr = (29 << 16) | (3);
  lcd->awcr = ((x + 29) << 16) | ((y + 3));
  lcd->twcr = ((x + 29 + 10) << 16) | (y + 4 + 3);
  lcd->bccr = 0x0000ff00;

  lcd->gcr = 0x00000001;

  set_lcd_colour(0);

  lcd->l1pfcr = LCD_PFCR_L8;
  lcd->l1cacr = 0xff;
  lcd->l1dccr = 0x0;
  lcd->l1bfcr = (6 << 8) | (7);
  lcd->l1whpcr = ((x + 29) << 16) | (29 + 1);
  lcd->l1wvpcr = ((y + 3) << 16) | (3 + 1);
  lcd->l1cfbar = (unsigned int)framebuf;
  lcd->l1cfblr = ((x * 2) << 16) | (x * 2 + 3);
  lcd->l1cfblnr = y;

  lcd->l1cr = LCD_CR_LEN;
  lcd->srcr = 1;

  for (i = 0; i < 256; i++)
    set_clut(i, 0, 0, i);

  lcd->l1cr |= LCD_CR_CLUTEN;
  lcd->srcr = 1;
}

typedef struct {
  gpio_handle_t gpio;
  unsigned char alt;
} gpio_init_tab_t;

/* *INDENT-OFF* */
static gpio_init_tab_t lcd_gpio_init[] = {
  { GPIO(4, 4), 14 },
  { GPIO(6, 12), 9 },
  { GPIO(8, 9), 14 },
  { GPIO(8, 10), 14 },
  { GPIO(8, 13), 14 },
  { GPIO(8, 14), 14 },
  { GPIO(8, 15), 14 },
  { GPIO(9, 0) , 14 },
  { GPIO(9, 1), 14 },
  { GPIO(9, 2), 14 },
  { GPIO(9, 3), 14 },
  { GPIO(9, 4), 14 },
  { GPIO(9, 5), 14 },
  { GPIO(9, 6), 14 },
  { GPIO(9, 7), 14 },
  { GPIO(9, 8), 14 },
  { GPIO(9, 9), 14 },
  { GPIO(9, 10), 14 },
  { GPIO(9, 11), 14 },
  { GPIO(9, 13), 14 },
  { GPIO(9, 14), 14 },
  { GPIO(9, 15), 14 },
  { GPIO(10, 0), 14 },
  { GPIO(10, 1), 14 },
  { GPIO(10, 2), 14 },
  { GPIO(10, 4), 14 },
  { GPIO(10, 5), 14 },
  { GPIO(10, 6), 14 },
  { GPIO(10, 7), 14 },
};
/* *INDENT-ON* */

#if APPL
static void eth_pin_init()
{
  enable_ahb1(25); /* ETHMACEN */
  enable_ahb1(26); /* ETHMACTXEN */
  enable_ahb1(27); /* ETHMACRXEN */
  enable_ahb1(28); /* ETHMACPTPEN */

  stm32_syscfg_eth_phy(1);

  /* RMII pins */
  gpio_init_attr(GPIO(0, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(0, 2), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(0, 7), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  //gpio_init_attr(GPIO(1, 13), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(2, 1), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(2, 4), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(2, 5), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(6, 11), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(6, 13), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
  gpio_init_attr(GPIO(6, 14), GPIO_ATTR_STM32(0, GPIO_SPEED_HIG, 11, GPIO_ALT));
}
#endif

void pin_init()
{
#if 1
  unsigned int i;
#endif

  enable_ahb1(0);  /* GPIOA */
  enable_ahb1(1);  /* GPIOB */
  enable_ahb1(2);  /* GPIOC */
  enable_ahb1(3);  /* GPIOD */
  enable_ahb1(5);  /* GPIOF */
  enable_ahb1(6);  /* GPIOG */
  enable_ahb1(7);  /* GPIOH */
  enable_ahb1(8);  /* GPIOI */
  enable_ahb1(9);  /* GPIOJ */
  enable_ahb1(10); /* GPIOK */

  enable_apb1(28); /* PWR */

#if 1
  /* USART 1 */
  enable_apb2(4);

  gpio_init_attr(GPIO(0, 9), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(1, 7), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 7, GPIO_ALT));
#endif

#define GPIO_BUTTON GPIO(8, 11)
  gpio_init(GPIO_BUTTON, GPIO_INPUT);

#define GPIO_BL_CTRL GPIO(10, 3)
  gpio_set(GPIO_BL_CTRL, 1);
  gpio_init(GPIO_BL_CTRL, GPIO_OUTPUT);

#define GPIO_LCD_DISP GPIO(8, 12)
  gpio_set(GPIO_LCD_DISP, 1);
  gpio_init(GPIO_LCD_DISP, GPIO_OUTPUT);

  /* USART 6 */
  enable_apb2(5);

  gpio_init_attr(GPIO(2, 6), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 8, GPIO_ALT));
  gpio_init_attr(GPIO(2, 7), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 8, GPIO_ALT));

  /* TIM2 */
  enable_apb1(0);

#if 0
  /* CAN1 */
  enable_apb1(25);
  gpio_init_attr(GPIO(3, 0), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 9, GPIO_ALT));
  gpio_init_attr(GPIO(3, 1), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 9, GPIO_ALT));
#endif

  /* SYSCFG */
  enable_apb2(14);

#if 1
  EXTI->pr = 0xffffffff;

  SYSCFG->exticr[2] = (8 << 12); /* EXT13 PI11 */
  EXTI->rtsr = 0;
  EXTI->ftsr |= BIT(11);
  EXTI->emr |= BIT(11);
  EXTI->imr |= BIT(11);
#endif

#if 1
  /* LCD */
  enable_apb2(26);

  for (i = 0; i < ARRSIZ(lcd_gpio_init); i++) {
    gpio_init_tab_t *e = &lcd_gpio_init[i];
    gpio_init_attr(e->gpio, GPIO_ATTR_STM32(0, \
                                            GPIO_SPEED_VHI, e->alt, GPIO_ALT));
  }
#endif
#if APPL
  eth_pin_init();
#endif
}

#define USART1_BASE (void *)0x40011000
#define USART6_BASE (void *)0x40011400
#define APB2_CLOCK 60000000
#if BMOS
uart_t debug_uart = { "debugser", USART1_BASE, APB2_CLOCK, 37 };
#endif

static const gpio_handle_t leds[] = { GPIO(8, 1) };

void hal_board_init()
{
  pin_init();
  clock_init();
  backup_domain_protect(0);
  clock_init_ls();
  led_init(leds, ARRSIZ(leds));
  debug_uart_init(USART1_BASE, 115200, APB2_CLOCK, 0);
#if APPL
  lcd_init(480, 272);
#endif
}
