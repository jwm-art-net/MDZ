#include "colour_gui.h"

#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "main_gui.h"

static void colour_dlg_destroy(GtkWidget* widget, colour_dialog* dl);
static void toggle_palette_ip(GtkWidget* widget, colour_dialog* dl);
static void update_colour(GtkWidget* widget, colour_dialog* dl);

void colour_dlg_new(colour_dialog** ptr, image_info* img)
{
    GtkWidget* tmp;
    GtkWidget* table;
    GtkWidget* vbox;
    GtkObject* adj;
    colour_dialog* dl;

    int y = 0;

    dl = g_malloc(sizeof(colour_dialog));
    *ptr = dl;

    dl->dialog = gtk_dialog_new();

    g_signal_connect(GTK_OBJECT(dl->dialog), "destroy",
                       G_CALLBACK(colour_dlg_destroy),
                       dl);
    g_signal_connect(GTK_OBJECT(dl->dialog), "destroy",
                       G_CALLBACK(gtk_widget_destroyed),
                       ptr);

    gtk_window_set_title(GTK_WINDOW(dl->dialog), "Colour Scaling");
    gtk_window_set_resizable(GTK_WINDOW(dl->dialog), FALSE);
    gtk_window_set_position(GTK_WINDOW(dl->dialog), GTK_WIN_POS_MOUSE);

    tmp = gtk_button_new_with_label("Dismiss");
    g_signal_connect_object(GTK_OBJECT(tmp), "clicked",
                              G_CALLBACK(gtk_widget_destroy),
                              GTK_OBJECT(dl->dialog), G_CONNECT_SWAPPED );
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dl->dialog)->action_area), tmp,
                       TRUE, TRUE, 0);
    gtk_widget_show(tmp);

    table = gtk_table_new(8, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 2);
    gtk_table_set_row_spacing(GTK_TABLE(table), 2, 10);
    gtk_table_set_row_spacing(GTK_TABLE(table), 3, 15);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dl->dialog)->vbox), vbox,
                       TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_widget_show(vbox);

    tmp = gtk_label_new("Scale:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, y, y+1);
    gtk_widget_show(tmp);
    dl->scale_label = tmp;

    adj = gtk_adjustment_new(img->colour_scale,
                             0.0000001, 999.0,
                             0.001953125, 0.0625, 0.0);
    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0.0, 8.0);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 1, 2, y, y+1);
    g_signal_connect(GTK_OBJECT(tmp), "changed",
                       G_CALLBACK(update_colour),
                       dl);
    gtk_widget_show(tmp);
    dl->scale = tmp;
    ++y;

    tmp = gtk_check_button_new_with_label("Interpolate");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp), img->palette_ip);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 1, 2, y, y+1);
    g_signal_connect(GTK_OBJECT(tmp), "toggled",
                       G_CALLBACK(toggle_palette_ip),
                       dl);
    gtk_widget_show(tmp);
    dl->palette_ip = tmp;
    ++y;

    gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);
    gtk_widget_show(table);
    
    gtk_widget_show(dl->dialog);
}

static void colour_dlg_destroy(GtkWidget* widget,
                               colour_dialog* dl)
{
    (void)widget;
    g_free(dl);
}

static void update_colour(GtkWidget* widget, colour_dialog* dl)
{
    (void)widget;
    double col_sc = gtk_spin_button_get_value(
                                    GTK_SPIN_BUTTON(dl->scale));
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

void colour_dlg_set(image_info* img, colour_dialog* dl)
{
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->scale),
                                              img->colour_scale);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dl->palette_ip),
                                              img->palette_ip);
}
