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

#include <stdlib.h>
#include <string.h>

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

#ifndef CONFIG_STM32_HAL_COMMANDS
#define CONFIG_STM32_HAL_COMMANDS 1
#endif

#if STM32_H745NM4
/* the m4 cannot access the system memory, including UDID and FLASH_SIZE */
#elif STM32_H7XX
#define UDID_ADDR 0x1FF1E800
#elif STM32_G4XX || STM32_G0XX || STM32_L4XX
#define UDID_ADDR 0x1FFF7590
#elif STM32_F4XX
#define UDID_ADDR 0x1FFF7A10
#elif STM32_UXXX
#define UDID_ADDR 0x0BFA0700
#elif STM32_F0XX
#define UDID_ADDR 0x1FFFF7AC
#endif

#ifdef UDID_ADDR
void stm32_get_udid(void *buf, unsigned int len)
{
  if (len > 12)
    len = 12;

  memcpy(buf, (void *)UDID_ADDR, len);
}
#endif

#if STM32_H745NM4
/* the m4 cannot access the system memory, including UDID and FLASH_SIZE */
#elif STM32_F4XX
#define FLASH_SIZE 0x1FFF7A22
#elif STM32_F7XX
#define FLASH_SIZE 0x1FF0F442
#elif STM32_H7XX
#define FLASH_SIZE 0x1FF1E880
#elif STM32_UXXX
#define FLASH_SIZE 0x0BFA07A0
#elif STM32_F1XX || AT32_F4XX
#define FLASH_SIZE 0x1FFFF7E0
#elif STM32_F0XX || STM32_F3XX
#define FLASH_SIZE 0x1FFFF7CC
#elif STM32_G0XX || STM32_G4XX || STM32_L4XX || STM32_WBXX || STM32_L4R
#define FLASH_SIZE 0x1FFF75E0
#elif STM32_L0XX
#define FLASH_SIZE 0x1FF8007C
#else
#error X
#endif

#if FLASH_SIZE
unsigned int hal_flash_size(void)
{
  return (unsigned int)*(unsigned short *)FLASH_SIZE;
}
#endif

#if CONFIG_STM32_HAL_COMMANDS
#if STM32_F030 || STM32_F070
/* No unique id on F030/070 */
#elif STM32_H7XX
#define DBGMCU_IDCODE 0x5C001000
#elif STM32_UXXX
#define DBGMCU_IDCODE 0xE0044000
#elif STM32_F0XX || STM32_G0XX || STM32_L0XX
#define DBGMCU_IDCODE 0x40015800
#else
#define DBGMCU_IDCODE 0xE0042000
#endif
#define DBGMCU_IDCODE_ID(val) ((val) & 0xfff)
#define DBGMCU_IDCODE_REV(val) ((val) >> 16 & 0xffff)

#define AT_MANU 0x7005
#define AT32_ID_MANU(val) ((val) >> 16 & 0xffff)
#define AT32_ID_DEV(val) ((val) & 0xffff)

#if AT32_F4XX
typedef struct {
  unsigned short devid;
  char *name;
} at32_devid_t;

static const at32_devid_t at32_devid[] =
{
  { 0x0240, "AT32F403AVCT7" },
  { 0x0241, "AT32F403ARCT7" },
  { 0x0242, "AT32F403ACCT7" },
  { 0x0243, "AT32F403ACCU7" },
  { 0x0344, "AT32F403AVGT7" },
  { 0x0345, "AT32F403ARGT7" },
  { 0x0346, "AT32F403ACGT7" },
  { 0x0347, "AT32F403ACGU7" },
  { 0x0249, "AT32F407VCT7"  },
  { 0x024A, "AT32F407RCT7"  },
  { 0x034B, "AT32F407VGT7"  },
  { 0x034C, "AT32F407RGT7"  },
  { 0x02CD, "AT32F403AVET7" },
  { 0x02CE, "AT32F403ARET7" },
  { 0x02CF, "AT32F403ACET7" },
  { 0x02D0, "AT32F403ACEU7" },
  { 0x02D1, "AT32F407VET7"  },
  { 0x02D2, "AT32F407RET7"  },
  { 0x0353, "AT32F407AVGT7" },
  { 0x0254, "AT32F407AVCT7" }
};

static const char *at32_devname(unsigned int id)
{
  unsigned int i, tlen = ARRSIZ(at32_devid);

  for (i = 0; i < tlen; i++) {
    const at32_devid_t *devid = &at32_devid[i];

    if (id == devid->devid)
      return devid->name;
  }

  return "unknown";
}

static int at32_show_devid(unsigned int id)
{
  const char *name = at32_devname(id);

  xprintf("Chip: %s (%03x)\n", name, id);
  return 0;
}
#endif

#if DBGMCU_IDCODE
int cmd_devid(int argc, char *argv[])
{
  unsigned int val = *(unsigned int *)DBGMCU_IDCODE;
  unsigned int id = DBGMCU_IDCODE_ID(val);
  const char *idstr;

#if AT32_F4XX
  if (AT32_ID_MANU(val) == AT_MANU)
    return at32_show_devid(AT32_ID_DEV(val));
#endif

  switch (id) {
#if STM32_H7XX
  case 0x450:
    idstr = "H743/45/47/53/55/57/50";
    break;
  case 0x483:
    idstr = "H72x/H73x";
    break;
#elif STM32_G0XX
  case 0x456:
    idstr = "G05/6";
    break;
  case 0x460:
    idstr = "G07/8";
    break;
  case 0x466:
    idstr = "G03/4";
    break;
  case 0x467:
    idstr = "G0B/C";
    break;
#elif STM32_F0XX
  case 0x440:
    idstr = "F070x8";
    break;
  case 0x442:
    idstr = "F030xC";
    break;
  case 0x444:
    idstr = "F030x4/6";
    break;
  case 0x445:
    idstr = "F070x6";
    break;
  case 0x448:
    idstr = "F070xB";
    break;
#elif STM32_F1XX
  case 0x0:
    /* errata 2.3 Debug registers cannot be read by user software */
    idstr = "F103";
    break;
  case 0x410:
    idstr = "F103 m";
    break;
  case 0x412:
    idstr = "F103 l";
    break;
  case 0x414:
    idstr = "F103 h";
    break;
  case 0x418:
    idstr = "F103 c";
    break;
  case 0x420:
    idstr = "F100 l/m";
    break;
  case 0x428:
    idstr = "F100 h";
    break;
  case 0x430:
    idstr = "F103 x";
    break;
#elif STM32_F3XX
  case 0x422:
    idstr = "F303xB/C,F358";
    break;
  case 0x438:
    idstr = "F303x6/8,F328";
    break;
  case 0x446:
    idstr = "F303xD/E,F398xE";
    break;
#elif STM32_F4XX
  case 0x413:
    idstr = "F405/407/415/417";
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
#elif STM32_L4XX
  case 0x415:
    idstr = "L475/476/486";
    break;
  case 0x435:
    idstr = "L43x/44x";
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
#elif STM32_L0XX
  case 0x417:
    idstr = "L0xx cat3";
    break;
  case 0x425:
    idstr = "L0xx cat2";
    break;
  case 0x447:
    idstr = "L0xx cat5";
    break;
  case 0x457:
    idstr = "L0xx cat1";
    break;
#elif STM32_U5XX
  case 0x482:
    idstr = "U575/585";
    break;
#elif STM32_WBXX
  case 0x495:
    idstr = "WB55xx/35xx";
    break;
#elif STM32_F7XX
  case 0x451:
    idstr = "F76x/77x";
    break;
#elif STM32_G4XX
  case 0x468:
    idstr = "G4 Cat 2";
    break;
  case 0x469:
    idstr = "G4 Cat 3";
    break;
  case 0x479:
    idstr = "G4 Cat 4";
    break;
#endif
  default:
    idstr = "Unknown";
    break;
  }

  xprintf("Chip: %s (%03x) Rev: %04x\n", idstr, id, DBGMCU_IDCODE_REV(val));

  return 0;
}

SHELL_CMD(devid, cmd_devid);
#endif

int cmd_led(int argc, char *argv[])
{
  unsigned int num, en;

  if (argc < 3)
    return -1;

  num = atoi(argv[1]);
  en = atoi(argv[2]);

  led_set(num, en);

  return 0;
}

SHELL_CMD(led, cmd_led);

#if STM32_H745N
#include "stm32_rcc_h7.h"
#include "stm32_pwr_h7xx.h"
#include "stm32_flash.h"
#include "stm32_wwdg.h"

int cmd_m4(int argc, char *argv[])
{
  char cmd = 's';
  unsigned int opt;
  char *m4opt;

  if (argc > 1)
    cmd = argv[1][0];

  switch (cmd) {
  case 'a':
    boot_cpu(1);
    break;
  case 'b':
    stm32_m4_en(1);
    break;
  case 'h':
    stm32_m4_en(0);
    break;
  case 'r':
    wwdg_reset_cpu2();
    break;
  case 's':
    opt = stm32_flash_opt();
    if (opt & BIT(22))
      m4opt = "boot";
    else
      m4opt = "halt";
    xprintf("m4 %s after reset\n", m4opt);
    break;
  case 'u':
    opt = stm32_ur_get(1);
    xprintf("ur1=%08x\n", opt);
    break;
  }

  return 0;
}

SHELL_CMD(m4, cmd_m4);
#endif
#endif
