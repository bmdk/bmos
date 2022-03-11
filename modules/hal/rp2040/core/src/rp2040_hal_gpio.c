#include "hal_gpio.h"
#include "rp2040_hal_gpio.h"

#define PADS0_BASE 0x4001c000

typedef struct {
  reg32_t voltage_select;
  reg32_t pad[32];
} rp2040_pads_t;

#define PADS0 ((rp2040_pads_t *)PADS0_BASE)

void rp2040_set_pad_attr(unsigned int n, unsigned int attr)
{
  if (n >= RP2040_N_GPIOS)
    return;
  PADS0->pad[n] = attr;
}

static void gpio_set_func(unsigned int n, unsigned int func)
{
  if (n >= RP2040_N_GPIOS)
    return;

  reg_set_field(&GPIO_PAD->pad[n].ctrl, 5, 0, func);
}

void gpio_init(unsigned int n, unsigned int type)
{
  if (n >= RP2040_N_GPIOS)
    return;

  reg_set_field(&GPIO_PAD->pad[n].ctrl, 5, 0, GPIO_FUNC_SIO);
  if (type == GPIO_INPUT)
    SIO->gpio_oe_clr = BIT(n);
  else
    SIO->gpio_oe_set = BIT(n);
}

#if 0
void xgpio_init(unsigned int n, unsigned int type)
{
  gpio_init(n, type);
}
#endif

void gpio_init_attr(unsigned int n, unsigned int attr)
{
  if (n >= RP2040_N_GPIOS)
    return;

  gpio_set_func(n, attr);
}

void gpio_set(gpio_handle_t gpio, int val)
{
  if (val)
    SIO->gpio_out_set = BIT(gpio);
  else
    SIO->gpio_out_clr = BIT(gpio);
}

int gpio_get(gpio_handle_t gpio)
{
  return (SIO->gpio_in >> gpio) & 0x1;
}
