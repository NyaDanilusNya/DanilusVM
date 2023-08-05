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
} Canvas_t;

/* Create a new canvas with specific width and height, return pointer to canvas */
Canvas_t* dt_NewCanvas(unsigned short width, unsigned short height);

/* Set specified pixel */
void dt_SetPixel(Canvas_t* canvas, int x, int y, uint32_t color);

/* Fills the entire canvas with a specific color */
void dt_Fill(Canvas_t* canvas, uint32_t color);

/* Draw filled rect on the canvas */
void dt_Rect(Canvas_t* canvas, int x, int y, int w, int h, uint32_t color);

/* Returns the color (RGBA) (uint32_t) of a pixel at the specified coordinates */
uint32_t dt_GetPixel(Canvas_t* canvas, int x, int y);

/* Free allocated memory */
void dt_FreeCanvas(Canvas_t* canvas);

/* copies the selected area of ​​pixels and shifts it */
void dt_Copy(Canvas_t* canvas, int x, int y, int w, int h, int xoffset, int yoffset);

#endif /* DANILUS_TEXTURE_H */
