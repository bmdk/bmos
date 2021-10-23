#ifndef FB_H
#define FB_H

#include "common.h"

typedef struct _fb_t fb_t;

#define FB_FLAG_SWAP BIT(0)

fb_t *fb_init(unsigned int width, unsigned int height, unsigned int depth,
              unsigned int flags);
void fb_draw(fb_t *fb, int x, int y, unsigned int col);

void fb_clear(fb_t *fb);
void *fb_get(fb_t *fb);
unsigned int fb_get_size(fb_t *fb);
unsigned int fb_width(fb_t *fb);
unsigned int fb_height(fb_t *fb);

#endif
