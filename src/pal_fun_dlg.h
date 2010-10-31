#ifndef __PAL_FUN_DLG_H
#define __PAL_FUN_DLG_H

#include "image_info.h"
#include "palette.h"
#include "main.h"

typedef struct {
    GtkWidget* window;
    GtkWidget* ex_rg_button;
    GtkWidget* ex_gb_button;
    GtkWidget* ex_br_button;
    GtkWidget* rot_rgb_button;
    GtkWidget* inv_rgb_button;
    GtkWidget* inv_r_button;
    GtkWidget* inv_g_button;
    GtkWidget* inv_b_button;
    GtkWidget* close_button;
    GtkWidget* dismiss_button;
    GtkWidget* offset;
    GtkWidget* stripe;
    GtkWidget* spread;
    function_palette* fun_pal;
} palette_fun_dialog;

void palette_fun_dlg_new(palette_fun_dialog** ptr, function_palette* fp);

#endif
