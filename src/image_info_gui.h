#ifndef IMAGE_INFO_GUI_H
#define IMAGE_INFO_GUI_H

#include <gtk/gtk.h>

#include "image_info.h"


typedef struct
{
    GtkWidget* dialog;

    GtkWidget* width;
    GtkWidget* height;
    GtkWidget* aa;
    GtkWidget* text;
    GtkWidget* const_ra;
    GtkWidget* ok_button;
    GtkWidget* apply_button;

    GtkWidget* using_mpfr;
    GtkWidget* precision;
    GtkWidget* thread_count;

    GtkWidget* plabel;

    double ratio;

} image_info_dialog;

void image_info_dlg_new(image_info_dialog** ptr, image_info* img);

void image_info_dlg_set(image_info* img, image_info_dialog* dl);

void image_info_ok_cmd(GtkWidget* w,     image_info_dialog* dl);

void image_info_apply_cmd(GtkWidget* w,  image_info_dialog* dl);

void do_image_info_dialog(void);

void do_image_info_save_dialog(image_info* img);

int do_image_info_load_dialog(image_info* img);

void image_dlg_update_recommended_precision(image_info_dialog* dl,
                                            image_info* img);

#endif
