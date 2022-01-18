#include "util.h"

char pchar(unsigned char b)
{
  if (b < 32 || b >= 127)
    return '.';
  return (char)b;
}


