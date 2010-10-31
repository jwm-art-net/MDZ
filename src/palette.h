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

#define PAL_IP_RGB_INIT \
    double ip_cval;     \
    double ip_diff;     \
    double ip_rdiff;    \
    int ip_ind1;        \
    int ip_ind2;        \
    guint32 ip_c1;      \
    guint32 ip_c2;      \
    guint8 ip_r;        \
    guint8 ip_g;        \
    guint8 ip_b;

#define PAL_IP_RGB( ip_val )                                            \
    /** FIXME: optimize this */                                         \
    ip_cval = ceil( ip_val );                                           \
    ip_diff = ip_cval - ip_val;                                         \
    ip_rdiff = 1.0 - ip_diff;                                           \
    ip_ind1 = ((guint32)floor( ip_val )) % (pal_indexes-1);             \
    ip_ind2 = (guint32)ip_cval % (pal_indexes-1);                       \
    ip_c1 = palette[ip_ind1];                                           \
    ip_c2 = palette[ip_ind2];                                           \
    ip_r = (guint8)(ip_diff * RED(  ip_c1) + ip_rdiff * RED(  ip_c2));  \
    ip_g = (guint8)(ip_diff * GREEN(ip_c1) + ip_rdiff * GREEN(ip_c2));  \
    ip_b = (guint8)(ip_diff * BLUE( ip_c1) + ip_rdiff * BLUE( ip_c2));  \
    /* results in ip_r, ip_g, ip_b */

extern int pal_indexes;


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

guint32 get_pixel_colour(double raw_scaled, gboolean palette_ip);

void palette_free(void);

#endif
