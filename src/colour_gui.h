#ifndef COLOUR_GUI_H
#define COLOUR_GUI_H

#include <gtk/gtk.h>

#include "image_info.h"

typedef struct
{
    GtkWidget* dialog;

    GtkWidget* scale;
    GtkWidget* scale_label;
    GtkWidget* palette_ip;

} colour_dialog;

void colour_dlg_new(colour_dialog** ptr, image_info* img);

void colour_dlg_set(image_info* img, colour_dialog* dl);

#endif
