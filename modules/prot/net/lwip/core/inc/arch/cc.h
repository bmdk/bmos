#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#define LWIP_PLATFORM_DIAG(x) do {xprintf x;} while(0)
#include <io.h>

#define LWIP_PLATFORM_ASSERT(x) do {xpanic("Assertion failed at line %d in %s\n", \
                                     __LINE__, __S_FILE__);} while(0)
#include <xassert.h>

#define LWIP_NO_CTYPE_H 1

#endif
