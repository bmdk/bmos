#include "xlib.h"

unsigned int scanx(const char *ptr, unsigned int max_len, unsigned int *val)
{
  unsigned int v = 0, i;
  char c;

  if (max_len < 0)
    max_len = 8;

  for (i = 0; i < max_len; i++) {
    c = 0x20 | *ptr++;
    if (c >= '0' && c <= '9')
      v = (v << 4) + c - '0';
    else if (c >= 'a' && c <= 'f')
      v = (v << 4) + c - 'a' + 10;
    else
      break;
  }

  *val = v;
  return i;
}
