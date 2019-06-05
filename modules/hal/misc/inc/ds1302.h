#ifndef DS1302_H
#define DS1302_H

#include "hal_gpio.h"

typedef void delay_t (void);

typedef struct {
  gpio_handle_t clk;
  gpio_handle_t dat;
  gpio_handle_t ce;
  delay_t *delay;
} ds1302_t;

void ds1302_init(ds1302_t *d);
unsigned int ds1302_read_reg(ds1302_t *d, unsigned int r);
unsigned int ds1302_read_ram(ds1302_t *d, unsigned int r);
void ds1302_write_ram(ds1302_t *d, unsigned int r, unsigned int val);
void ds1302_write_reg(ds1302_t *d, unsigned int r, unsigned int val);
void ds1302_read_regs(ds1302_t *d, unsigned char *data, unsigned int len);
void ds1302_write_regs(ds1302_t *d, unsigned char *data, unsigned int len);

#endif
