/* vim: set ai et ts=4 sw=4: */
#ifndef __ST7735_H__
#define __ST7735_H__

#define ST7735_MADCTL_MY  0x80
#define ST7735_MADCTL_MX  0x40
#define ST7735_MADCTL_MV  0x20
#define ST7735_MADCTL_ML  0x10
#define ST7735_MADCTL_RGB 0x00
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH  0x04

// AliExpress/eBay 1.8" display, default orientation
/*
 #define ST7735_IS_160X128 1
 #define ST7735_WIDTH  128
 #define ST7735_HEIGHT 160
 #define ST7735_XSTART 0
 #define ST7735_YSTART 0
 #define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MY)
 */

// AliExpress/eBay 1.8" display, rotate right
/*
 #define ST7735_IS_160X128 1
 #define ST7735_WIDTH  160
 #define ST7735_HEIGHT 128
 #define ST7735_XSTART 0
 #define ST7735_YSTART 0
 #define ST7735_ROTATION (ST7735_MADCTL_MY | ST7735_MADCTL_MV)
 */

// AliExpress/eBay 1.8" display, rotate left
/*
 #define ST7735_IS_160X128 1
 #define ST7735_WIDTH  160
 #define ST7735_HEIGHT 128
 #define ST7735_XSTART 0
 #define ST7735_YSTART 0
 #define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MV)
 */

// AliExpress/eBay 1.8" display, upside down
/*
 #define ST7735_IS_160X128 1
 #define ST7735_WIDTH  128
 #define ST7735_HEIGHT 160
 #define ST7735_XSTART 0
 #define ST7735_YSTART 0
 #define ST7735_ROTATION (0)
 */

// WaveShare ST7735S-based 1.8" display, default orientation
/*
 #define ST7735_IS_160X128 1
 #define ST7735_WIDTH  128
 #define ST7735_HEIGHT 160
 #define ST7735_XSTART 2
 #define ST7735_YSTART 1
 #define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB)
 */

// WaveShare ST7735S-based 1.8" display, rotate right
/*
 #define ST7735_IS_160X128 1
 #define ST7735_WIDTH  160
 #define ST7735_HEIGHT 128
 #define ST7735_XSTART 1
 #define ST7735_YSTART 2
 #define ST7735_ROTATION (ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_RGB)
 */

// WaveShare ST7735S-based 1.8" display, rotate left
/*
 #define ST7735_IS_160X128 1
 #define ST7735_WIDTH  160
 #define ST7735_HEIGHT 128
 #define ST7735_XSTART 1
 #define ST7735_YSTART 2
 #define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_RGB)
 */

// WaveShare ST7735S-based 1.8" display, upside down
/*
 #define ST7735_IS_160X128 1
 #define ST7735_WIDTH  128
 #define ST7735_HEIGHT 160
 #define ST7735_XSTART 2
 #define ST7735_YSTART 1
 #define ST7735_ROTATION (ST7735_MADCTL_RGB)
 */

// 1.44" display, default orientation
/*
 #define ST7735_IS_128X128 1
 #define ST7735_WIDTH  128
 #define ST7735_HEIGHT 128
 #define ST7735_XSTART 2
 #define ST7735_YSTART 3
 #define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_BGR)
 */

// 1.44" display, rotate right
/*
 #define ST7735_IS_128X128 1
 #define ST7735_WIDTH  128
 #define ST7735_HEIGHT 128
 #define ST7735_XSTART 3
 #define ST7735_YSTART 2
 #define ST7735_ROTATION (ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_BGR)
 */

// 1.44" display, rotate left
/*
 #define ST7735_IS_128X128 1
 #define ST7735_WIDTH  128
 #define ST7735_HEIGHT 128
 #define ST7735_XSTART 1
 #define ST7735_YSTART 2
 #define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_BGR)
 */

// 1.44" display, upside down
/*
 #define ST7735_IS_128X128 1
 #define ST7735_WIDTH  128
 #define ST7735_HEIGHT 128
 #define ST7735_XSTART 2
 #define ST7735_YSTART 1
 #define ST7735_ROTATION (ST7735_MADCTL_BGR)
 */

// mini 160x80 display (it's unlikely you want the default orientation)
/*
 #define ST7735_IS_160X80 1
 #define ST7735_XSTART 26
 #define ST7735_YSTART 1
 #define ST7735_WIDTH  80
 #define ST7735_HEIGHT 160
 #define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_BGR)
 */

// mini 160x80, rotate left
/*
 #define ST7735_IS_160X80 1
 #define ST7735_XSTART 1
 #define ST7735_YSTART 26
 #define ST7735_WIDTH  160
 #define ST7735_HEIGHT 80
 #define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_BGR)
 */

// mini 160x80, rotate right

#define ST7735_IS_160X80 1
#define ST7735_XSTART 1
#define ST7735_YSTART 26
#define ST7735_WIDTH  160
#define ST7735_HEIGHT 80
#define ST7735_ROTATION (ST7735_MADCTL_MY | ST7735_MADCTL_MV | \
                         ST7735_MADCTL_BGR)

/****************************/

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

void st7735_init(void);
void st7735_write(void *data, unsigned int len);

#endif // __ST7735_H__
