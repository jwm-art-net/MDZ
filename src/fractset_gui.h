#ifndef FRACSET_DLG_H
#define FRACSET_DLG_H

#include <gtk/gtk.h>


#include "image_info.h"


typedef struct
{
    GtkWidget* dialog;

    GtkWidget* fractal;

    GtkWidget* ok_button;
    GtkWidget* apply_button;

    image_info* img;

} fracset_dialog;


void fracset_dlg_new(       fracset_dialog**,   image_info*);
void fracset_dlg_set(       fracset_dialog*,    image_info*);
void get_fractal_settings(  fracset_dialog*,    image_info*);

void fracset_dlg_ok_cmd(    GtkWidget*,     fracset_dialog*);
void fracset_dlg_apply_cmd( GtkWidget*,     fracset_dialog*);

#endif
