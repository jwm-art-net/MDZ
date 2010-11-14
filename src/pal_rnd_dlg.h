#ifndef PAL_RND_DLG_H
#define PAL_RND_DLG_H

#include "image_info.h"
#include "palette.h"
#include "pal_display_gui.h"


typedef struct
{
    GtkWidget* box;

    GtkWidget* r_strength;
    GtkWidget* g_strength;
    GtkWidget* b_strength;
    GtkWidget* r_bands;
    GtkWidget* g_bands;
    GtkWidget* b_bands;

    pal_affect* pa;

    image_info* img;

} palette_rnd_dialog;


/* FIXME: get rid of the access to image_info - it's some ballsed up
    way of fffffffarhhh..  (the random palette data is stored there
    which is required even without the palette randomization dialog
    open due to high possiblity of colour maps (ie palettes) not
    being available.
*/

void palette_rnd_dlg_new(palette_rnd_dialog** ptr, image_info* img);
void palette_rnd_dlg_set(random_palette* rp, palette_rnd_dialog* dl);


void palette_rnd_dlg_apply(GtkWidget* widget, palette_rnd_dialog* dl);

#endif
