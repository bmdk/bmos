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
#include "hal_gpio.h"
#include "stm32_hal.h"
#include "stm32_hal_gpio.h"
#include "shell.h"
#include "io.h"

static unsigned char nleds;
static const gpio_handle_t *led;
static const unsigned char *led_flags;

void led_set(unsigned int n, unsigned int v)
{
  unsigned int inv;

  if (n >= nleds || !led)
    return;

  if (led_flags && (led_flags[n] & LED_FLAG_INV))
    inv = 1;
  else
    inv = 0;

  gpio_set(led[n], v ^ inv);
}

void led_init_flags(const gpio_handle_t *led_list,
                    const unsigned char *flags, unsigned int _nleds)
{
  unsigned int i;

  nleds = _nleds;
  led = led_list;
  led_flags = flags;

  for (i = 0; i < nleds; i++)
    gpio_init(led[i], GPIO_OUTPUT);
}

void led_init(const gpio_handle_t *led_list, unsigned int _nleds)
{
  led_init_flags(led_list, 0, _nleds);
}

void delay(unsigned int count)
{
  unsigned int i;

  for (i = 0; i < count; i++)
    asm volatile ("nop");
}

#if STM32_H7XX
#define DBGMCU_IDCODE 0x5C001000
#else
#define DBGMCU_IDCODE 0xE0042000
#endif
#define DBGMCU_IDCODE_ID(val) ((val) & 0xfff)
#define DBGMCU_IDCODE_REV(val) ((val) >> 16 & 0xffff)

int cmd_devid(int argc, char *argv[])
{
  unsigned int val = *(unsigned int *)DBGMCU_IDCODE;
  unsigned int id = DBGMCU_IDCODE_ID(val);
  const char *idstr;

  switch (id) {
  case 0x413:
    idstr = "F405/407/415/417";
    break;
  case 0x415:
    idstr = "L475/476/486";
    break;
  case 0x419:
    idstr = "F42x/43x";
    break;
  case 0x423:
    idstr = "F401xB/C";
    break;
  case 0x433:
    idstr = "F401xD/E";
    break;
  case 0x431:
    idstr = "F411";
    break;
  case 0x435:
    idstr = "L43x/44x";
    break;
  case 0x450:
    idstr = "H743/45/47/53/55/57/50";
    break;
  case 0x451:
    idstr = "F76x/77x";
    break;
  case 0x461:
    idstr = "L496/4A6";
    break;
  case 0x462:
    idstr = "L45x/46x";
    break;
  case 0x464:
    idstr = "L41x/42x";
    break;
  case 0x468:
    idstr = "G4 Cat 2";
    break;
  case 0x469:
    idstr = "G4 Cat 3";
    break;
  case 0x479:
    idstr = "G4 Cat 4";
    break;
  case 0x483:
    idstr = "H72x/H73x";
    break;
  default:
    idstr = "Unknown";
    break;
  }

  xprintf("Chip: %s (%03x) Rev: %04x\n", idstr, id, DBGMCU_IDCODE_REV(val));

  return 0;
}

SHELL_CMD(devid, cmd_devid);
