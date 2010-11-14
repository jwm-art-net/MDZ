#ifndef FRACTAL_H
#define FRACTAL_H

#include "image_info.h"



enum
{
    FAMILY_MANDEL,
    FAMILY_JULIA,
/* ----- leave last ------ */
    FAMILY_NULL,
    FAMILY_XXX_LAST
};

enum
{
    MANDELBROT,
    BURNING_SHIP,
    GENERALIZED_CELTIC,
    VARIANT,
/* ----- leave last ------ */
    FRACTAL_NULL,
    FRACTAL_XXX_LAST
};

extern const char* family_str[FAMILY_XXX_LAST];
extern const char* fractal_str[FRACTAL_XXX_LAST];

/* in both instances, line is the line in the image to be rendered,
    the return value is 1 if rendering should continue,
    or 0 if rendering has been stopped.

    a return value of 0 is obtained by a call to the render threads
    system to determine if it has been stopped.

    the purpose is to allow line rendering to be aborted before
    the line has finished (but without checking so at every pixel)
*/

int fractal_calculate_line(image_info* img, int line);
int fractal_mpfr_calculate_line(image_info* img, int line);
int fractal_gmp_calculate_line(image_info* img, int line);



#endif
