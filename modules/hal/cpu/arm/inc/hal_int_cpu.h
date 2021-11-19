#ifndef HAL_INT_CPU_H
#define HAL_INT_CPU_H

#define INTERRUPT_OFF() asm volatile ("cpsid i")
#define INTERRUPT_ON() asm volatile ("cpsie i")

#define __ISB() asm volatile ("isb")
#define __DSB() asm volatile ("dsb")

#undef __WFI
#define __WFI() asm volatile ("wfi")

static inline void set_psp(void *psp)
{
  asm volatile ("msr psp, %0\n" : : "r" (psp));
}

static inline void set_basepri(unsigned int pri)
{
  asm volatile ("msr basepri, %0\n" : : "r" (pri));
}

static inline void set_faultmask(unsigned int msk)
{
  asm volatile ("msr faultmask, %0\n" : : "r" (msk));
}

static inline unsigned int interrupt_disable()
{
  unsigned int ret;

  asm volatile ("mrs %0, primask\n"
                "cpsid i\n"
                : "=r" (ret));
  return ret;
}

static inline void interrupt_enable(unsigned int saved)
{
  asm volatile ("msr primask, %0\n"
                : : "r" (saved));
}

#endif
