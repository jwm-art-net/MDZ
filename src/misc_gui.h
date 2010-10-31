#ifndef MISC_GUI_H
#define MISC_GUI_H


#include <gtk/gtk.h>

void file_chooser_add_filter(GtkWidget* chooser,    const char* name,
                                                    const char* pattern);


#endif
