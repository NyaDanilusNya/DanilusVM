#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "dtexture.h"


Canvas_t*
dt_NewCanvas(uint16_t width, uint16_t height)
{
  Canvas_t* can = (Canvas_t*)malloc(sizeof(Canvas_t));
  can->pixels = malloc(sizeof(uint32_t)*width*height);
  can->width = width;
  can->height = height;
  return can;
}

void
dt_SetPixel(Canvas_t* canvas, int x, int y, uint32_t color)
{
  // -1 cause we need a pixel from the array, not the resolution
  if (x < 0 || y < 0 || x > canvas->width-1 || y > canvas->height-1)
    return;
  canvas->pixels[canvas->width*y+x] = color;
}

void
dt_Fill(Canvas_t* canvas, uint32_t color)
{
  for (int iw = 0; iw < canvas->width; iw++)
  {
    for (int ih = 0; ih < canvas->height; ih++)
    {
      dt_SetPixel(canvas, iw, ih, color);
    }
  }
}

void
dt_Rect(Canvas_t* canvas, int x, int y, int w, int h, uint32_t color)
{
  for (int iw = 0; iw < w; iw++)
  {
    for (int ih = 0; ih < h; ih++)
    {
        dt_SetPixel(canvas, x+iw, y+ih, color);
    }
  }
}

uint32_t
dt_GetPixel(Canvas_t* canvas, int x, int y)
{
  if (x < 0 || y < 0 || x > canvas->width-1 || y > canvas->height-1)
    return 0;
  return canvas->pixels[canvas->width*y+x];
}

void
dt_FreeCanvas(Canvas_t* canvas)
{
  free(canvas->pixels);
  free(canvas);
}

void
dt_Copy(Canvas_t* canvas, int x, int y, int w, int h, int xoffset, int yoffset)
{
  for (int iw = 0; iw < w; iw++)
  {
    for (int ih = 0; ih < h; ih++)
    {
      dt_SetPixel(canvas, x+iw+xoffset, y+ih+yoffset, dt_GetPixel(canvas, x+iw, y+ih));
    }
  }
}

