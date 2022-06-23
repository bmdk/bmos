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

#include <stddef.h>
#include <string.h>

#include "bmos_task.h"
#include "fast_log.h"
#include "stm32_hal_adc.h"
#include "xslog.h"

unsigned short adc_val[2];

#if AT32_F4XX
unsigned char adc_seq[] = { 17, 16 };

/* from the datasheet for AT32F4XX */
#define VREFINT_V 1200
#define V25 1280
#define T_AVG_SLOPE -4260 // uV/C

#elif STM32_F1XX
/* no calibration values in F1XX so we need to convert from datasheet values */
unsigned char adc_seq[] = { 17, 16 };

/* from the datasheet for F103 */
#define VREFINT_V 1200
#define V25 1430
#define T_AVG_SLOPE 4300 // uV/C

#elif STM32_L4XX || STM32_G4XX
#if STM32_L4XX
unsigned char adc_seq[] = { 0, 17 };
#else
unsigned char adc_seq[] = { 18, 16 };
#endif

#define TS_CAL1 *(unsigned short *)0x1FFF75A8
#define TS_CAL1_TEMP 30
#define TS_CAL2 *(unsigned short *)0x1FFF75CA
#define TS_CAL2_TEMP 130
#define VREFINT_CAL *(unsigned short *)0x1FFF75AA
#define VREFINT_V 3000
#elif STM32_F4XX
unsigned char adc_seq[] = { 17, 18 };

#define TS_CAL1 *(unsigned short *)0x1FFF7A2C
#define TS_CAL1_TEMP 30
#define TS_CAL2 *(unsigned short *)0x1FFF7A2E
#define TS_CAL2_TEMP 110
#define VREFINT_CAL *(unsigned short *)0x1FFF7A2A
#define VREFINT_V 3300
#endif

#ifdef VREFINT_CAL
static unsigned int get_vdd(void)
{
  if (adc_val[0] == 0)
    return 0;

  return VREFINT_V * VREFINT_CAL / adc_val[0];
}

/* scale an adc value by the ratio of calibrated reference voltage
   to the measured reference voltage in adc_val[0] */
static int adc_conv_voltage(int adc_raw_val)
{
  unsigned int vrefint = VREFINT_CAL;
  unsigned int aval = adc_val[0];

  if (aval == 0)
    return 0;

  return (vrefint * adc_raw_val + aval / 2) / aval;
}

/* calculate a temperature from an adc measurement */
static int get_temp(int adcval)
{
  int ts_cal1, ts_cal2, diff;

  ts_cal1 = TS_CAL1;
  ts_cal2 = TS_CAL2;
  diff = ts_cal2 - ts_cal1;

  return (1000 * (TS_CAL2_TEMP - TS_CAL1_TEMP) *
          (adc_conv_voltage(adcval) - ts_cal1) + diff / 2) / diff +
         TS_CAL1_TEMP * 1000;
}
#else
static unsigned int get_vdd(void)
{
  if (adc_val[0] == 0)
    return 0;

  return VREFINT_V * 4095 / adc_val[0];
}

// Temperature (in °C) = {(V 25 - V SENSE ) / Avg_Slope} + 25.
static int get_temp(int adcval)
{
  int v;

  if (adc_val[0] == 0)
    return 0;

#if 1
  v = VREFINT_V * adcval / adc_val[0];
#else
  v = 3300 * adcval / 4095;
#endif

  return 1000000 * (V25 - v) / T_AVG_SLOPE + 25000;
}
#endif

static void adc_conv_done(unsigned short *data, unsigned int count)
{
  memcpy(adc_val, data, count * sizeof(unsigned short));

  xslog(LOG_INFO, "R: VDD:%d T:%d\n", adc_val[0], adc_val[1]);
  xslog(LOG_INFO, "VDD:%d T:%d\n", get_vdd(), get_temp(adc_val[1]));
}

static void adc_task(void *arg)
{
  stm32_adc_init(adc_seq, sizeof(adc_seq));

  for (;;) {
    task_delay(2000);
    stm32_adc_conv(adc_conv_done);
  }
}

void adc_init()
{
  task_init(adc_task, NULL, "adc", 2, 0, 128);
}
