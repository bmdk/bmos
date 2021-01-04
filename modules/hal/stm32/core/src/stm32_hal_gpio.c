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

#include "common.h"
#include "hal_common.h"
#include "hal_gpio.h"
#include "stm32_hal_gpio.h"
#include "stm32_regs.h"

static void stm32_gpio_mode(volatile stm32_gpio_t *gpio, unsigned int pin,
                            unsigned int mode)
{
  reg_set_field(&gpio->moder, 2, pin << 1, mode);
}

static void stm32_gpio_otype(volatile stm32_gpio_t *gpio, unsigned int pin,
                             unsigned int flags)
{
  if (flags & GPIO_FLAG_OPEN_DRAIN)
    gpio->otyper |= BIT(pin);
  else
    gpio->otyper &= ~BIT(pin);
}

static void stm32_gpio_pupd(volatile stm32_gpio_t *gpio, unsigned int pin,
                            unsigned int flags)
{
  reg_set_field(&gpio->pupdr, 2, pin << 1, flags & 0x3);
}

static void stm32_gpio_ospeed(volatile stm32_gpio_t *gpio, unsigned int pin,
                              unsigned int speed)
{
  reg_set_field(&gpio->ospeedr, 2, pin << 1, speed);
}

static void stm32_gpio_alt(volatile stm32_gpio_t *gpio, unsigned int pin,
                           unsigned int mode)
{
  volatile unsigned int *reg;

  stm32_gpio_mode(gpio, pin, GPIO_ALT);

  if (pin < 8)
    reg = &gpio->afrl;
  else {
    reg = &gpio->afrh;
    pin -= 8;
  }

  reg_set_field(reg, 4, pin << 2, mode);
}

static void stm32_gpio_set(volatile stm32_gpio_t *gpio, unsigned int num,
                           unsigned int val)
{
  unsigned int reg;

  if (num > 15)
    return;

  if (val)
    reg = BIT(num);
  else
    reg = BIT(16 + num);

  gpio->bsrr = reg;
}

static int stm32_gpio_get(volatile stm32_gpio_t *gpio, unsigned int num)
{
  if (num > 15)
    return 0;

  return (int)((gpio->idr >> num) & 0x1);
}

void gpio_set(gpio_handle_t gpio, int val)
{
  unsigned int bank = GPIO_BANK(gpio);
  unsigned int pin = GPIO_PIN(gpio);

  stm32_gpio_set(STM32_GPIO(bank), pin, val);
}

int gpio_get(gpio_handle_t gpio)
{
  unsigned int bank = GPIO_BANK(gpio);
  unsigned int pin = GPIO_PIN(gpio);

  return stm32_gpio_get(STM32_GPIO(bank), pin);
}

void gpio_init(gpio_handle_t gpio, unsigned int type)
{
  unsigned int bank = GPIO_BANK(gpio);
  unsigned int pin = GPIO_PIN(gpio);

  stm32_gpio_mode(STM32_GPIO(bank), pin, type);
}

void gpio_init_attr(gpio_handle_t gpio, unsigned int attr)
{
  unsigned int bank = GPIO_BANK(gpio);
  unsigned int pin = GPIO_PIN(gpio);
  volatile stm32_gpio_t *gpio_reg = STM32_GPIO(bank);

  stm32_gpio_alt(gpio_reg, pin, GPIO_ATTR_STM32_ALT(attr));
  stm32_gpio_pupd(gpio_reg, pin, GPIO_ATTR_STM32_FLAGS(attr));
  stm32_gpio_otype(gpio_reg, pin, GPIO_ATTR_STM32_FLAGS(attr));
  stm32_gpio_ospeed(gpio_reg, pin, GPIO_ATTR_STM32_SPEED(attr));
  stm32_gpio_mode(gpio_reg, pin, GPIO_ATTR_STM32_TYPE(attr));
}
