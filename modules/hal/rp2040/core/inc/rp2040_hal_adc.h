#ifndef RP2040_HAL_ADC
#define RP2040_HAL_ADC

void rp2040_adc_init(void);
int rp2040_adc_conv(unsigned int chan);

#endif
