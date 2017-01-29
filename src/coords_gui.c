#include "coords_gui.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpfr.h>


#include "debug.h"
#include "main.h"
#include "main_gui.h"
#include "my_mpfr_to_str.h"
#include "setting.h"

static void coords_update(  GtkButton *button,  coords_dialog* dl);

static void coords_entry_delete(GtkEntry* entry,
                                gint start_pos, gint end_pos,
                                coords_dialog* dl);

static void coords_entry_insert(GtkEntry* entry,
                                gchar* new_text,
                                gint new_text_len,
                                gint* position,
                                coords_dialog* dl);


static void coords_dlg_destroy(GtkWidget* widget, coords_dialog* dl);
static void _coords_dlg_set(coords_dialog* dl);


static GtkWidget* coords_dialog_new_entry( coords_dialog* dl,
                                    const char* label,
                                    GtkWidget* table,
                                    int y,
                                    guint* ins_handle,
                                    guint* del_handle)
{
    GtkWidget* tmp = gtk_label_new(label);

    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), tmp, 0, 1, y, y+1,
                                    GTK_SHRINK, GTK_FILL, 0, 0);
    gtk_widget_show(tmp);

    tmp = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(table), tmp, 1, 2, y, y+1,
                                GTK_EXPAND, GTK_EXPAND, 0, 0);

    if (dl->entry_chars)
        gtk_entry_set_width_chars(GTK_ENTRY(tmp), dl->entry_chars);

    gtk_widget_show(tmp);

    *ins_handle = g_signal_connect(GTK_OBJECT(tmp), "insert-text",
                            G_CALLBACK(coords_entry_insert), dl);

    *del_handle = g_signal_connect(GTK_OBJECT(tmp), "delete-text",
                            G_CALLBACK(coords_entry_delete), dl);

    return tmp;
}



void coords_dlg_new(coords_dialog** ptr, image_info* img)
{
    GtkWidget* tmp;
    GtkWidget* table;
    GtkWidget* vbox;

    coords_dialog* dl;

    int y = 0;

    dl = g_malloc(sizeof(*dl));
    *ptr = dl;

    dl->pcoords = coords_dup(img->pcoords);

    dl->dialog = gtk_dialog_new();
    dl->img = img;
    dl->entry_chars = 64;
    dl->center = true;

    g_signal_connect(GTK_OBJECT(dl->dialog), "destroy",
                       G_CALLBACK(coords_dlg_destroy),
                       dl);
    g_signal_connect(GTK_OBJECT(dl->dialog), "destroy",
                       G_CALLBACK(gtk_widget_destroyed),
                       ptr);

    gtk_window_set_title(GTK_WINDOW(dl->dialog), "Coordinates");
    gtk_window_set_resizable(GTK_WINDOW(dl->dialog), TRUE);
    gtk_window_set_position(GTK_WINDOW(dl->dialog), GTK_WIN_POS_MOUSE);

    dl->ok_button = gtk_button_new_with_label("OK");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dl->dialog)->action_area),
                       dl->ok_button, TRUE, TRUE, 0);
    gtk_widget_show(dl->ok_button);

    dl->apply_button = gtk_button_new_with_label("Apply");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dl->dialog)->action_area),
                       dl->apply_button, TRUE, TRUE, 0);
    gtk_widget_show(dl->apply_button);

    tmp = gtk_button_new_with_label("Dismiss");
    g_signal_connect_object(GTK_OBJECT(tmp), "clicked",
                              G_CALLBACK(gtk_widget_destroy),
                              GTK_OBJECT(dl->dialog), G_CONNECT_SWAPPED );
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dl->dialog)->action_area), tmp,
                       TRUE, TRUE, 0);
    gtk_widget_show(tmp);

    table = gtk_table_new(6, 3, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 2);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dl->dialog)->vbox), vbox,
                       TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_widget_show(vbox);

    dl->xmin = coords_dialog_new_entry(dl, "xmin", table, y++,
                                        &dl->xmin_ins, &dl->xmin_del);

    dl->xmax = coords_dialog_new_entry(dl, "xmax", table, y++,
                                        &dl->xmax_ins, &dl->xmax_del);

    dl->ymax = coords_dialog_new_entry(dl, "ymax", table, y++,
                                        &dl->ymax_ins, &dl->ymax_del);

    tmp = gtk_hseparator_new();
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 2, y, y+1);
    gtk_widget_show(tmp);
    ++y;

    dl->cx = coords_dialog_new_entry(dl, "cx", table, y++,
                                        &dl->cx_ins, &dl->cx_del);

    dl->cy = coords_dialog_new_entry(dl, "cy", table, y++,
                                        &dl->cy_ins, &dl->cy_del);

    dl->size = coords_dialog_new_entry(dl, "size", table, y++,
                                        &dl->size_ins, &dl->size_del);

    tmp = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
    g_signal_connect(GTK_OBJECT(tmp), "clicked",
                              G_CALLBACK(coords_update), dl);

    gtk_table_attach(GTK_TABLE(table), tmp, 2, 3, 0, 7,
                                GTK_EXPAND, GTK_FILL, 0, 0);
    gtk_widget_show(tmp);
    dl->refresh_button = tmp;
    gtk_widget_set_sensitive(dl->refresh_button, FALSE);

    gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);
    gtk_widget_show(table);

    coords_dlg_set(img, dl);

    gtk_widget_show(dl->dialog);
}

static void coords_dlg_destroy(GtkWidget* widget, coords_dialog* dl)
{
    (void)widget;
    coords_free(dl->pcoords);
    g_free(dl);
}

static void coords_update(GtkButton *button, coords_dialog* dl)
{
    (void)button;

    mpfr_t* val[3];
    GtkWidget* ent[3];

    mpfr_t n;
    int i;

    DMSG("coords update..\n");

    if (dl->center)
    {
        val[0] = &dl->pcoords->cx;      ent[0] = dl->cx;
        val[1] = &dl->pcoords->cy;      ent[1] = dl->cy;
        val[2] = dl->pcoords->size;     ent[2] = dl->size;
    }
    else
    {
        val[0] = &dl->pcoords->xmin;    ent[0] = dl->xmin;
        val[1] = &dl->pcoords->xmax;    ent[1] = dl->xmax;
        val[2] = &dl->pcoords->ymax;    ent[2] = dl->ymax;
    }

    mpfr_init2(n, dl->pcoords->precision);

    for (i = 0; i < 3; ++i)
    {
        const gchar* str = gtk_entry_get_text(GTK_ENTRY(ent[i]));
        if (setting_get_mpfr_t(str, NULL, n))
            mpfr_set(*val[i], n, GMP_RNDN);
    }

    if (dl->center)
    {
        coords_size(dl->pcoords, *dl->pcoords->size);
        coords_center_to_rect(dl->pcoords);
    }
    else
        coords_rect_to_center(dl->pcoords);

    mpfr_clear(n);

    _coords_dlg_set(dl);

    gtk_widget_set_sensitive(dl->xmin,  true);
    gtk_widget_set_sensitive(dl->xmax,  true);
    gtk_widget_set_sensitive(dl->ymax,  true);
    gtk_widget_set_sensitive(dl->cx,    true);
    gtk_widget_set_sensitive(dl->cy,    true);
    gtk_widget_set_sensitive(dl->size,  true);
    gtk_widget_set_sensitive(dl->apply_button,  true);
    gtk_widget_set_sensitive(dl->ok_button,     true);
    gtk_widget_set_sensitive(dl->refresh_button, false);
}


static void coords_set_sensitive(coords_dialog* dl, GtkWidget* w)
{
    if (w == dl->xmin || w == dl->xmax || w == dl->ymax)
        dl->center = false;
    else
        dl->center = true;

    gtk_widget_set_sensitive(dl->xmin,  !dl->center);
    gtk_widget_set_sensitive(dl->xmax,  !dl->center);
    gtk_widget_set_sensitive(dl->ymax,  !dl->center);
    gtk_widget_set_sensitive(dl->cx,    dl->center);
    gtk_widget_set_sensitive(dl->cy,    dl->center);
    gtk_widget_set_sensitive(dl->size,  dl->center);

    gtk_widget_set_sensitive(dl->apply_button,  false);
    gtk_widget_set_sensitive(dl->ok_button,     false);
    gtk_widget_set_sensitive(dl->refresh_button, true);
}


static void coords_entry_delete(GtkEntry* entry,
                                gint start_pos, gint end_pos,
                                coords_dialog* dl)
{
    (void)start_pos;
    (void)end_pos;
    coords_set_sensitive(dl, GTK_WIDGET(entry));
}


static void coords_entry_insert(GtkEntry* entry,
                                gchar* new_text,
                                gint new_text_len,
                                gint* position,
                                coords_dialog* dl)
{
    (void)new_text;
    (void)new_text_len;
    (void)position;
    coords_set_sensitive(dl, GTK_WIDGET(entry));
}



void coords_dlg_set(image_info* img, coords_dialog* dl)
{
    coords_cpy(dl->pcoords, img->pcoords);
    coords_center_to_rect(dl->pcoords);
    _coords_dlg_set(dl);

    gtk_widget_set_sensitive(dl->xmin,  true);
    gtk_widget_set_sensitive(dl->xmax,  true);
    gtk_widget_set_sensitive(dl->ymax,  true);
    gtk_widget_set_sensitive(dl->cx,    true);
    gtk_widget_set_sensitive(dl->cy,    true);
    gtk_widget_set_sensitive(dl->size,  true);
/*
    gtk_widget_set_sensitive(dl->apply_button,  false);
    gtk_widget_set_sensitive(dl->ok_button,     false);
*/
    gtk_widget_set_sensitive(dl->refresh_button, false);
}


static void _coords_dlg_set(coords_dialog* dl)
{
    char* tmp;

    g_signal_handler_block(GTK_OBJECT(dl->xmin),    dl->xmin_ins);
    g_signal_handler_block(GTK_OBJECT(dl->xmax),    dl->xmax_ins);
    g_signal_handler_block(GTK_OBJECT(dl->ymax),    dl->ymax_ins);
    g_signal_handler_block(GTK_OBJECT(dl->cx),      dl->cx_ins);
    g_signal_handler_block(GTK_OBJECT(dl->cy),      dl->cy_ins);
    g_signal_handler_block(GTK_OBJECT(dl->size),    dl->size_ins);

    g_signal_handler_block(GTK_OBJECT(dl->xmin),    dl->xmin_del);
    g_signal_handler_block(GTK_OBJECT(dl->xmax),    dl->xmax_del);
    g_signal_handler_block(GTK_OBJECT(dl->ymax),    dl->ymax_del);
    g_signal_handler_block(GTK_OBJECT(dl->cx),      dl->cx_del);
    g_signal_handler_block(GTK_OBJECT(dl->cy),      dl->cy_del);
    g_signal_handler_block(GTK_OBJECT(dl->size),    dl->size_del);


    tmp = my_mpfr_to_str(dl->pcoords->xmin);
    gtk_entry_set_text( GTK_ENTRY(dl->xmin), tmp);
    free(tmp);

    tmp = my_mpfr_to_str(dl->pcoords->xmax);
    gtk_entry_set_text( GTK_ENTRY(dl->xmax), tmp);
    free(tmp);

    tmp = my_mpfr_to_str(dl->pcoords->ymax);
    gtk_entry_set_text( GTK_ENTRY(dl->ymax), tmp);
    free(tmp);

    tmp = my_mpfr_to_str(dl->pcoords->cx);
    gtk_entry_set_text( GTK_ENTRY(dl->cx), tmp);
    free(tmp);

    tmp = my_mpfr_to_str(dl->pcoords->cy);
    gtk_entry_set_text( GTK_ENTRY(dl->cy), tmp);
    free(tmp);

    tmp = my_mpfr_to_str(dl->pcoords->width);
    gtk_entry_set_text( GTK_ENTRY(dl->size), tmp);
    free(tmp);

    g_signal_handler_unblock(GTK_OBJECT(dl->xmin),  dl->xmin_ins);
    g_signal_handler_unblock(GTK_OBJECT(dl->xmax),  dl->xmax_ins);
    g_signal_handler_unblock(GTK_OBJECT(dl->ymax),  dl->ymax_ins);
    g_signal_handler_unblock(GTK_OBJECT(dl->cx),    dl->cx_ins);
    g_signal_handler_unblock(GTK_OBJECT(dl->cy),    dl->cy_ins);
    g_signal_handler_unblock(GTK_OBJECT(dl->size),  dl->size_ins);

    g_signal_handler_unblock(GTK_OBJECT(dl->xmin),  dl->xmin_del);
    g_signal_handler_unblock(GTK_OBJECT(dl->xmax),  dl->xmax_del);
    g_signal_handler_unblock(GTK_OBJECT(dl->ymax),  dl->ymax_del);
    g_signal_handler_unblock(GTK_OBJECT(dl->cx),    dl->cx_del);
    g_signal_handler_unblock(GTK_OBJECT(dl->cy),    dl->cy_del);
    g_signal_handler_unblock(GTK_OBJECT(dl->size),  dl->size_del);
}

void coords_ok_cmd(GtkWidget* w, coords_dialog* dl)
{
    coords_apply_cmd(w, dl);
    gtk_widget_destroy(dl->dialog);
}

void coords_apply_cmd(GtkWidget* w, coords_dialog* dl)
{
    (void)w;
    coords_to(dl->img->pcoords, dl->pcoords->cx, dl->pcoords->cy);
    coords_size(dl->img->pcoords, *dl->pcoords->size);
    gui_start_rendering(dl->img);
}

