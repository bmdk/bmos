#ifndef STM32_DAC
#define STM32_DAC

/* DAC configurations */
/* external with buffer */
#define STM32_DAC_OUT_EXT_BUF 0
/* external with buffer and internal */
#define STM32_DAC_OUT_EXT_INT 1
/* external no buffer */
#define STM32_DAC_OUT_EXT 2
/* internal only */
#define STM32_DAC_OUT_INT 3

void dac_init(unsigned int dac_no, unsigned int channel, unsigned int cfg);
void dac_set_val(unsigned int dac_no, unsigned int channel, unsigned int val);

#endif
