#ifndef COLOUR_GUI_H
#define COLOUR_GUI_H

#include <gtk/gtk.h>

#include "image_info.h"

typedef struct
{
    GtkWidget* box;
    GtkWidget* scale;
    GtkWidget* palette_ip;

} colour_dialog;

colour_dialog*  colour_dlg_new(image_info* img);
void            colour_dlg_free(colour_dialog*);

void            colour_dlg_set(image_info* img, colour_dialog* dl);
#endif
