#include "zoom.h"

#include "debug.h"
#include "main.h"
#include "main_gui.h"
#include "reposition.h"

#include "../pixmaps/zoom_in.xpm"
#include "../pixmaps/zoom_in_1click.xpm"
#include "../pixmaps/zoom_out.xpm"

#define ZOOM_INTERVAL   25
#define ZOOM_BOX_WIDTH  .35

static GtkWidget* zoom_in_button =          NULL;
static GtkWidget* zoom_in_1click_button =   NULL;
static GtkWidget* zoom_out_button =         NULL;

static void zoom_in_func(GtkWidget* widget);
static void zoom_in_1click_func(GtkWidget* widget);
static void zoom_out_func(GtkWidget* widget);
static void start_zoom_1click(void);
static void stop_zoom_1click(void);
static void start_zooming(void);
static void stop_zooming(void);

static void zoom_resize(int arg);
static int  zoom_is_valid_size(void);

void zoom_gui_sensitive_zoom_in_button(gboolean t)
    {   gtk_widget_set_sensitive(zoom_in_button, t);    }

void zoom_gui_sensitive_zoom_out_button(gboolean t)
    {   gtk_widget_set_sensitive(zoom_out_button, t);    }

void zoom_gui_sensitive_zoom_in_1click_button(gboolean t)
    {   gtk_widget_set_sensitive(zoom_in_1click_button, t);    }


void zoom_gui_create_buttons(GtkBox* box)
{
    /* zoom in */
    zoom_in_button = gtk_toggle_button_new();
    gtk_container_add(GTK_CONTAINER(zoom_in_button),
                      gui_create_pixmap(window, zoom_in_xpm));
    gtk_widget_set_tooltip_text(zoom_in_button,
        "box zooming, middle mouse button to zoom");
    g_signal_connect(GTK_OBJECT(zoom_in_button),    "toggled",
                     G_CALLBACK(zoom_in_func),      NULL);

    gtk_box_pack_start(box, zoom_in_button, FALSE, FALSE, 0);
    gtk_button_set_relief(GTK_BUTTON(zoom_in_button), GTK_RELIEF_NONE);
    GTK_WIDGET_UNSET_FLAGS(zoom_in_button, GTK_CAN_FOCUS);

    /* zoom in 1 click */
    zoom_in_1click_button = gtk_toggle_button_new();
    gtk_container_add(GTK_CONTAINER(zoom_in_1click_button),
                      gui_create_pixmap(window, zoom_in_1click_xpm));
    gtk_widget_set_tooltip_text(zoom_in_1click_button,
        "quick 1 click zooming, left mouse button to zoom");

    g_signal_connect(GTK_OBJECT(zoom_in_1click_button), "toggled",
                     G_CALLBACK(zoom_in_1click_func), NULL);

    gtk_box_pack_start(box, zoom_in_1click_button,
                                      FALSE, FALSE, 0);

    gtk_button_set_relief(GTK_BUTTON(zoom_in_1click_button),
                          GTK_RELIEF_NONE);

    GTK_WIDGET_UNSET_FLAGS(zoom_in_1click_button, GTK_CAN_FOCUS);

    /* zoom out */
    zoom_out_button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(zoom_out_button),
                      gui_create_pixmap(window, zoom_out_xpm));
    gtk_widget_set_tooltip_text(zoom_out_button,
        "click this to zoom out");

    g_signal_connect(GTK_OBJECT(zoom_out_button), "clicked",
                     G_CALLBACK(zoom_out_func), NULL);

    gtk_box_pack_start(box, zoom_out_button, FALSE, FALSE, 0);
    gtk_button_set_relief(GTK_BUTTON(zoom_out_button), GTK_RELIEF_NONE);
    GTK_WIDGET_UNSET_FLAGS(zoom_out_button, GTK_CAN_FOCUS);

    gtk_widget_show(zoom_in_button);
    gtk_widget_show(zoom_in_1click_button);
    gtk_widget_show(zoom_out_button);
}

static void zoom_in_func(GtkWidget* widget)
{
    if (GTK_TOGGLE_BUTTON(widget)->active)
        start_zooming();
    else
        stop_zooming();
}

void zoom_in_1click_func(GtkWidget* widget)
{
    if (GTK_TOGGLE_BUTTON(widget)->active)
        start_zoom_1click();
    else
        stop_zoom_1click();
}

void zoom_out_func(GtkWidget* widget)
{
    (void)widget;
    coords_zoom(img->pcoords,   2);
    gui_start_rendering(img);
}


void start_zoom_1click(void)
{
    stat.action = STAT_ZOOMING_1CLICK;
    stat.z_width = 16;
    stat.z_height = stat.z_width / img->aspect;
    stat.reposition_p2 = FALSE;

    zoom_gui_sensitive_zoom_in_button(FALSE);
    gui_sensitive_switch_menu(FALSE);
    repos_gui_sensitive_reposition_button(FALSE);
}

void stop_zoom_1click(void)
{
    stat.action = STAT_NORMAL;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(zoom_in_1click_button),
                                FALSE);
    zoom_gui_sensitive_zoom_in_button(TRUE);
    gui_sensitive_switch_menu(TRUE);
    repos_gui_sensitive_reposition_button(TRUE);
}

void start_zooming(void)
{
    stat.action = STAT_ZOOMING;
    stat.z_x = 0;
    stat.z_y = 0;
    stat.z_width = ZOOM_BOX_WIDTH * img->user_width;

    stat.z_height = stat.z_width / img->aspect;

    zoom_gui_sensitive_zoom_in_1click_button(FALSE);
    gui_sensitive_switch_menu(FALSE);
    repos_gui_sensitive_reposition_button(FALSE);

    gdk_gc_set_line_attributes(drawing_area->style->white_gc,
            1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
}

void stop_zooming(void)
{
    stat.action = STAT_NORMAL;
    zoom_gui_kill_timers();
    zoom_gui_sensitive_zoom_in_1click_button(TRUE);
    gui_sensitive_switch_menu(TRUE);
    repos_gui_sensitive_reposition_button(TRUE);
}

void zoom_in(void)
{
    coords* c = (img->zoom_new_win) ? coords_dup(img->pcoords) : 0;

    if (stat.action == STAT_ZOOMING)
        stat.action = STAT_NORMAL;

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(zoom_in_button),
                                FALSE);

    coords_zoom_to(img->pcoords, stat.z_x, stat.z_y, stat.z_width);

    if (img->zoom_new_win == TRUE)
    {
        duplicate();
        coords_cpy(img->pcoords, c);
        coords_free(c);
    }
    else
    {
        gui_start_rendering(img);
    }
}

void zoom_resize(int arg)
{
    stat.z_width += arg * 4;
    stat.z_height = stat.z_width / img->aspect;
}

int zoom_is_valid_size(void)
{
    if ((stat.z_width < 4) || (stat.z_width  > (img->user_width - 16))
    || (stat.z_height < 4) || (stat.z_height > (img->user_height - 16)))
        return FALSE;
    else
        return TRUE;
}

gint zoom_callback(int arg)
{
    zoom_gui_draw_zoom_box();

    zoom_resize(arg);
    if (!zoom_is_valid_size())
        zoom_resize(-1*arg);

    zoom_gui_draw_zoom_box();

    return TRUE;
}


void zoom_gui_process_zooming_click(int button)
{
    zoom_gui_draw_zoom_box();
    if (button == 1)
    {
        zoom_resize(1);
        if (!zoom_is_valid_size())
            zoom_resize(-1);
        else
            stat.zoom_timer_id = g_timeout_add(ZOOM_INTERVAL,
                                       (GtkFunction)zoom_callback,
                                       (gpointer)2);
    }
    else if (button == 2)
        zoom_in();
    else if (button == 3)
    {
        zoom_resize(-1);
        if (!zoom_is_valid_size())
            zoom_resize(1);
        else
            stat.zoom_timer_id = g_timeout_add(ZOOM_INTERVAL,
                                       (GtkFunction)zoom_callback,
                                       (gpointer)-2);
    }
    zoom_gui_draw_zoom_box();
}

void zoom_gui_process_zooming_1click_click(int button)
{
    if (!img->zoom_new_win)
        zoom_gui_draw_zoom_box();
    if (button == 1 || button == 2)
        zoom_in();
    else if (button == 3)
        stop_zoom_1click();
}

void zoom_gui_kill_timers(void)
{
    if (stat.zoom_timer_id != -1)
        g_source_remove(stat.zoom_timer_id);
    stat.zoom_timer_id = -1;
}

void zoom_gui_draw_zoom_box(void)
{
    gdk_gc_set_function(drawing_area->style->white_gc, GDK_XOR);

    gdk_draw_rectangle(
        drawing_area->window, drawing_area->style->white_gc,
        FALSE,  stat.z_x - (stat.z_width >> 1),
                stat.z_y - (stat.z_height >> 1),
                stat.z_width, stat.z_height);
    
    gdk_gc_set_function(drawing_area->style->white_gc, GDK_COPY);
}
