#ifndef PAL_RND_DLG_H
#define PAL_RND_DLG_H

#include "image_info.h"
#include "palette.h"
#include "main.h"

typedef struct {
    GtkWidget* window;
    GtkWidget* r_strength;
    GtkWidget* g_strength;
    GtkWidget* b_strength;
    GtkWidget* r_bands;
    GtkWidget* g_bands;
    GtkWidget* b_bands;
    GtkWidget* offset;
    GtkWidget* stripe;
    GtkWidget* spread;
    GtkWidget* apply_button;
    GtkWidget* dismiss_button;
    image_info* img;
} palette_rnd_dialog;

void palette_rnd_dlg_new(palette_rnd_dialog** ptr, image_info* img);
void palette_rnd_dlg_set(random_palette* rp, palette_rnd_dialog* dl);

#endif
