#ifndef STM32_DAC
#define STM32_DAC

void dac_init(unsigned int dac_no, unsigned int channel, unsigned int cfg);
void dac_set_val(unsigned int dac_no, unsigned int channel, unsigned int val);

#endif
