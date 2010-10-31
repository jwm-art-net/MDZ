#ifndef RENDER_H
#define RENDER_H

#include "image_info.h"


int render_to_file(image_info* img); /* 0 fail, 1 success */

void do_anti_aliasing(image_info* img,
                      int x0, int y0, int width, int height);


#endif
