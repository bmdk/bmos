#include "hal_int.h"
#include "stdbool.h"

typedef unsigned int uint;

typedef void (*pico_irq_handler_t)();

pico_irq_handler_t irq_get_vtable_handler(uint num)
{
  return (pico_irq_handler_t)0;
}

void irq_init_priorities()
{
}

void irq_remove_handler(uint num, pico_irq_handler_t handler)
{
}

void irq_set_enabled(uint num, bool enabled)
{
}

void irq_set_exclusive_handler(uint num, pico_irq_handler_t handler)
{
  irq_register("pico", handler, 0, num);
}

