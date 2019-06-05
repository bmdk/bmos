#include <stdlib.h>

#include <lwip/mem.h>
#include <lwip/sys.h>

#include "xtime.h"

void  mem_init(void)
{
}

void *mem_trim(void *mem, mem_size_t size)
{
  return mem;
}

void *mem_malloc(mem_size_t size)
{
  return malloc(size);
}

void *mem_calloc(mem_size_t count, mem_size_t size)
{
  return calloc(count, size);
}

void  mem_free(void *mem)
{
  free(mem);
}

u32_t sys_now(void)
{
  return xtime_ms();
}
