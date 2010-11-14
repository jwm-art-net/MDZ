#include "pal_rnd_dlg.h"
#include "main_gui.h"
#include "misc_gui.h"


static void channel(const char* label, GtkWidget* container,
                        GtkWidget** str_w, double str_v,
                        GtkWidget** bnd_w, double bnd_v)
{
    GtkWidget* table;
    GtkWidget* frame;
    int y = 0;

    frame = gtk_frame_new(label);
    gtk_container_add(GTK_CONTAINER(container), frame);

    /*  make the table 5 columns wide. the first column is for the
        labels, and the slider spans the remaining columns. i haven't
        found another way to get the proportions (of the label to the
        slider) right.
    */

    table = gtk_table_new(2, 5, FALSE);
    gtk_container_add(GTK_CONTAINER(frame), table);

    *str_w = gui_slider_new("Strength", str_v, 0.0, 1.0, 0.01,
                                                        table, y++, 4);
    *bnd_w = gui_slider_new("Bands",    bnd_v, 0.0, 1.0, 0.01,
                                                        table, y++, 4);
    gtk_widget_show(frame);
}


void palette_rnd_dlg_new(palette_rnd_dialog** ptr, image_info* img)
{
    GtkWidget* vbox;

    palette_rnd_dialog* dl;

    random_palette* rp = img->rnd_pal;

    dl = g_malloc(sizeof(palette_rnd_dialog));
    dl->img = img;
    *ptr = dl;

    dl->box = vbox = gtk_vbox_new(FALSE, 0);

    channel("Red",  dl->box,    &dl->r_strength, rp->r_strength,
                                &dl->r_bands,    rp->r_bands);

    channel("Green",dl->box,    &dl->g_strength, rp->g_strength,
                                &dl->g_bands,    rp->g_bands);

    channel("Blue", dl->box,    &dl->b_strength, rp->b_strength,
                                &dl->b_bands,    rp->b_bands);

    dl->pa = pal_affect_new(dl->box,    &dl->img->rnd_pal->offset,
                                        &dl->img->rnd_pal->stripe,
                                        &dl->img->rnd_pal->spread);
    gtk_widget_show_all(dl->box);
}


void palette_rnd_dlg_set(random_palette* rp, palette_rnd_dialog* dl)
{
    gtk_range_set_value(GTK_RANGE(dl->r_strength), rp->r_strength);
    gtk_range_set_value(GTK_RANGE(dl->g_strength), rp->g_strength);
    gtk_range_set_value(GTK_RANGE(dl->b_strength), rp->b_strength);
    gtk_range_set_value(GTK_RANGE(dl->r_bands), rp->r_bands);
    gtk_range_set_value(GTK_RANGE(dl->g_bands), rp->g_bands);
    gtk_range_set_value(GTK_RANGE(dl->b_bands), rp->b_bands);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->pa->offset),rp->offset);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->pa->stripe),rp->stripe);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->pa->spread),rp->spread);
}


void palette_rnd_dlg_apply(GtkWidget* widget, palette_rnd_dialog* dl)
{
    (void)widget;
    dl->img->rnd_pal->r_strength =
                        gtk_range_get_value(GTK_RANGE(dl->r_strength));
    dl->img->rnd_pal->g_strength =
                        gtk_range_get_value(GTK_RANGE(dl->g_strength));
    dl->img->rnd_pal->b_strength =
                        gtk_range_get_value(GTK_RANGE(dl->b_strength));
    dl->img->rnd_pal->r_bands =
                        gtk_range_get_value(GTK_RANGE(dl->r_bands));
    dl->img->rnd_pal->g_bands =
                        gtk_range_get_value(GTK_RANGE(dl->g_bands));
    dl->img->rnd_pal->b_bands =
                        gtk_range_get_value(GTK_RANGE(dl->b_bands));

    dl->img->rnd_pal->offset =
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->pa->offset));

    dl->img->rnd_pal->stripe =
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->pa->stripe));

    dl->img->rnd_pal->spread =
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->pa->spread));

    do_palette_randomize(dl->img->rnd_pal);

    palette_gui_rot_update(palgui);
}
