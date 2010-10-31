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

gint do_palette_rotation(gpointer dir);
void do_palette_randomize(random_palette* rnd_pal);
void do_palette_function(function_palette* fun_pal);

#endif /* __MAIN_H */
