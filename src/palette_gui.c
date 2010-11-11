#include "palette_gui.h"


#include <gdk/gdkkeysyms.h>
#include <math.h> /* for round */


#include "main_gui.h"
#include "main.h"

/* callbacks */
static void cycle_forward_cb(   GtkWidget*, gpointer data);
static void cycle_backward_cb(  GtkWidget*, gpointer data);
static void step_forward_cb(    GtkWidget*, gpointer data);
static void step_backward_cb(   GtkWidget*, gpointer data);
static void cycle_stop_cb(      GtkWidget*, gpointer data);


static void palette_gui_destroy(GtkWidget*, palette_gui*);


static gint key_event(GtkWidget* widget, GdkEventKey* event,
                                            gpointer data)
{
    (void)widget;
    palette_gui* pg = (palette_gui*)data;

    switch (event->keyval)
    {
    case GDK_plus:
    case GDK_KP_Add:
        step_forward_cb(NULL, pg);
        break;

    case GDK_minus:
    case GDK_KP_Subtract:
        step_backward_cb(NULL, pg);
        break;

    case GDK_f:
    case GDK_F:
        palette_gui_cycle(pg, 1);
        break;

    case GDK_b:
    case GDK_B:
        palette_gui_cycle(pg, -1);
        break;

    case GDK_s:
    case GDK_S:
        palette_gui_cycle_stop(pg);
        break;

    default: /* let someone else deal with it... */
        return FALSE;
    }

    return TRUE;
}


void cycle_forward_cb(GtkWidget* widget, gpointer data)
{
    (void)widget;
    palette_gui_cycle((palette_gui*)data, 1);
}


void cycle_backward_cb(GtkWidget* widget, gpointer data)
{
    (void)widget;
    palette_gui_cycle((palette_gui*)data, -1);
}


void palette_gui_cycle(palette_gui* pg, int dir)
{
    gboolean sen = dir > 0 ? TRUE : FALSE;

    pg->cycle_dir = dir;

    gtk_widget_set_sensitive(pg->stop_button, TRUE);

    gtk_widget_set_sensitive(pg->cycle_rev_button,  sen);
    gtk_widget_set_sensitive(pg->cycle_fwd_button,  !sen);
    gtk_widget_set_sensitive(pg->step_rev_button,   sen);
    gtk_widget_set_sensitive(pg->step_fwd_button,   !sen);

    if (pg->cycler_id == -1)
    {
        /* note: use of (GtkFunction) to cast, NOT G_CALLBACK() */
        pg->cycler_id =
            g_timeout_add(30, (GtkFunction)do_palette_rotation,
                                        &pg->cycle_dir);
    }
}

void palette_gui_cycle_stop(palette_gui* pg)
{
    if (pg->cycler_id != -1)
    {
        g_source_remove(pg->cycler_id);
        pg->cycler_id = -1;
    }

    gtk_widget_set_sensitive(pg->stop_button, FALSE);

    gtk_widget_set_sensitive(pg->step_fwd_button,   TRUE);
    gtk_widget_set_sensitive(pg->cycle_fwd_button,  TRUE);
    gtk_widget_set_sensitive(pg->step_rev_button,   TRUE);
    gtk_widget_set_sensitive(pg->cycle_rev_button,  TRUE);

    pg->cycle_dir = 0;
}

void cycle_stop_cb(GtkWidget* widget, gpointer data)
{
    (void)widget;
    palette_gui_cycle_stop((palette_gui*)data);
}

void step_forward_cb(GtkWidget* widget, gpointer data)
{
    (void)widget;
    palette_gui* pg = (palette_gui*)data;
    pg->cycle_dir = 1;
    do_palette_rotation(&pg->cycle_dir);
}

void step_backward_cb(GtkWidget* widget, gpointer data)
{
    (void)widget;
    palette_gui* pg = (palette_gui*)data;
    pg->cycle_dir = -1;
    do_palette_rotation(&pg->cycle_dir);
}


void palette_gui_rot_update(palette_gui* pg)
{
    pal_display_update(pg->pd);
}


void palette_gui_update(palette_gui* pg, image_info* img)
{
    pal_display_update(pg->pd);
    colour_dlg_set(img, pg->cd);
}


void palette_gui_new(palette_gui** ptr, image_info* img)
{
    GtkWidget* tmp;
    GtkWidget* vbox;
    GtkWidget* hbox;
    palette_gui* pg;

    pg = malloc(sizeof(*pg));

    pg->cycler_id = -1;
    *ptr = pg;

    tmp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(GTK_OBJECT(tmp), "destroy",
                       G_CALLBACK(palette_gui_destroy),
                       pg);

    g_signal_connect(GTK_OBJECT(tmp), "destroy",
                       G_CALLBACK(gtk_widget_destroyed),
                       ptr);

    gtk_window_set_title(GTK_WINDOW(tmp), "Colours");
    gtk_window_set_resizable(GTK_WINDOW(tmp), FALSE);
    g_signal_connect(GTK_OBJECT(tmp), "key_press_event",
                       G_CALLBACK(key_event), pg);
    pg->window = tmp;

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(pg->window), vbox);

    palette_fun_dlg_new(&pg->pal_functions, &fun_palette);
    palette_rnd_dlg_new(&pg->pal_randomize, img);

    tmp = gtk_notebook_new();
    pg->notebook = tmp;

    tmp = gtk_label_new("Randomization");
    gtk_notebook_append_page(GTK_NOTEBOOK(pg->notebook),
                            pg->pal_randomize->box, tmp);

    tmp = gtk_label_new("Functions");
    gtk_notebook_append_page(GTK_NOTEBOOK(pg->notebook),
                            pg->pal_functions->box, tmp);

    gtk_box_pack_start(GTK_BOX(vbox), pg->notebook, FALSE, FALSE, 0);


    /* colour cycling */

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    /* step reverse */
    tmp = gtk_button_new_with_label("<");
    g_signal_connect(GTK_OBJECT(tmp), "clicked",
                        G_CALLBACK(step_backward_cb),
                            pg);
    gtk_box_pack_start(GTK_BOX(hbox), tmp, TRUE, FALSE, 0);
    pg->step_rev_button = tmp;

    /* cycle reverse */
    tmp = gtk_button_new_with_label("<<");
    g_signal_connect(GTK_OBJECT(tmp), "clicked",
                        G_CALLBACK(cycle_backward_cb),
                            pg);
    gtk_box_pack_start(GTK_BOX(hbox), tmp, TRUE, FALSE, 0);
    pg->cycle_rev_button = tmp;

    /* cycle stop */
    tmp = gtk_button_new_with_label("Stop");
    g_signal_connect(GTK_OBJECT(tmp), "clicked",
                        G_CALLBACK(cycle_stop_cb),
                            pg);
    gtk_box_pack_start(GTK_BOX(hbox), tmp, TRUE, FALSE, 4);
    gtk_widget_set_sensitive(tmp, FALSE);
    pg->stop_button = tmp;

    /* cycle forward */
    tmp = gtk_button_new_with_label(">>");
    g_signal_connect(GTK_OBJECT(tmp), "clicked",
                        G_CALLBACK(cycle_forward_cb),
                            pg);
    gtk_box_pack_start(GTK_BOX(hbox), tmp, TRUE, FALSE, 0);
    pg->cycle_fwd_button = tmp;

    /* step forward */
    tmp = gtk_button_new_with_label(">");
    g_signal_connect(GTK_OBJECT(tmp), "clicked",
                        G_CALLBACK(step_forward_cb),
                            pg);
    gtk_box_pack_start(GTK_BOX(hbox), tmp, TRUE, FALSE, 0);
    pg->step_fwd_button = tmp;

    pg->pd = pal_display_new(vbox);

    pal_display_update(pg->pd);

    /* colour scaling and interpolation */
    pg->cd = colour_dlg_new(img);
    gtk_box_pack_start(GTK_BOX(vbox), pg->cd->box, TRUE, FALSE, 0);

    tmp = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), tmp, TRUE, FALSE, 0);

    gtk_widget_show_all(pg->window);
}

static void palette_gui_destroy(GtkWidget* widget, palette_gui* pg)
{
    (void)widget;
    palette_gui_cycle_stop(pg);
    pal_display_free(pg->pd);
    colour_dlg_free(pg->cd);
    free(pg);
}
