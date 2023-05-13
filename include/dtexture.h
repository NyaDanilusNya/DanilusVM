/*
 * a simple library for drawing simple shapes on the canvas, which will be used for texture in SDL in the future
 */
#ifndef DANILUS_TEXTURE_H
#define DANILUS_TEXTURE_H

#include <stdint.h>

;
/* canvas lol */
typedef struct d_canvas
{
  uint32_t* pixels;
  unsigned short width, height;
} d_Canvas;

/* Create a new canvas with specific width and height, return pointer to canvas */
d_Canvas* d_newCanvas(unsigned short width, unsigned short height);

/* Set specified pixel */
void d_setpixel(d_Canvas* canvas, int x, int y, uint32_t color);

/* Fills the entire canvas with a specific color */
void d_fill(d_Canvas* canvas, uint32_t color);

/* Draw filled rect on the canvas */
void d_rect(d_Canvas* canvas, int x, int y, int w, int h, uint32_t color);

/* Returns the color (RGBA) (uint32_t) of a pixel at the specified coordinates */
uint32_t d_getPixel(d_Canvas* canvas, int x, int y);

/* Free allocated memory */
void d_freeCanvas(d_Canvas* canvas);

/* copies the selected area of ​​pixels and shifts it */
void d_copy(d_Canvas* canvas, int x, int y, int w, int h, int xoffset, int yoffset);

#endif /* DANILUS_TEXTURE_H */
