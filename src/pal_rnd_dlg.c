#include "pal_rnd_dlg.h"
#include "main.h"

static void set_rnd_palette(GtkWidget* widget, palette_rnd_dialog* dl);
static void pal_rnd_destroy(GtkWidget* widget, palette_rnd_dialog* dl);

static void affect_update(GtkWidget* w, palette_rnd_dialog* dl);

void palette_rnd_dlg_new(palette_rnd_dialog** ptr, image_info* img)
{
    GtkWidget* tmp;
    GtkWidget* vbox;
    GtkWidget* r_frame;
    GtkWidget* g_frame;
    GtkWidget* b_frame;
    GtkWidget* r_table;
    GtkWidget* g_table;
    GtkWidget* b_table;
    GtkWidget* af_frame;
    GtkWidget* af_table;
    GtkObject* adj;
    palette_rnd_dialog* dl;

    dl = g_malloc(sizeof(palette_rnd_dialog));
    dl->img = img;
    *ptr = dl;

    dl->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(GTK_OBJECT(dl->window), "destroy",
                       G_CALLBACK(pal_rnd_destroy),
                       dl);
    g_signal_connect(GTK_OBJECT(dl->window), "destroy",
                       G_CALLBACK(gtk_widget_destroyed),
                       ptr);
    gtk_window_set_title(GTK_WINDOW(dl->window), "Palette Randomization");
    gtk_window_set_position(GTK_WINDOW(dl->window), GTK_WIN_POS_MOUSE);
    gtk_window_set_resizable(GTK_WINDOW(dl->window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(dl->window), 10);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(dl->window), vbox);
    gtk_widget_show(vbox);

    r_frame = gtk_frame_new("Red");
    g_frame = gtk_frame_new("Green");
    b_frame = gtk_frame_new("Blue");
    gtk_container_set_border_width(GTK_CONTAINER(r_frame),4);
    gtk_container_set_border_width(GTK_CONTAINER(g_frame),4);
    gtk_container_set_border_width(GTK_CONTAINER(b_frame),4);
    gtk_container_add(GTK_CONTAINER(vbox), r_frame);
    gtk_container_add(GTK_CONTAINER(vbox), g_frame);
    gtk_container_add(GTK_CONTAINER(vbox), b_frame);

    r_table = gtk_table_new(2, 4, TRUE);
    tmp = gtk_label_new("Strength:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.9, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(r_table), tmp, 0, 1, 0, 1);
    gtk_widget_show(tmp);
    dl->r_strength = gtk_hscale_new_with_range(0.0, 1.0, 0.01);
    gtk_range_set_value(GTK_RANGE(dl->r_strength),
                                  dl->img->rnd_pal->r_strength);
    gtk_table_attach_defaults(GTK_TABLE(r_table), dl->r_strength,
                                                  1, 4, 0, 1);
    gtk_widget_show(dl->r_strength);
    tmp = gtk_label_new("Bands:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.9, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(r_table), tmp, 0, 1, 1, 2);
    gtk_widget_show(tmp);
    dl->r_bands = gtk_hscale_new_with_range(0.0, 1.0, 0.01);
    gtk_range_set_value(GTK_RANGE(dl->r_bands),
                                  dl->img->rnd_pal->r_bands);
    gtk_table_attach_defaults(GTK_TABLE(r_table), dl->r_bands,
                                                  1, 4, 1, 2);
    gtk_widget_show(dl->r_bands);
    gtk_container_add(GTK_CONTAINER(r_frame), r_table);
    gtk_widget_show(r_table);
    gtk_widget_show(r_frame);

    g_table = gtk_table_new(2, 4, TRUE);
    tmp = gtk_label_new("Strength:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.9, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(g_table), tmp, 0, 1, 0, 1);
    gtk_widget_show(tmp);
    dl->g_strength = gtk_hscale_new_with_range(0.0, 1.0, 0.01);
    gtk_range_set_value(GTK_RANGE(dl->g_strength),
                                  dl->img->rnd_pal->g_strength);
    gtk_table_attach_defaults(GTK_TABLE(g_table), dl->g_strength,
                                                  1, 4, 0, 1);
    gtk_widget_show(dl->g_strength);
    tmp = gtk_label_new("Bands:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.9, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(g_table), tmp, 0, 1, 1, 2);
    gtk_widget_show(tmp);
    dl->g_bands = gtk_hscale_new_with_range(0.0, 1.0, 0.01);
    gtk_range_set_value(GTK_RANGE(dl->g_bands),
                                  dl->img->rnd_pal->g_bands);
    gtk_table_attach_defaults(GTK_TABLE(g_table), dl->g_bands,
                                                  1, 4, 1, 2);
    gtk_widget_show(dl->g_bands);
    gtk_container_add(GTK_CONTAINER(g_frame), g_table);
    gtk_widget_show(g_table);
    gtk_widget_show(g_frame);

    b_table = gtk_table_new(2, 4, TRUE);
    tmp = gtk_label_new("Strength:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.9, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(b_table), tmp, 0, 1, 0, 1);
    gtk_widget_show(tmp);
    dl->b_strength = gtk_hscale_new_with_range(0.0, 1.0, 0.01);
    gtk_range_set_value(GTK_RANGE(dl->b_strength),
                                  dl->img->rnd_pal->b_strength);
    gtk_table_attach_defaults(GTK_TABLE(b_table), dl->b_strength,
                                                  1, 4, 0, 1);
    gtk_widget_show(dl->b_strength);
    tmp = gtk_label_new("Bands:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.9, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(b_table), tmp, 0, 1, 1, 2);
    gtk_widget_show(tmp);
    dl->b_bands = gtk_hscale_new_with_range(0.0, 1.0, 0.01);
    gtk_range_set_value(GTK_RANGE(dl->b_bands),
                                  dl->img->rnd_pal->b_bands);
    gtk_table_attach_defaults(GTK_TABLE(b_table), dl->b_bands,
                                                  1, 4, 1, 2);
    gtk_widget_show(dl->b_bands);
    gtk_container_add(GTK_CONTAINER(b_frame), b_table);
    gtk_widget_show(b_table);
    gtk_widget_show(b_frame);

    af_frame = gtk_frame_new("Affect");
    gtk_container_set_border_width(GTK_CONTAINER(af_frame),4);
    gtk_container_add(GTK_CONTAINER(vbox), af_frame);

    af_table = gtk_table_new(3, 2, TRUE);

    tmp = gtk_label_new("Offset:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(af_table), tmp, 0, 1, 0, 1);
    gtk_widget_show(tmp);

    adj = gtk_adjustment_new(dl->img->rnd_pal->offset, 0, 255, 1, 1, 0);
    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(af_table), tmp, 1, 2, 0, 1);
    gtk_widget_show(tmp);
    dl->offset = tmp;

    tmp = gtk_label_new("Stripe:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(af_table), tmp, 0, 1, 1, 2);
    gtk_widget_show(tmp);

    adj = gtk_adjustment_new(dl->img->rnd_pal->stripe, 1, 128, 1, 1, 0);
    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);

    gtk_table_attach_defaults(GTK_TABLE(af_table), tmp, 1, 2, 1, 2);
    g_signal_connect(GTK_OBJECT(tmp), "changed",
                       G_CALLBACK(affect_update),
                       dl);
    gtk_widget_show(tmp);
    dl->stripe = tmp;

    tmp = gtk_label_new("Spread:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(af_table), tmp, 0, 1, 2, 3);
    gtk_widget_show(tmp);

    adj = gtk_adjustment_new(dl->img->rnd_pal->spread, 1, 128, 1, 1, 0);
    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(af_table), tmp, 1, 2, 2, 3);
    g_signal_connect(GTK_OBJECT(tmp), "changed",
                       G_CALLBACK(affect_update),
                       dl);
    gtk_widget_show(tmp);
    dl->spread = tmp;

    gtk_container_add(GTK_CONTAINER(af_frame), af_table);

    dl->apply_button = gtk_button_new_with_label("Apply");
    g_signal_connect(GTK_OBJECT(dl->apply_button), "clicked",
                     G_CALLBACK(set_rnd_palette),
                     dl);
    gtk_box_pack_start(GTK_BOX(vbox), dl->apply_button, TRUE, TRUE, 0);
    gtk_widget_show(dl->apply_button);

    dl->dismiss_button = gtk_button_new_with_label("Dismiss");
    g_signal_connect_object(GTK_OBJECT(dl->dismiss_button), "clicked",
                            G_CALLBACK(gtk_widget_destroy),
                            GTK_OBJECT(dl->window), G_CONNECT_SWAPPED);
    gtk_container_add(GTK_CONTAINER(vbox), dl->dismiss_button);

    gtk_widget_show_all(dl->window);
}

void palette_rnd_dlg_set(random_palette* rp, palette_rnd_dialog* dl)
{
    gtk_range_set_value(GTK_RANGE(dl->r_strength), rp->r_strength);
    gtk_range_set_value(GTK_RANGE(dl->g_strength), rp->g_strength);
    gtk_range_set_value(GTK_RANGE(dl->b_strength), rp->b_strength);
    gtk_range_set_value(GTK_RANGE(dl->r_bands), rp->r_bands);
    gtk_range_set_value(GTK_RANGE(dl->g_bands), rp->g_bands);
    gtk_range_set_value(GTK_RANGE(dl->b_bands), rp->b_bands);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->offset), rp->offset);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->stripe), rp->stripe);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->spread), rp->spread);
}


static void affect_update(GtkWidget* w, palette_rnd_dialog* dl)
{
    int st, sp;
    st = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->stripe));
    sp = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->spread));
    if (w == dl->stripe) {
        if (st < sp)
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->spread), st);
    }
    else if (w == dl->spread) {
        if (sp > st)
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->spread), st);
    }
}

static void set_rnd_palette(GtkWidget* widget, palette_rnd_dialog* dl)
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
            gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->offset));
    dl->img->rnd_pal->stripe =
            gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->stripe));
    dl->img->rnd_pal->spread =
            gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->spread));
    do_palette_randomize(dl->img->rnd_pal);
}

static void pal_rnd_destroy(GtkWidget* widget, palette_rnd_dialog* dl)
{
    (void)widget;
    g_free(dl);
}
