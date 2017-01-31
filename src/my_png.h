#ifndef MY_PNG_H
#define MY_PNG_H

#include "image_info.h"

void do_png_save(image_info* img);

void save_png_file(image_info* img, char* filename);


const char* my_png_get_last_used_dir(void);
const char* my_png_get_last_used_filename(void);
void my_png_reset_last_used_filename(void);
void my_png_cleanup(void);

#endif /* __MY_PNG_H */
