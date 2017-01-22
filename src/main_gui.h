#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>

#include "coords_gui.h"
#include "image_info.h"
#include "palette_gui.h"

#define MIN_WINDOW_WIDTH    320

typedef enum
{
    STAT_NORMAL         = 0,

    STAT_FIRST_BUTTON,

    STAT_ZOOMING,
    STAT_ZOOMING_1CLICK,
    STAT_REPOSITIONING,

    STAT_LAST_BUTTON,

    STAT_JULIA_BROWSING
} stat_action;

typedef struct
{
    stat_action action;

    gboolean reposition_p2;

    /* zoom box info */
    int z_x;
    int z_y;
    int z_width;
    int z_height;
    int zoom_timer_id;

    /* repositioning info */
    int cl_x; /* center lines */
    int cl_y;
    int fp_x; /* first point */
    int fp_y;

    gboolean ptr_in_drawable;

} status_info;



extern image_info*      j_pre;
extern status_info      stat;
extern GtkWidget*       drawing_area;
extern GtkWidget*       window;
extern coords_dialog*   coords_dlg;
extern palette_gui*     palgui;


int gui_init(int* argc, char*** argv, image_info* img);

void gui_close_display(void);

void gui_resize(int width, int height);

void gui_start_rendering(image_info* img);

void gui_stop_rendering(image_info* img);

void gui_resize_preview(void);

gboolean gui_is_julia_browsing(void);

void gui_show_julia_preview(void);

void gui_hide_julia_preview(void);

GtkWindow* gui_window();

void gui_reapply_palette(void);

void gui_sensitive_switch_menu(gboolean t);
void gui_sensitive_reposition_button(gboolean t);

void gui_draw_center_lines(int draw_cl);

GtkWidget* gui_create_pixmap(GtkWidget* widget, char** xpm_data);

gint do_palette_rotation(gpointer dir);
void do_palette_randomize(random_palette* rnd_pal);
void do_palette_function(function_palette* fun_pal);


#endif
