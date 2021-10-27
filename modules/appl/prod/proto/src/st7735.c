#include "st7735.h"

#include "bmos_task.h"
#include "fb.h"
#include "hal_gpio.h"
#include "hal_gpio.h"
#include "stm32_hal_spi.h"

#define SPI4_BASE 0x40013400

#define SPI (void *)SPI4_BASE

static stm32_hal_spi_t spi = {
  .base    = (void *)SPI,
  .wordlen = 8,
  .div     = 3,
  .cs      = GPIO(4, 11)
};

#define LCD_DC GPIO(4, 13)

#define DELAY 0x80

// based on Adafruit ST7735 library for Arduino
/* *INDENT-OFF* */
static const unsigned char
  init_cmds[] = {            // Init for 7735R, part 1 (red or green tab)
    ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
      15,                     //     150 ms delay
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
      50,                     //     500 ms delay
    ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,       //     Dot inversion mode
      0x01, 0x2C, 0x2D,       //     Line inversion mode
    ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
      0x07,                   //     No inversion
    ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                   //     -4.6V
      0x84,                   //     AUTO mode
    ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
      0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
      0x0A,                   //     Opamp current small
      0x00,                   //     Boost frequency
    ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
      0x8A,                   //     BCLK/2, Opamp current small & Medium low
      0x2A,  
    ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
      0x0E,
    ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
    ST7735_MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
      ST7735_ROTATION,        //     row addr/col addr, bottom to top refresh
    ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
      0x05,                   //     16-bit color

#if (defined(ST7735_IS_128X128) || defined(ST7735_IS_160X128))
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
#endif // ST7735_IS_128X128

#ifdef ST7735_IS_160X80
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x4F,             //     XEND = 79
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x9F ,            //     XEND = 159
    ST7735_INVON, 0,          //  3: Invert colors
#endif

    ST7735_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      1,                      //     10 ms delay
    ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      10 };                   //     100 ms delay
/* *INDENT-ON* */

static void _wrcmd(unsigned int cmd)
{
  gpio_set(LCD_DC, 0);
  stm32_hal_spi_write(&spi, cmd);
}

static void _wrdat(void* data, unsigned int len)
{
  gpio_set(LCD_DC, 1);
  stm32_hal_spi_write_buf(&spi, data, len);
}

static void _run_cmdlist(const unsigned char *addr, int len)
{
  unsigned int argc, ms;

  while (len > 0) {
    unsigned char cmd = *addr++;
    len--;
    _wrcmd(cmd);

    argc = *addr++;
    len--;
    // If high bit set, delay follows args
    ms = argc & DELAY;
    argc &= ~DELAY;
    if (argc) {
      _wrdat((unsigned char*)addr, argc);
      addr += argc;
      len -= argc;
    }

    if (ms) {
      ms = *addr++;
      ms *= 10;
      len--;
      task_delay(ms);
    }
  }
}

static void _setaddrwin(unsigned int x0,
                        unsigned int y0, unsigned int x1, unsigned int y1)
{
  unsigned char data[] = { 0x00, x0 + ST7735_XSTART,
                           0x00, x1 + ST7735_XSTART };

  // column address set
  _wrcmd(ST7735_CASET);

  _wrdat(data, sizeof(data));

  // row address set
  _wrcmd(ST7735_RASET);
  data[1] = y0 + ST7735_YSTART;
  data[3] = y1 + ST7735_YSTART;
  _wrdat(data, sizeof(data));

  // write to RAM
  _wrcmd(ST7735_RAMWR);
}

void st7735_init()
{
  stm32_hal_spi_init(&spi);

  _run_cmdlist(init_cmds, sizeof(init_cmds));

  _setaddrwin(0, 0, ST7735_WIDTH - 1, ST7735_HEIGHT - 1);

  gpio_set(LCD_DC, 1);
}

void st7735_write(void *data, unsigned int len)
{
  stm32_hal_spi_write_buf(&spi, data, len);
}
