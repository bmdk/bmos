#ifndef STM32_HRTIM
#define STM32_HRTIM

#define HRTIM_TIMX_PERR_MAX 0xffdf /* 2^16 - 32 */

#define HRTIM_ADCTRIG_MC1 0
#define HRTIM_ADCTRIG_MC2 1
#define HRTIM_ADCTRIG_MC3 2
#define HRTIM_ADCTRIG_MC4 3
#define HRTIM_ADCTRIG_MPER 4

void hrtim_adc_trig_en(unsigned int adc_trig, unsigned int bit, int en);

void hrtim_set_compare(unsigned int timer,
                       unsigned int compare, unsigned int val);

void hrtim_mst_set_compare(unsigned int compare, unsigned int val);

void hrtim_set_output(unsigned int timer, unsigned int o,
                      unsigned int set, unsigned int rst);

void hrtim_set_output_on(unsigned int timer, unsigned int o, int on);

void hrtim_set_output_compare(unsigned int timer, unsigned int o,
                              unsigned int compare, int on);

#define HRTIM_TIM_FLG_O1EN_NUM 0
#define HRTIM_TIM_FLG_O1EN BIT(HRTIM_TIM_FLG_O1EN_NUM)
#define HRTIM_TIM_FLG_O2EN_NUM 1
#define HRTIM_TIM_FLG_O2EN BIT(HRTIM_TIM_FLG_O2EN_NUM)
#define HRTIM_TIM_FLG_CONT_NUM 2
#define HRTIM_TIM_FLG_CONT BIT(HRTIM_TIM_FLG_CONT_NUM)

void hrtim_tim_init(unsigned int tim, unsigned int cntr,
                   unsigned int div_pow_2, unsigned int flags);

void hrtim_mst_init(unsigned int cntr,
                    unsigned int div_pow_2, unsigned int flags);

#endif
