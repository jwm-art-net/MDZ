#include "reposition.h"

#include "coords.h"
#include "main.h"
#include "main_gui.h"
#include "zoom.h"

#include "../pixmaps/reposition.xpm"

static GtkWidget* reposition_button =       NULL;

static void start_reposition(void);
static void stop_reposition(void);
static void reposition_func(GtkWidget* widget);

void repos_gui_create_buttons(GtkBox* box)
{
    /* reposition */
    reposition_button = gtk_toggle_button_new();
    gtk_container_add(GTK_CONTAINER(reposition_button),
                      gui_create_pixmap(window, reposition_xpm));
    gtk_widget_set_tooltip_text(reposition_button,
        "left click to reposition, middle click to center");
    g_signal_connect(GTK_OBJECT(reposition_button), "clicked",
                       G_CALLBACK(reposition_func), NULL);
    gtk_box_pack_start(box, reposition_button, FALSE, FALSE,
                       0);
    gtk_button_set_relief(GTK_BUTTON(reposition_button),
                          GTK_RELIEF_NONE);
    GTK_WIDGET_UNSET_FLAGS(reposition_button, GTK_CAN_FOCUS);
    gtk_widget_show(reposition_button);
}


void repos_gui_sensitive_reposition_button(gboolean t)
    {   gtk_widget_set_sensitive(reposition_button, t);  }

void start_reposition(void)
{
    stat.action = STAT_REPOSITIONING;
    stat.reposition_p2 = FALSE;

    gui_sensitive_switch_menu(FALSE);
    zoom_gui_sensitive_zoom_in_button(FALSE);
    zoom_gui_sensitive_zoom_out_button(FALSE);
    zoom_gui_sensitive_zoom_in_1click_button(FALSE);
}

void stop_reposition(void)
{
    stat.action = STAT_NORMAL;

    stat.reposition_p2 = FALSE;

    gui_sensitive_switch_menu(TRUE);
    zoom_gui_sensitive_zoom_in_button(TRUE);
    zoom_gui_sensitive_zoom_out_button(TRUE);
    zoom_gui_sensitive_zoom_in_1click_button(TRUE);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(reposition_button),
                                FALSE);
}


void reposition_func(GtkWidget* widget)
{
    if (GTK_TOGGLE_BUTTON(widget)->active)
        start_reposition();
    else
        stop_reposition();
}


void repos_gui_process_repositioning_click(GdkEventButton* event)
{
    if (event->button == 1)
    {
        /* repositon point 1 to point 2 */

        if (!stat.reposition_p2)
        {
            stat.reposition_p2 = TRUE;
            gui_draw_center_lines(TRUE);
            stat.fp_x = event->x;
            stat.fp_y = event->y;
        }
        else
        {
            coords_reposition(img->pcoords, stat.cl_x, stat.cl_y,
                                            stat.fp_x, stat.fp_y);
            gui_draw_center_lines(TRUE);
            gui_draw_center_lines(FALSE);
            stop_reposition();
            gui_draw_center_lines(TRUE);
            gui_start_rendering(img);
        }
    }
    else if (event->button == 2)
    {
        /* center this exact point */

        coords_center_to(img->pcoords, stat.cl_x, stat.cl_y);

        if (stat.reposition_p2)
        {
            gui_draw_center_lines(FALSE);
            gui_draw_center_lines(TRUE);
            stat.reposition_p2 = FALSE;
            gui_draw_center_lines(TRUE);
        }
        stop_reposition();
        gui_start_rendering(img);
    }
    else if (event->button == 3)
    {
        /* cancel reposition */

        gui_draw_center_lines(TRUE);
        if (stat.reposition_p2)
        {
            gui_draw_center_lines(FALSE);
            gui_draw_center_lines(TRUE);
            stat.reposition_p2 = FALSE;
            gui_draw_center_lines(TRUE);
        }
        stop_reposition();
    }
}
