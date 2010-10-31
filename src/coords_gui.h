#ifndef COORDS_GUI_H
#define COORDS_GUI_H

#include <gtk/gtk.h>

#include "coords.h"
#include "image_info.h"


typedef struct
{
    GtkWidget* dialog;

    GtkWidget*  xmin;
    GtkWidget*  xmax;
    GtkWidget*  ymax;

    GtkWidget*  cx;
    GtkWidget*  cy;
    GtkWidget*  size;

    GtkWidget* ok_button;
    GtkWidget* apply_button;
    GtkWidget* refresh_button;

    coords* pcoords;

    image_info* img;

    int entry_chars;

    bool center;

    guint xmin_ins, xmax_ins, ymax_ins, cx_ins, cy_ins, size_ins;
    guint xmin_del, xmax_del, ymax_del, cx_del, cy_del, size_del;

} coords_dialog;


void coords_dlg_new(coords_dialog** ptr, image_info* img);

void coords_dlg_set(image_info* img, coords_dialog* dl);

void coords_ok_cmd(GtkWidget* w, coords_dialog* dl);

void coords_apply_cmd(GtkWidget* w, coords_dialog* dl);

#endif
