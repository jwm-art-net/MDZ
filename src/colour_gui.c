#include "colour_gui.h"

#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "main_gui.h"


static void toggle_palette_ip(GtkWidget* widget, colour_dialog* dl);
static void update_colour(GtkWidget* widget, colour_dialog* dl);


colour_dialog* colour_dlg_new(image_info* img)
{
    GtkWidget* tmp;
    GtkWidget* table;
    GtkObject* adj;
    colour_dialog* cd;

    int y = 0;

    cd = malloc(sizeof(*cd));

    if (!cd)
        return 0;

    cd->box = gtk_vbox_new(FALSE, 0);
    table = gtk_table_new(2, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(cd->box), table, TRUE, TRUE, 0);

    tmp = gtk_label_new("Scale:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, y, y+1);
    gtk_widget_show(tmp);

    adj = gtk_adjustment_new(img->colour_scale,
                             0.0000001, 999.0,
                             0.001953125, 0.0625, 0.0);

    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0.0, 8.0);

    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 1, 2, y, y+1);
    g_signal_connect(GTK_OBJECT(adj), "value-changed",
                        G_CALLBACK(update_colour),
                            cd);
    gtk_widget_show(tmp);
    cd->scale = tmp;
    ++y;

    tmp = gtk_check_button_new_with_label("Interpolate");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp), img->palette_ip);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 1, 2, y, y+1);
    g_signal_connect(GTK_OBJECT(tmp), "toggled",
                        G_CALLBACK(toggle_palette_ip),
                            cd);
    gtk_widget_show(tmp);
    cd->palette_ip = tmp;
    ++y;

    gtk_widget_show(table);
    gtk_widget_show(cd->box);

    return cd;
}


void colour_dlg_free(colour_dialog* cd)
{
    free(cd);
}


static void update_colour(GtkWidget* widget, colour_dialog* dl)
{
    (void)widget;
    double col_sc = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dl->scale));

    if (col_sc != img->colour_scale)
    {
        img->colour_scale = col_sc;
        gui_reapply_palette();
    }
}


static void toggle_palette_ip(GtkWidget* widget, colour_dialog* dl)
{
    (void)widget;
    img->palette_ip = GTK_TOGGLE_BUTTON(dl->palette_ip)->active;
    gui_reapply_palette();
}


void colour_dlg_set(image_info* img, colour_dialog* cd)
{
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(cd->scale),
                                              img->colour_scale);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cd->palette_ip),
                                              img->palette_ip);
}

