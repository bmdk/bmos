#include <stdlib.h>
#include <string.h>

#include "fb.h"

struct _fb_t {
  unsigned char stride;
  unsigned char depth;
  unsigned short width;
  unsigned short height;
  unsigned int size;
  unsigned char *fb;
};

fb_t *fb_init(unsigned int width, unsigned int height, unsigned int depth)
{
  unsigned int size, stride;
  fb_t *fb;

  if (depth < 8)
    stride = 1;
  else if (depth < 16)
    stride = 2;
  else
    stride = 4;

  size = width * height * stride;

  fb = malloc(sizeof(fb_t));
  if (fb == 0)
    return 0;

  fb->fb = calloc(1, size);
  if (fb == 0) {
    free(fb);
    return 0;
  }

  fb->size = size;
  fb->width = width;
  fb->height = height;
  fb->stride = stride;
  fb->depth = depth;

  return fb;
}

void *fb_get(fb_t *fb)
{
  return (void *)fb->fb;
}

void fb_clear(fb_t *fb)
{
  memset(fb->fb, 0, fb->size);
}

unsigned int fb_width(fb_t *fb)
{
  return (unsigned int)fb->width;
}

unsigned int fb_height(fb_t *fb)
{
  return (unsigned int)fb->height;
}

void fb_draw(fb_t *fb, int x, int y, unsigned int col)
{
  void *addr;
  unsigned int base, stride;

  if (x < 0 || x >= fb->width)
    return;

  if (y < 0 || y >= fb->height)
    return;

  stride = fb->stride;

  base = (y * fb->width + x) * stride;

  addr = &fb->fb[base];

  switch (stride) {
  case 1:
    *(unsigned char *)addr = (unsigned char)col;
    break;
  case 2:
    *(unsigned short *)addr = (unsigned short)col;
    break;
  case 4:
  default:
    *(unsigned int *)addr = (unsigned int)col;
    break;
  }
}
