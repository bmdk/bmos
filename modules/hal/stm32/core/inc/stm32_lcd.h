#ifndef STM32_LCD_H
#define STM32_LCD_H

extern unsigned char framebuf[];

void draw_line(unsigned int fgcol, unsigned int bgcol,
               int min, int max);
void set_lcd_colour(unsigned int colour, unsigned int width,
                    unsigned int height);
void lcd_init(unsigned int x, unsigned int y);

#endif
