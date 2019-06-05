#ifndef HAL_INT_H
#define HAL_INT_H

#include "hal_int_cpu.h"

void irq_enable(unsigned int n, int en);
void irq_ack(unsigned int n);
void irq_set_pri(unsigned int n, unsigned int pri);
void irq_set_pending(unsigned int n, int en);

typedef void irq_handler_t (void *);

void irq_register(const char *name, irq_handler_t *handler,
                  void *data, unsigned int num);

void irq_call(unsigned int num);

#if 0
unsigned int interrupt_disable(void);
void interrupt_enable(unsigned int saved);
#define INTERRUPT_OFF() XXXX
#define INTERRUPT_ON() XXXX
#endif

#endif
