#include <hal_common.h>
#include <common.h>
#include <shell.h>
#include <io.h>
#include <rp2040_hal_resets.h>

#define ADC_CS_READY BIT(8)
#define ADC_CS_START_MANY BIT(3)
#define ADC_CS_START_ONE BIT(2)
#define ADC_CS_TS_EN BIT(1)
#define ADC_CS_EN BIT(0)

typedef struct {
  reg32_t cs;
  reg32_t result;
  reg32_t fcs;
  reg32_t fifo;
  reg32_t div;
  reg32_t intr;
  reg32_t inte;
  reg32_t intf;
  reg32_t ints;
} rp2040_adc_t;

#define ADC_BASE 0x4004c000
#define ADC ((rp2040_adc_t *)ADC_BASE)

void rp2040_adc_init()
{
  rp2040_reset_clr(RESETS_RESET_ADC);

  ADC->cs |= ADC_CS_EN;

  while (!(ADC->cs & ADC_CS_READY))
    ;
}

int rp2040_adc_conv(unsigned int chan)
{
  while (!(ADC->cs & ADC_CS_READY))
    ;

  reg_set_field(&ADC->cs, 12, 3, chan);

  ADC->cs |= ADC_CS_START_ONE;

  while (!(ADC->cs & ADC_CS_READY))
    ;

  return ADC->result;
}

int cmd_adc(int argc, char *argv[])
{
  int res;

  rp2040_adc_init();

  res = rp2040_adc_conv(0);

  xprintf("%d\n", res);

  return 0;
}

SHELL_CMD(adc, cmd_adc);
