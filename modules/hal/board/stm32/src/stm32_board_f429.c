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
#include <string.h>

#include "common.h"
#include "debug_ser.h"
#include "debug_ser.h"
#include "hal_board.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "io.h"
#include "shell.h"
#include "stm32_hal.h"
#include "hal_board.h"
#include "stm32_hal_gpio.h"
#include "stm32_hal_spi.h"
#include "stm32_pwr_f4xx.h"
#include "stm32_rcc_a.h"
#include "stm32_lcd.h"

#define LCD_X 240
#define LCD_Y 320

#define LCD_MEM_SIZE (LCD_X * LCD_Y)

unsigned char framebuf[LCD_MEM_SIZE];

static stm32_hal_spi_t spi = {
  (void *)0x40015000, 8, 0, 0
};

typedef struct {
  gpio_handle_t gpio;
  unsigned char alt;
} gpio_init_tab_t;

/* *INDENT-OFF* */
static gpio_init_tab_t lcd_gpio_init[] = {
 { GPIO(0, 3), 14 },
 { GPIO(0, 4), 14 },
 { GPIO(0, 6), 14 },
 { GPIO(0, 11), 14 },
 { GPIO(0, 12), 14 },
 { GPIO(1, 0), 9 },
 { GPIO(1, 1), 9 },
 { GPIO(1, 8), 14 },
 { GPIO(1, 9), 14 },
 { GPIO(1, 10), 14 },
 { GPIO(1, 11), 14 },
 { GPIO(2, 6), 14 },
 { GPIO(2, 7), 14 },
 { GPIO(2, 10), 14 },
 { GPIO(3, 3), 14 },
 { GPIO(3, 6), 14 },
 { GPIO(5, 10), 14 },
 { GPIO(6, 6), 14 },
 { GPIO(6, 7), 14 },
 { GPIO(6, 10), 9 },
 { GPIO(6, 11), 14 },
 { GPIO(6, 12), 9 },
};
/* *INDENT-ON* */

void pin_init()
{
  unsigned int i;

  enable_ahb1(0); /* GPIOA */
  enable_ahb1(1); /* GPIOB */
  enable_ahb1(2); /* GPIOC */
  enable_ahb1(3); /* GPIOD */
  enable_ahb1(5); /* GPIOF */
  enable_ahb1(6); /* GPIOG */

  /* USART 1 */
  enable_apb2(4);

  gpio_init_attr(GPIO(0, 9), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 7, GPIO_ALT));
  gpio_init_attr(GPIO(0, 10), GPIO_ATTR_STM32(0, \
                                              GPIO_SPEED_HIG, 7, GPIO_ALT));

  /* TIM1 */
  enable_apb2(0);

  /* TIM2 */
  enable_apb1(0);

  /* DMA 1 */
  enable_ahb1(21);

  /* DMA 2 */
  enable_ahb1(22);

#if 0
  /* CAN1 */
  enable_apb1(25);
  gpio_init_attr(GPIO(3, 0), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 9, GPIO_ALT));
  gpio_init_attr(GPIO(3, 1), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 9, GPIO_ALT));
#endif

#define LCD_CSX GPIO(2, 2)
#define LCD_WRX GPIO(3, 13)

  gpio_init(LCD_CSX, GPIO_OUTPUT);
  gpio_init(LCD_WRX, GPIO_OUTPUT);

  gpio_set(LCD_CSX, 0);
  gpio_set(LCD_CSX, 1);
  gpio_set(LCD_WRX, 0);

#if 1
  /* SPI5 */
  enable_apb2(20);

  gpio_init_attr(GPIO(5, 7), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 5, GPIO_ALT));
  gpio_init_attr(GPIO(5, 8), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 5, GPIO_ALT));
  gpio_init_attr(GPIO(5, 9), GPIO_ATTR_STM32(0, \
                                             GPIO_SPEED_HIG, 5, GPIO_ALT));
#endif

  /* LCD */
  enable_apb2(26);

  for (i = 0; i < ARRSIZ(lcd_gpio_init); i++) {
    gpio_init_tab_t *e = &lcd_gpio_init[i];
    gpio_init_attr(e->gpio, GPIO_ATTR_STM32(0, \
                                            GPIO_SPEED_HIG, e->alt, GPIO_ALT));
  }
}

void lcd_wr(unsigned int data)
{
  gpio_set(LCD_CSX, 0);
  stm32_hal_spi_write(&spi, data);
  stm32_hal_spi_wait_done(&spi);
  gpio_set(LCD_CSX, 1);
}

void lcd_wr_data(unsigned int data)
{
  gpio_set(LCD_WRX, 1);
  lcd_wr(data);
}

void lcd_wr_reg(unsigned int reg)
{
  gpio_set(LCD_WRX, 0);
  lcd_wr(reg);
}

/* Level 1 Commands */
#define LCD_SWRESET             0x01   /* Software Reset */
#define LCD_READ_DISPLAY_ID     0x04   /* Read display identification information */
#define LCD_RDDST               0x09   /* Read Display Status */
#define LCD_RDDPM               0x0A   /* Read Display Power Mode */
#define LCD_RDDMADCTL           0x0B   /* Read Display MADCTL */
#define LCD_RDDCOLMOD           0x0C   /* Read Display Pixel Format */
#define LCD_RDDIM               0x0D   /* Read Display Image Format */
#define LCD_RDDSM               0x0E   /* Read Display Signal Mode */
#define LCD_RDDSDR              0x0F   /* Read Display Self-Diagnostic Result */
#define LCD_SPLIN               0x10   /* Enter Sleep Mode */
#define LCD_SLEEP_OUT           0x11   /* Sleep out register */
#define LCD_PTLON               0x12   /* Partial Mode ON */
#define LCD_NORMAL_MODE_ON      0x13   /* Normal Display Mode ON */
#define LCD_DINVOFF             0x20   /* Display Inversion OFF */
#define LCD_DINVON              0x21   /* Display Inversion ON */
#define LCD_GAMMA               0x26   /* Gamma register */
#define LCD_DISPLAY_OFF         0x28   /* Display off register */
#define LCD_DISPLAY_ON          0x29   /* Display on register */
#define LCD_COLUMN_ADDR         0x2A   /* Colomn address register */
#define LCD_PAGE_ADDR           0x2B   /* Page address register */
#define LCD_GRAM                0x2C   /* GRAM register */
#define LCD_RGBSET              0x2D   /* Color SET */
#define LCD_RAMRD               0x2E   /* Memory Read */
#define LCD_PLTAR               0x30   /* Partial Area */
#define LCD_VSCRDEF             0x33   /* Vertical Scrolling Definition */
#define LCD_TEOFF               0x34   /* Tearing Effect Line OFF */
#define LCD_TEON                0x35   /* Tearing Effect Line ON */
#define LCD_MAC                 0x36   /* Memory Access Control register*/
#define LCD_VSCRSADD            0x37   /* Vertical Scrolling Start Address */
#define LCD_IDMOFF              0x38   /* Idle Mode OFF */
#define LCD_IDMON               0x39   /* Idle Mode ON */
#define LCD_PIXEL_FORMAT        0x3A   /* Pixel Format register */
#define LCD_WRITE_MEM_CONTINUE  0x3C   /* Write Memory Continue */
#define LCD_READ_MEM_CONTINUE   0x3E   /* Read Memory Continue */
#define LCD_SET_TEAR_SCANLINE   0x44   /* Set Tear Scanline */
#define LCD_GET_SCANLINE        0x45   /* Get Scanline */
#define LCD_WDB                 0x51   /* Write Brightness Display register */
#define LCD_RDDISBV             0x52   /* Read Display Brightness */
#define LCD_WCD                 0x53   /* Write Control Display register*/
#define LCD_RDCTRLD             0x54   /* Read CTRL Display */
#define LCD_WRCABC              0x55   /* Write Content Adaptive Brightness Control */
#define LCD_RDCABC              0x56   /* Read Content Adaptive Brightness Control */
#define LCD_WRITE_CABC          0x5E   /* Write CABC Minimum Brightness */
#define LCD_READ_CABC           0x5F   /* Read CABC Minimum Brightness */
#define LCD_READ_ID1            0xDA   /* Read ID1 */
#define LCD_READ_ID2            0xDB   /* Read ID2 */
#define LCD_READ_ID3            0xDC   /* Read ID3 */

/* Level 2 Commands */
#define LCD_RGB_INTERFACE       0xB0   /* RGB Interface Signal Control */
#define LCD_FRMCTR1             0xB1   /* Frame Rate Control (In Normal Mode) */
#define LCD_FRMCTR2             0xB2   /* Frame Rate Control (In Idle Mode) */
#define LCD_FRMCTR3             0xB3   /* Frame Rate Control (In Partial Mode) */
#define LCD_INVTR               0xB4   /* Display Inversion Control */
#define LCD_BPC                 0xB5   /* Blanking Porch Control register */
#define LCD_DFC                 0xB6   /* Display Function Control register */
#define LCD_ETMOD               0xB7   /* Entry Mode Set */
#define LCD_BACKLIGHT1          0xB8   /* Backlight Control 1 */
#define LCD_BACKLIGHT2          0xB9   /* Backlight Control 2 */
#define LCD_BACKLIGHT3          0xBA   /* Backlight Control 3 */
#define LCD_BACKLIGHT4          0xBB   /* Backlight Control 4 */
#define LCD_BACKLIGHT5          0xBC   /* Backlight Control 5 */
#define LCD_BACKLIGHT7          0xBE   /* Backlight Control 7 */
#define LCD_BACKLIGHT8          0xBF   /* Backlight Control 8 */
#define LCD_POWER1              0xC0   /* Power Control 1 register */
#define LCD_POWER2              0xC1   /* Power Control 2 register */
#define LCD_VCOM1               0xC5   /* VCOM Control 1 register */
#define LCD_VCOM2               0xC7   /* VCOM Control 2 register */
#define LCD_NVMWR               0xD0   /* NV Memory Write */
#define LCD_NVMPKEY             0xD1   /* NV Memory Protection Key */
#define LCD_RDNVM               0xD2   /* NV Memory Status Read */
#define LCD_READ_ID4            0xD3   /* Read ID4 */
#define LCD_PGAMMA              0xE0   /* Positive Gamma Correction register */
#define LCD_NGAMMA              0xE1   /* Negative Gamma Correction register */
#define LCD_DGAMCTRL1           0xE2   /* Digital Gamma Control 1 */
#define LCD_DGAMCTRL2           0xE3   /* Digital Gamma Control 2 */
#define LCD_INTERFACE           0xF6   /* Interface control register */

/* Extend register commands */
#define LCD_POWERA               0xCB   /* Power control A register */
#define LCD_POWERB               0xCF   /* Power control B register */
#define LCD_DTCA                 0xE8   /* Driver timing control A */
#define LCD_DTCB                 0xEA   /* Driver timing control B */
#define LCD_POWER_SEQ            0xED   /* Power on sequence register */
#define LCD_3GAMMA_EN            0xF2   /* 3 Gamma enable register */
#define LCD_PRC                  0xF7   /* Pump ratio control register */

/* Size of read registers */
#define LCD_READ_ID4_SIZE        3      /* Size of Read ID4 */


void lcd_panel_init()
{
  lcd_wr_reg(0xCA);
  lcd_wr_data(0xC3);
  lcd_wr_data(0x08);
  lcd_wr_data(0x50);
  lcd_wr_reg(LCD_POWERB);
  lcd_wr_data(0x00);
  lcd_wr_data(0xC1);
  lcd_wr_data(0x30);
  lcd_wr_reg(LCD_POWER_SEQ);
  lcd_wr_data(0x64);
  lcd_wr_data(0x03);
  lcd_wr_data(0x12);
  lcd_wr_data(0x81);
  lcd_wr_reg(LCD_DTCA);
  lcd_wr_data(0x85);
  lcd_wr_data(0x00);
  lcd_wr_data(0x78);
  lcd_wr_reg(LCD_POWERA);
  lcd_wr_data(0x39);
  lcd_wr_data(0x2C);
  lcd_wr_data(0x00);
  lcd_wr_data(0x34);
  lcd_wr_data(0x02);
  lcd_wr_reg(LCD_PRC);
  lcd_wr_data(0x20);
  lcd_wr_reg(LCD_DTCB);
  lcd_wr_data(0x00);
  lcd_wr_data(0x00);
  lcd_wr_reg(LCD_FRMCTR1);
  lcd_wr_data(0x00);
  lcd_wr_data(0x1B);
  lcd_wr_reg(LCD_DFC);
  lcd_wr_data(0x0A);
  lcd_wr_data(0xA2);
  lcd_wr_reg(LCD_POWER1);
  lcd_wr_data(0x10);
  lcd_wr_reg(LCD_POWER2);
  lcd_wr_data(0x10);
  lcd_wr_reg(LCD_VCOM1);
  lcd_wr_data(0x45);
  lcd_wr_data(0x15);
  lcd_wr_reg(LCD_VCOM2);
  lcd_wr_data(0x90);
  lcd_wr_reg(LCD_MAC);
  lcd_wr_data(0xC8);
  lcd_wr_reg(LCD_3GAMMA_EN);
  lcd_wr_data(0x00);
  lcd_wr_reg(LCD_RGB_INTERFACE);
  lcd_wr_data(0xC2);
  lcd_wr_reg(LCD_DFC);
  lcd_wr_data(0x0A);
  lcd_wr_data(0xA7);
  lcd_wr_data(0x27);
  lcd_wr_data(0x04);

  /* Colomn address set */
  lcd_wr_reg(LCD_COLUMN_ADDR);
  lcd_wr_data(0x00);
  lcd_wr_data(0x00);
  lcd_wr_data(0x00);
  lcd_wr_data(0xEF);
  /* Page address set */
  lcd_wr_reg(LCD_PAGE_ADDR);
  lcd_wr_data(0x00);
  lcd_wr_data(0x00);
  lcd_wr_data(0x01);
  lcd_wr_data(0x3F);
  lcd_wr_reg(LCD_INTERFACE);
  lcd_wr_data(0x01);
  lcd_wr_data(0x00);
  lcd_wr_data(0x06);

  lcd_wr_reg(LCD_GRAM);
  delay(2000000);

  lcd_wr_reg(LCD_GAMMA);
  lcd_wr_data(0x01);

  lcd_wr_reg(LCD_PGAMMA);
  lcd_wr_data(0x0F);
  lcd_wr_data(0x29);
  lcd_wr_data(0x24);
  lcd_wr_data(0x0C);
  lcd_wr_data(0x0E);
  lcd_wr_data(0x09);
  lcd_wr_data(0x4E);
  lcd_wr_data(0x78);
  lcd_wr_data(0x3C);
  lcd_wr_data(0x09);
  lcd_wr_data(0x13);
  lcd_wr_data(0x05);
  lcd_wr_data(0x17);
  lcd_wr_data(0x11);
  lcd_wr_data(0x00);
  lcd_wr_reg(LCD_NGAMMA);
  lcd_wr_data(0x00);
  lcd_wr_data(0x16);
  lcd_wr_data(0x1B);
  lcd_wr_data(0x04);
  lcd_wr_data(0x11);
  lcd_wr_data(0x07);
  lcd_wr_data(0x31);
  lcd_wr_data(0x33);
  lcd_wr_data(0x42);
  lcd_wr_data(0x05);
  lcd_wr_data(0x0C);
  lcd_wr_data(0x0A);
  lcd_wr_data(0x28);
  lcd_wr_data(0x2F);
  lcd_wr_data(0x0F);

  lcd_wr_reg(LCD_SLEEP_OUT);
  delay(2000000);
  lcd_wr_reg(LCD_DISPLAY_ON);
  /* GRAM start writing */
  lcd_wr_reg(LCD_GRAM);
}

#define USART1_BASE (void *)0x40011000
#define APB2_CLOCK 60000000
#if BMOS
uart_t debug_uart = { "debugser", USART1_BASE, APB2_CLOCK, 37 };
#endif

static const gpio_handle_t leds[] = { GPIO(6, 13), GPIO(6, 14) };

/* 8MHz crystal input
   120MHz CPU clock
   60MHz AHB1
   30MHz AHB2
 */
static const struct pll_params_t pll_params = {
  .src     = RCC_A_CLK_HSE_OSC,
  .pllr    = 0,
  .pllp    = RCC_A_PLLP_2,
  .pllq    = 5,
  .plln    = 120,
  .pllm    = 4,
  .hpre    = 0,
  .ppre1   = RCC_A_PPRE_4,
  .ppre2   = RCC_A_PPRE_2,
  .latency = 5
};

unsigned int hal_cpu_clock = 120000000;

void hal_board_init()
{
  pin_init();
  clock_init(&pll_params);
  led_init(leds, ARRSIZ(leds));
  debug_uart_init(USART1_BASE, 115200, APB2_CLOCK, 0);
#if APPL
  stm32_hal_spi_init(&spi);
  lcd_panel_init();
  lcd_init(LCD_X, LCD_Y);
#endif
}
