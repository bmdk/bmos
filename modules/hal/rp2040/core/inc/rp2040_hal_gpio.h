#ifndef PICO_HAL_GPIO_H
#define PICO_HAL_GPIO_H

#include "hal_common.h"
#include "hal_gpio.h"

#define GPIO_FUNC_XIP 0
#define GPIO_FUNC_SPI 1
#define GPIO_FUNC_UART 2
#define GPIO_FUNC_I2C 3
#define GPIO_FUNC_PWM 4
#define GPIO_FUNC_SIO 5
#define GPIO_FUNC_PIO0 6
#define GPIO_FUNC_PIO1 7
#define GPIO_FUNC_GPCK 8
#define GPIO_FUNC_USB 9
#define GPIO_FUNC_NULL 0xf

#define GPIO_ATTR_PICO(func) (func)

#define IO_BANK0_BASE 0x40014000

#define RP2040_N_GPIOS 30

typedef struct {
  reg32_t status;
  reg32_t ctrl;
} gpio_pad_t;

typedef struct {
  gpio_pad_t pad[RP2040_N_GPIOS];
} gpio_regs_t;

typedef struct {
  reg32_t cpuid;
  reg32_t gpio_in;
  reg32_t gpio_hi_in;
  reg32_t pad0;
  reg32_t gpio_out;
  reg32_t gpio_out_set;
  reg32_t gpio_out_clr;
  reg32_t gpio_out_xor;
  reg32_t gpio_oe;
  reg32_t gpio_oe_set;
  reg32_t gpio_oe_clr;
  reg32_t gpio_oe_xor;
  reg32_t gpio_hi_out;
  reg32_t gpio_hi_out_set;
  reg32_t gpio_hi_out_clr;
  reg32_t gpio_hi_out_xor;
  reg32_t gpio_hi_oe;
  reg32_t gpio_hi_oe_set;
  reg32_t gpio_hi_oe_clr;
  reg32_t gpio_hi_oe_xor;
  reg32_t fifo_st;
  reg32_t fifo_wr;
  reg32_t fifo_rd;
  reg32_t spinlock_st;
  /* ... later */
} sio_t;

#define GPIO_PAD ((gpio_regs_t *)IO_BANK0_BASE)
#define SIO_BASE 0xd0000000
#define SIO ((sio_t *)SIO_BASE)

#define PAD_OD BIT(7)
#define PAD_IE BIT(6)
#define PAD_DRIVE(_v_) (((_v_) & 0x3) << 4)
#define PAD_PUE BIT(3)
#define PAD_PDE BIT(2)
#define PAD_SCHMITT BIT(1)
#define PAD_SLEWFAST BIT(0)

void rp2040_set_pad_attr(unsigned int n, unsigned int attr);

#endif
