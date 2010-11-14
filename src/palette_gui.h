#ifndef PALETTE_GUI_H
#define PALETTE_GUI_H

#include "colour_gui.h"
#include "image_info.h"
#include "palette.h"
#include "pal_fun_dlg.h"
#include "pal_rnd_dlg.h"
#include "pal_display_gui.h"

typedef struct
{
    GtkWidget* window;
    GtkWidget* notebook;

    /* tabs */

    palette_fun_dialog* pal_functions;
    palette_rnd_dialog* pal_randomize;

    GtkWidget* funcs_label;
    GtkWidget* randz_label;

    GtkWidget*  randomize;

    /* palette cycling */
    GtkWidget* cycle_fwd_button;
    GtkWidget* cycle_rev_button;
    GtkWidget* step_fwd_button;
    GtkWidget* step_rev_button;
    GtkWidget* stop_button;

    pal_display* pd;

    /* scale & interpolation */

    colour_dialog* cd;

    int cycler_id;
    int cycle_dir;

} palette_gui;


/* FIXEME: palette_gui_new should not need access to image_info
    it is only for the sake of the putrid code for the random
    palette gui that it is required...
*/

void palette_gui_new(palette_gui** pg_ptr, image_info* img);
void palette_gui_update(palette_gui*, image_info* img);
void palette_gui_rot_update(palette_gui*);
void palette_gui_cycle(palette_gui*, int dir);
void palette_gui_cycle_stop(palette_gui*);
#endif
