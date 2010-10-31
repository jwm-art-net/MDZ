#include "pal_fun_dlg.h"
#include "main.h"

/* Palette functions dialogue */

static void affect_update(GtkWidget* w, palette_fun_dialog* dl);

static void pal_fun_buttonfunc(GtkWidget* widget, palette_fun_dialog* dl);

static void set_fun_palette(GtkWidget* widget, palette_fun_dialog* dl);
static void pal_fun_destroy(GtkWidget* widget, palette_fun_dialog* dl);

void palette_fun_dlg_new(palette_fun_dialog** ptr, function_palette* fp)
{
    GtkWidget* tmp;
    GtkWidget* vbox;
    GtkWidget* fn_frame;
    GtkWidget* fn_box;
    GtkWidget* af_frame;
    GtkWidget* af_table;
    GtkObject* adj;
    palette_fun_dialog* dl;

    dl = g_malloc(sizeof(palette_fun_dialog));
    dl->fun_pal = fp;
    *ptr = dl;

    dl->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(GTK_OBJECT(dl->window), "destroy",
                     G_CALLBACK(pal_fun_destroy), dl);
    g_signal_connect(GTK_OBJECT(dl->window), "destroy",
                     G_CALLBACK(gtk_widget_destroyed), ptr);
    gtk_window_set_title(GTK_WINDOW(dl->window), "Palette Functions");
    gtk_window_set_position(GTK_WINDOW(dl->window), GTK_WIN_POS_MOUSE);
    gtk_window_set_resizable(GTK_WINDOW(dl->window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(dl->window), 10);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(dl->window), vbox);
    gtk_widget_show(vbox);

    fn_frame = gtk_frame_new("Functions");
    af_frame = gtk_frame_new("Affect");
    gtk_container_set_border_width(GTK_CONTAINER(fn_frame),4);
    gtk_container_set_border_width(GTK_CONTAINER(af_frame),4);
    gtk_container_add(GTK_CONTAINER(vbox), fn_frame);
    gtk_container_add(GTK_CONTAINER(vbox), af_frame);

    af_table = gtk_table_new(3, 2, TRUE);

    tmp = gtk_label_new("Offset:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(af_table), tmp, 0, 1, 0, 1);
    gtk_widget_show(tmp);

    tmp = gtk_label_new("Stripe:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(af_table), tmp, 0, 1, 1, 2);
    gtk_widget_show(tmp);

    tmp = gtk_label_new("Spread:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(af_table), tmp, 0, 1, 2, 3);
    gtk_widget_show(tmp);

    adj = gtk_adjustment_new(dl->fun_pal->offset, 0, 255, 1, 1, 0);
    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(af_table), tmp, 1, 2, 0, 1);
    g_signal_connect(GTK_OBJECT(tmp), "changed",
                       G_CALLBACK(affect_update),
                       dl);
    gtk_widget_show(tmp);
    dl->offset = tmp;

    adj = gtk_adjustment_new(dl->fun_pal->stripe, 1, 128, 1, 1, 0);
    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(af_table), tmp, 1, 2, 1, 2);
    g_signal_connect(GTK_OBJECT(tmp), "changed",
                       G_CALLBACK(affect_update),
                       dl);
    gtk_widget_show(tmp);
    dl->stripe = tmp;

    adj = gtk_adjustment_new(dl->fun_pal->spread, 1, 128, 1, 1, 0);
    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(af_table), tmp, 1, 2, 2, 3);
    g_signal_connect(GTK_OBJECT(tmp), "changed",
                       G_CALLBACK(affect_update),
                       dl);
    gtk_widget_show(tmp);
    dl->spread = tmp;

    gtk_container_add(GTK_CONTAINER(af_frame), af_table);

    fn_box = gtk_vbox_new(FALSE, 0);

    dl->ex_rg_button = gtk_button_new_with_label("R <-> G");
    g_signal_connect(GTK_OBJECT(dl->ex_rg_button), "clicked",
                     G_CALLBACK(pal_fun_buttonfunc), dl);
    dl->ex_gb_button = gtk_button_new_with_label("G <-> B");
    g_signal_connect(GTK_OBJECT(dl->ex_gb_button), "clicked",
                     G_CALLBACK(pal_fun_buttonfunc), dl);
    dl->ex_br_button = gtk_button_new_with_label("B <-> R");
    g_signal_connect(GTK_OBJECT(dl->ex_br_button), "clicked",
                     G_CALLBACK(pal_fun_buttonfunc), dl);

    dl->rot_rgb_button = gtk_button_new_with_label("Rotate RGB");
    g_signal_connect(GTK_OBJECT(dl->rot_rgb_button), "clicked",
                     G_CALLBACK(pal_fun_buttonfunc), dl);

    dl->inv_rgb_button = gtk_button_new_with_label("Invert RGB");
    g_signal_connect(GTK_OBJECT(dl->inv_rgb_button), "clicked",
                     G_CALLBACK(pal_fun_buttonfunc), dl);

    dl->inv_r_button = gtk_button_new_with_label("Invert Red");
    g_signal_connect(GTK_OBJECT(dl->inv_r_button), "clicked",
                     G_CALLBACK(pal_fun_buttonfunc), dl);
    dl->inv_g_button = gtk_button_new_with_label("Invert Green");
    g_signal_connect(GTK_OBJECT(dl->inv_g_button), "clicked",
                     G_CALLBACK(pal_fun_buttonfunc), dl);
    dl->inv_b_button = gtk_button_new_with_label("Invert Blue");
    g_signal_connect(GTK_OBJECT(dl->inv_b_button), "clicked",
                     G_CALLBACK(pal_fun_buttonfunc), dl);

    gtk_box_pack_start(GTK_BOX(fn_box), dl->ex_rg_button,
                                        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(fn_box), dl->ex_gb_button,
                                        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(fn_box), dl->ex_br_button,
                                        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(fn_box), dl->rot_rgb_button,
                                        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(fn_box), dl->inv_rgb_button,
                                        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(fn_box), dl->inv_r_button,
                                        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(fn_box), dl->inv_g_button,
                                        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(fn_box), dl->inv_b_button,
                                        FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(fn_frame), fn_box);

    dl->dismiss_button = gtk_button_new_with_label("Dismiss");
    g_signal_connect_object(GTK_OBJECT(dl->dismiss_button), "clicked",
                            G_CALLBACK(gtk_widget_destroy),
                            GTK_OBJECT(dl->window), G_CONNECT_SWAPPED);
    gtk_container_add(GTK_CONTAINER(vbox), dl->dismiss_button);

    gtk_widget_show_all(dl->window);
}

static void affect_update(GtkWidget* w, palette_fun_dialog* dl)
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

static void pal_fun_buttonfunc(GtkWidget* widget, palette_fun_dialog* dl)
{
    set_fun_palette(widget, dl);
    if (widget == dl->ex_rg_button)      dl->fun_pal->func = PF_EX_RG;
    else if (widget == dl->ex_gb_button) dl->fun_pal->func = PF_EX_GB;
    else if (widget == dl->ex_br_button) dl->fun_pal->func = PF_EX_BR;
    else if (widget == dl->rot_rgb_button)
        dl->fun_pal->func = PF_ROT_RGB;
    else if (widget == dl->inv_rgb_button)
        dl->fun_pal->func = PF_INV_RGB;
    else if (widget == dl->inv_r_button) dl->fun_pal->func = PF_INV_R;
    else if (widget == dl->inv_g_button) dl->fun_pal->func = PF_INV_G;
    else if (widget == dl->inv_b_button) dl->fun_pal->func = PF_INV_B;
    do_palette_function(dl->fun_pal);
}

static void set_fun_palette(GtkWidget* widget, palette_fun_dialog* dl)
{
    (void)widget;
    dl->fun_pal->offset =
                        atoi(gtk_entry_get_text(GTK_ENTRY(dl->offset)));
    dl->fun_pal->stripe =
                        atoi(gtk_entry_get_text(GTK_ENTRY(dl->stripe)));
    dl->fun_pal->spread =
                        atoi(gtk_entry_get_text(GTK_ENTRY(dl->spread)));
}

static void pal_fun_destroy(GtkWidget* widget, palette_fun_dialog* dl)
{
    (void)widget;
    g_free(dl);
}
