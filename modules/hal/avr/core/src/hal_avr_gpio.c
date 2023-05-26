/* Copyright (c) 2019-2022 Brian Thomas Murphy
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

#include <common.h>
#include <avr/io.h>
#include <hal_gpio.h>

#if __AVR_ATmega328P__
#define N_GPIOS 4
#else
#define N_GPIOS 7
#endif

typedef volatile unsigned char reg8_t;

typedef struct {
  reg8_t *pin;
  reg8_t *port;
  reg8_t *ddr;
} avr_gpio_regs_t;

static const avr_gpio_regs_t avr_gpio_regs[N_GPIOS] = {
#if __AVR_ATmega328P__
  { 0, 0, 0 },
  { &PINB, &PORTB, &DDRB },
  { &PINC, &PORTC, &DDRC },
  { &PIND, &PORTD, &DDRD },
#else
  { &PINA, &PORTA, &DDRA },
  { &PINB, &PORTB, &DDRB },
  { &PINC, &PORTC, &DDRC },
  { &PIND, &PORTD, &DDRD },
  { &PINE, &PORTE, &DDRE },
  { &PINF, &PORTF, &DDRF },
  { &PING, &PORTG, &DDRG }
#endif
};

void gpio_init(gpio_handle_t gpio, unsigned int type)
{
  unsigned char bank = GPIO_BANK(gpio);
  unsigned char pin = GPIO_PIN(gpio);

  if (bank >= N_GPIOS)
    return;

  if (avr_gpio_regs[bank].ddr == 0)
    return;

  if (type == GPIO_INPUT)
    *avr_gpio_regs[bank].ddr &= ~BIT(pin);
  else
    *avr_gpio_regs[bank].ddr |= BIT(pin);
}

void gpio_set(gpio_handle_t gpio, int val)
{
  unsigned char bank = GPIO_BANK(gpio);
  unsigned char pin = GPIO_PIN(gpio);

  if (bank >= N_GPIOS)
    return;

  if (val)
    *avr_gpio_regs[bank].port |= BIT(pin);
  else
    *avr_gpio_regs[bank].port &= ~BIT(pin);
}

int gpio_get(gpio_handle_t gpio)
{
  unsigned char bank = GPIO_BANK(gpio);
  unsigned char pin = GPIO_PIN(gpio);

  if (bank >= N_GPIOS)
    return 0;

  return (*avr_gpio_regs[bank].pin >> pin) & 1;
}

