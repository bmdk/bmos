#ifndef CORTEXM_H
#define CORTEXM_H

#include "common.h"

#define SCS_BASE 0xE000E000UL
#define SYSTICK_BASE (SCS_BASE + 0x10UL)
#define NVIC_BASE (SCS_BASE + 0x100UL)
#define SCB_BASE (SCS_BASE + 0xD00UL)

typedef struct {
  unsigned int r0;
  unsigned int r1;
  unsigned int r2;
  unsigned int r3;
  unsigned int r12;
  unsigned int lr;
  unsigned int pc;
  unsigned int xpsr;
} stack_frame_t;

typedef struct {
  unsigned int r4;
  unsigned int r5;
  unsigned int r6;
  unsigned int r7;
  unsigned int r8;
  unsigned int r9;
  unsigned int r10;
  unsigned int r11;
} sw_stack_frame_t;

typedef struct {
  unsigned int ctrl;
  unsigned int load;
  unsigned int val;
  unsigned int calib;
} systick_t;

typedef struct {
  unsigned int cpuid;
  unsigned int icsr;
  unsigned int vtor;
  unsigned int aircr;
  unsigned int scr;
  unsigned int ccr;
  unsigned int shpr[3];
  unsigned int shcsr;
  unsigned int cfsr;
  unsigned int hfsr;
  unsigned int pad[1];
  unsigned int mmfar;
  unsigned int bfar;
  unsigned int afsr;
} scb_t;

typedef struct {
  unsigned int iser[8];
  unsigned int pad0[24];
  unsigned int icer[8];
  unsigned int pad1[24];
  unsigned int ispr[8];
  unsigned int pad2[24];
  unsigned int icpr[8];
  unsigned int pad3[24];
  unsigned int iabr[8];
  unsigned int pad4[56];
  unsigned int ip[240];
  unsigned int pad5[644];
  unsigned int stir;
} nvic_t;

typedef struct {
  reg32_t ctrl;
  reg32_t cyccnt;
  reg32_t cpicnt;
  reg32_t exccnt;
  reg32_t sleepcnt;
  reg32_t lsucnt;
  reg32_t foldcnt;
  reg32_t pcsr;
  struct {
    reg32_t comp;
    reg32_t mask;
    reg32_t function;
    reg32_t pad;
  } event[4];
} dwt_t;

#define DWT ((dwt_t *)0xE0001000)

void set_low_power(int en);

void systick_init();

#define SCB ((volatile scb_t *)SCB_BASE)

static inline void trigger_pendsv()
{
  SCB->icsr = BIT(28);
}

static inline void clear_pendsv()
{
  SCB->icsr = BIT(27);
}

static inline unsigned int cycle_cnt()
{
  return DWT->cyccnt;
}

void dwt_init(void);

#endif
