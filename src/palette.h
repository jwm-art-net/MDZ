#ifndef PALETTE_H
#define PALETTE_H

#include <stdlib.h>
#include <gtk/gtk.h>

#include "image_info.h"
#include "random_palette.h"

#if (G_BYTE_ORDER == G_BIG_ENDIAN)
#   define RGB(r,g,b) ((r) << 24 | ((g) << 16) | ((b) << 8))
#   define RED(x)     (((x) & 0xff000000) >> 24)
#   define GREEN(x)   (((x) & 0x00ff0000) >> 16)
#   define BLUE(x)    (((x) & 0x0000ff00) >> 8)
#elif (G_BYTE_ORDER == G_LITTLE_ENDIAN)
#   define RGB(r,g,b) ((r) | ((g) << 8) | ((b) << 16))
#   define RED(x)     ((x) & 0x000000ff)
#   define GREEN(x)   (((x) & 0x0000ff00) >> 8)
#   define BLUE(x)    (((x) & 0x00ff0000) >> 16)
#else
#   error Your machine has an unsupported byte order. Please send patch :)
#endif


extern int pal_offset;
extern int pal_indexes;
extern guint32* palette;

int palette_init(void);

char** palette_get_paths(void);

int palette_load(char* filename);
int palette_save(char* filename);
int palette_read(FILE* fd);
int palette_write(FILE* fd);

char* palette_get_filename(void);

void palette_apply(image_info* img,
                   int x0, int y0, int width, int height);

void palette_rotate_backward(void);

void palette_rotate_forward(void);

void palette_randomize(random_palette* rndpal);

void palette_apply_func(function_palette* funpal);

void palette_shift(int offset);

guint32 get_pixel_colour(double raw_scaled, gboolean palette_ip);

void palette_free(void);

#endif
