#ifndef FRACSET_DLG_H
#include "fractset_gui.h"

#include <stdlib.h>
#include <stdio.h>


#include "main_gui.h"


enum {
    FS_NO_COLUMN,
    FS_NAME_COLUMN,
    FS_N_COLUMNS
};


static void update_fracset_dialog(GtkWidget* widget, fracset_dialog* fsd);
static void fracset_dlg_destroy(GtkWidget* widget, fracset_dialog* fsd);
static GtkWidget* create_fracset_param( GtkListStore* plist,
                                        int initial_item );


void fracset_dlg_new(fracset_dialog** ptr, image_info* img)
{
    GtkWidget* vbox;
    GtkWidget* tmp;
    GtkListStore* flist  = 0;
    GtkTreeIter   fiter;
    fracset_dialog* fsd;
    int i;

    fsd = g_malloc(sizeof(*fsd));
    *ptr = fsd;

    if (!fsd)
        return;
    
    fsd->img = img;

    flist = gtk_list_store_new(FS_N_COLUMNS, G_TYPE_INT, G_TYPE_STRING);

    i = 0;
    while(fractal_str[i] != 0)
    {
        gtk_list_store_append(  flist,  &fiter);
        gtk_list_store_set(     flist,  &fiter,
                                FS_NO_COLUMN, i,
                                FS_NAME_COLUMN, fractal_str[i], -1);
        ++i;
    }

    fsd->dialog = gtk_dialog_new();
    g_signal_connect(GTK_OBJECT(fsd->dialog), "destroy",
                       G_CALLBACK(fracset_dlg_destroy), fsd);
    g_signal_connect(GTK_OBJECT(fsd->dialog), "destroy",
                       G_CALLBACK(gtk_widget_destroyed),
                       ptr);

    gtk_window_set_title(GTK_WINDOW(fsd->dialog), "Fractal");
    gtk_window_set_resizable(GTK_WINDOW(fsd->dialog), FALSE);
    gtk_window_set_position(GTK_WINDOW(fsd->dialog), GTK_WIN_POS_MOUSE);

    /* BUTTONS */
    fsd->ok_button = gtk_button_new_with_label("OK");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(fsd->dialog)->action_area),
                       fsd->ok_button,   TRUE, TRUE, 0);

    fsd->apply_button = gtk_button_new_with_label("Apply");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(fsd->dialog)->action_area),
                       fsd->apply_button,TRUE, TRUE, 0);

    tmp = gtk_button_new_with_label("Dismiss");
    g_signal_connect_object(GTK_OBJECT(tmp), "clicked",
                              G_CALLBACK(gtk_widget_destroy),
                              GTK_OBJECT(fsd->dialog), G_CONNECT_SWAPPED);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(fsd->dialog)->action_area),
                       tmp,             TRUE, TRUE, 0);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(fsd->dialog)->vbox),
                                        vbox, TRUE, TRUE, 0);

    fsd->fractal = create_fracset_param(flist, img->fractal);
    gtk_container_add(GTK_CONTAINER(vbox), fsd->fractal);

    gtk_widget_show_all(fsd->dialog);
}

void fracset_dlg_set(fracset_dialog* fsd, image_info* img)
{
    gtk_combo_box_set_active(GTK_COMBO_BOX(fsd->fractal), img->fractal);
}

void get_fractal_settings(fracset_dialog* fsd, image_info* img)
{
    img->fractal = gtk_combo_box_get_active(GTK_COMBO_BOX(fsd->fractal));
}


static GtkWidget* create_fracset_param( GtkListStore* plist,
                                        int initial_item )
{
    GtkWidget*          combo;
    GtkCellRenderer*    renderer;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(plist));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), initial_item);

    renderer = gtk_cell_renderer_text_new();

    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, FALSE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), 
                                    renderer, "text", 1, NULL);
    return combo;
}


static void fracset_dlg_destroy(GtkWidget* widget, fracset_dialog* fsd)
{
    (void)widget;
    g_free(fsd);
}

void fracset_dlg_ok_cmd(GtkWidget* w, fracset_dialog* dl)
{
    fracset_dlg_apply_cmd(w, dl);
    gtk_widget_destroy(dl->dialog);
}

void fracset_dlg_apply_cmd(GtkWidget* w,fracset_dialog* dl)
{
    get_fractal_settings(dl, dl->img);
    gui_start_rendering(dl->img);
}

#endif
