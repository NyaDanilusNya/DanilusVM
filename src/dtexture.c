#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "dtexture.h"


d_Canvas* d_newCanvas(uint16_t width, uint16_t height)
{
  d_Canvas* can = (d_Canvas*)malloc(sizeof(d_Canvas));
  can->pixels = malloc(sizeof(uint32_t)*width*height);
  can->width = width;
  can->height = height;
  return can;
}

void d_setpixel(d_Canvas* canvas, int x, int y, uint32_t color)
{
  // -1 cause we need a pixel from the array, not the resolution
  if (x < 0 || y < 0 || x > canvas->width-1 || y > canvas->height-1)
    return;
  canvas->pixels[canvas->width*y+x] = color;
}

void d_fill(d_Canvas* canvas, uint32_t color)
{
  for (int iw = 0; iw < canvas->width; iw++)
  {
    for (int ih = 0; ih < canvas->height; ih++)
    {
      d_setpixel(canvas, iw, ih, color);
    }
  }
}

void d_rect(d_Canvas* canvas, int x, int y, int w, int h, uint32_t color)
{
  for (int iw = 0; iw < w; iw++)
  {
    for (int ih = 0; ih < h; ih++)
    {
      /*if (x+w > canvas->width)
        d_setpixel(canvas, canvas->width, y+ih, color);
      else if (y+h > canvas->height)
        d_setpixel(canvas, x+iw, canvas->height, color);
      else */
        d_setpixel(canvas, x+iw, y+ih, color);
    }
  }
}

uint32_t
d_getPixel(d_Canvas* canvas, int x, int y)
{
  if (x < 0 || y < 0 || x > canvas->width-1 || y > canvas->height-1)
    return 0;
  return canvas->pixels[canvas->width*y+x];
}

void
d_freeCanvas(d_Canvas* canvas)
{
  free(canvas->pixels);
  free(canvas);
}

void d_copy(d_Canvas* canvas, int x, int y, int w, int h, int xoffset, int yoffset)
{
  for (int iw = 0; iw < w; iw++)
  {
    for (int ih = 0; ih < h; ih++)
    {
      // \/ stupid thing
      /*if (xoffset+x+iw < 0 || xoffset+x+iw > canvas->width || yoffset+y+ih < 0 || yoffset+y+ih > canvas->height)
        continue;*/

      d_setpixel(canvas, x+iw+xoffset, y+ih+yoffset, d_getPixel(canvas, x+iw, y+ih));
    }
  }
}

