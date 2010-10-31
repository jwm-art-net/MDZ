#ifndef PAL_ROT_DLG_H
#define PAL_ROT_DLG_H

#include "image_info.h"
#include "palette.h"

typedef struct
{
    GtkWidget* window;
    GtkWidget* cycle_f;
    GtkWidget* cycle_b;
    GtkWidget* step_f;
    GtkWidget* step_b;
    GtkWidget* stop;
    image_info* img;
    int idle_id;
} palette_rot_dialog;

void palette_rot_dlg_new(palette_rot_dialog** ptr, image_info* img);

#endif
