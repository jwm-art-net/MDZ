#ifndef MAIN_H
#define MAIN_H

#include "palette.h"
#include "cmdline.h"

extern char*            program_name;
extern options          opts;
extern random_palette   rnd_palette;
extern function_palette fun_palette;
extern image_info*      img;

void duplicate(void);


#endif /* __MAIN_H */
