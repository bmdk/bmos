#ifndef STM32_HAL_ADC_H
#define STM32_HAL_ADC_H

typedef void conv_done_f (unsigned short *adc_dat, unsigned int count);

void stm32_adc_init(unsigned char *reg_seq, unsigned int cnt);
int stm32_adc_conv(conv_done_f *conv_done);

#endif
