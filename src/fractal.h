#ifndef FRACTAL_H
#define FRACTAL_H

#include "image_info.h"

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

#endif
