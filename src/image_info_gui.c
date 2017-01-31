#include "image_info_gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <mpfr.h>

#include "debug.h"
#include "my_mpfr_to_str.h"
#include "main.h"
#include "main_gui.h"
#include "misc_gui.h"
#include "render_threads.h"

static GtkWidget* check_button_fractal = 0;
static GtkWidget* check_button_palette = 0;

static void image_dlg_destroy(GtkWidget* widget, image_info_dialog* dl);
static void width_update(GtkWidget* w, image_info_dialog* dl);
static void height_update(GtkWidget* w, image_info_dialog* dl);
static void aa_update(GtkWidget* w, image_info_dialog* dl);
static void constrain_update(GtkWidget*w, image_info_dialog* dl);
static void update_text(GtkLabel* label, int w, int h, int aa);
static void mp_update(GtkWidget*w, image_info_dialog* dl);

static guint width_sig = 0;
static guint height_sig = 0;

void image_info_dlg_new(image_info_dialog** ptr, image_info* img)
{
    GtkWidget* tmp;
    GtkWidget* table;
    GtkWidget* vbox;
    GtkObject* adj;
    image_info_dialog* dl;

    DMSG("image_info_dlg_new\n");

    int y = 0;

    dl = g_malloc(sizeof(image_info_dialog));
    *ptr = dl;

    dl->ratio = (double)img->user_width/img->user_height;
    dl->dialog = gtk_dialog_new();

    g_signal_connect(GTK_OBJECT(dl->dialog), "destroy",
                       G_CALLBACK(image_dlg_destroy),
                       dl);
    g_signal_connect(GTK_OBJECT(dl->dialog), "destroy",
                       G_CALLBACK(gtk_widget_destroyed),
                       ptr);
    gtk_window_set_title(GTK_WINDOW(dl->dialog), "Attributes");
    gtk_window_set_resizable(GTK_WINDOW(dl->dialog), FALSE);
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

    tmp = gtk_label_new("Width:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, y, y+1);
    gtk_widget_show(tmp);

    adj = gtk_adjustment_new(img->user_width, 1.0, 999999.0,
                             4.0, 4.0, 0.0);
    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0.0, 0.0);

    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 1, 2, y, y+1);
    width_sig = g_signal_connect(GTK_OBJECT(tmp), "changed",
                       G_CALLBACK(width_update),
                       dl);
    gtk_widget_show(tmp);
    dl->width = tmp;
    ++y;

    tmp = gtk_label_new("Height:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, y, y+1);
    gtk_widget_show(tmp);

    adj = gtk_adjustment_new(img->user_height, 1.0, 999999.0,
                             3.0, 3.0, 0.0);
    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0.0, 0.0);

    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 1, 2, y, y+1);
    height_sig = g_signal_connect(GTK_OBJECT(tmp), "changed",
                       G_CALLBACK(height_update),
                       dl);
    gtk_widget_show(tmp);
    dl->height = tmp;
    ++y;

    tmp = gtk_check_button_new_with_label("Maintain aspect ratio");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 2, y, y+1);
    g_signal_connect(GTK_OBJECT(tmp), "toggled",
                       G_CALLBACK(constrain_update),
                       dl);
    gtk_widget_show(tmp);
    dl->const_ra = tmp;
    ++y;

    tmp = gtk_label_new("Anti-aliasing factor:\n(1 = no anti-aliasing)");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_label_set_justify(GTK_LABEL(tmp), GTK_JUSTIFY_LEFT);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, y, y+1);
    gtk_widget_show(tmp);

    adj = gtk_adjustment_new(img->aa_factor, 1.0, MAX_AA, 1.0, 1.0, 0.0);
    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0.0, 0.0);

    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 1, 2, y, y+1);
    g_signal_connect(GTK_OBJECT(tmp), "changed",
                       G_CALLBACK(aa_update),
                       dl);
    gtk_widget_show(tmp);
    dl->aa = tmp;
    ++y;

    tmp = gtk_label_new("");
    update_text(GTK_LABEL(tmp), img->user_width, img->user_height,
                img->aa_factor);
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.5, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 2, y, y+1);
    gtk_widget_show(tmp);
    dl->text = tmp;
    ++y;

    tmp = gtk_hseparator_new();
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 2, y, y+1);
    gtk_widget_show(tmp);
    ++y;

    tmp = gtk_check_button_new_with_label("Multiple Precision Math");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp),
                                                img->use_multi_prec);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 2, y, y+1);
    g_signal_connect(GTK_OBJECT(tmp), "toggled",
                       G_CALLBACK(mp_update),
                       dl);
    gtk_widget_set_tooltip_text(tmp, "Increases zoom depth");
    gtk_widget_show(tmp);
    dl->use_multi_prec = tmp;
    ++y;

    tmp = gtk_check_button_new_with_label("Correct Rounding");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp),
                                                img->use_rounding);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 2, y, y+1);
    gtk_widget_set_tooltip_text(tmp,
            "Correct rounding rounds the result of calculations exactly"
            "to the specified precision. Is slower in many instances, "
            "but calculations may use more precision in order to arrive "
            "at the correct result.");

    gtk_widget_show(tmp);
    dl->use_rounding = tmp;
    ++y;

    tmp = gtk_label_new(""); /* precision (bits) */
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, y, y+1);
    dl->plabel = tmp;
    gtk_widget_show(tmp);
    adj = gtk_adjustment_new(img->precision,
                             MIN_PRECISION, MAX_PRECISION,
                             8.0, 16.0, 0.0);
    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0.0, 0.0);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 1, 2, y, y+1);
    gtk_widget_set_tooltip_text(tmp,
                            "How many bits to use for Maths precision");
    gtk_widget_set_sensitive(tmp, img->use_multi_prec);
    gtk_widget_show(tmp);
    dl->precision = tmp;
    ++y;

    tmp = gtk_label_new(""); /* actual precision (non-rounding) */
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 2, y, y+1);
    dl->actual_prec = tmp;
    gtk_widget_show(tmp);
    gtk_widget_set_sensitive(tmp, FALSE);
    gtk_widget_set_tooltip_text(tmp,
            "The actual precision used when not using correct rounding "
            "(and multiple precision is activated).");
    ++y;

    tmp = gtk_hseparator_new();
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 2, y, y+1);
    gtk_widget_show(tmp);
    ++y;

    tmp = gtk_label_new("Thread count:");
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, y, y+1);
    gtk_widget_show(tmp);
    adj = gtk_adjustment_new(img->thread_count,
                             1, MAX_THREAD_COUNT,
                             1.0, 2.0, 0.0);
    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0.0, 0.0);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 1, 2, y, y+1);
    gtk_widget_set_tooltip_text(tmp,
                            "Number of threads to use for rendering.");
    gtk_widget_show(tmp);
    dl->thread_count = tmp;

    gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);
    gtk_widget_show(table);

    image_dlg_update_recommended_precision(dl, img);

    gtk_widget_show(dl->dialog);
}


void width_update(GtkWidget* w, image_info_dialog* dl)
{
    DMSG("width_update\n");
    (void)w;
    int height;
    int width = atoi(gtk_entry_get_text(GTK_ENTRY(dl->width)));

    if (width <= 0)
        return;

    if (GTK_TOGGLE_BUTTON(dl->const_ra)->active)
    {
        height = width/dl->ratio;
        g_signal_handler_block(GTK_OBJECT(dl->height),height_sig);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->height),
                                  height);
        g_signal_handler_unblock(GTK_OBJECT(dl->height),height_sig);
    }

    update_text(GTK_LABEL(dl->text), width,
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->height)),
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->aa)));
}


void height_update(GtkWidget* w, image_info_dialog* dl)
{
    DMSG("height_update\n");
    (void)w;
    int width;
    int height = atoi(gtk_entry_get_text(GTK_ENTRY(dl->height)));

    if (height <= 0)
        return;

    if (GTK_TOGGLE_BUTTON(dl->const_ra)->active)
    {
        width = height*dl->ratio;
        g_signal_handler_block(GTK_OBJECT(dl->width),width_sig);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->width),
                                  width);
        g_signal_handler_unblock(GTK_OBJECT(dl->width),width_sig);
    }

    update_text(GTK_LABEL(dl->text),
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->width)),
        height, gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(dl->aa)));
}

void aa_update(GtkWidget* w, image_info_dialog* dl)
{
    (void)w;
    int aa = atoi(gtk_entry_get_text(GTK_ENTRY(dl->aa)));
    if (aa <= 0)
        return;
    update_text(GTK_LABEL(dl->text),
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->width)),
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->height)),
        aa);
}

void constrain_update(GtkWidget* w, image_info_dialog* dl)
{
    (void)w;
    if (GTK_TOGGLE_BUTTON(dl->const_ra)->active)
    {
        dl->ratio =
            gtk_spin_button_get_value(GTK_SPIN_BUTTON(dl->width)) /
            gtk_spin_button_get_value(GTK_SPIN_BUTTON(dl->height));
    }
}

void update_text(GtkLabel* label, int w, int h, int aa)
{
    char buf[256];
    double megs;

    megs = (w*h*4.0*aa*aa + w*h*4.0)/(1024.0*1024.0);
    snprintf(buf, 256, "Memory required: %.2f M", megs);
    gtk_label_set_text(label, buf);
}

void image_dlg_update_recommended_precision(image_info_dialog* dl,
                                            image_info* img)
{
    DMSG("image_dlg_update_recommended_precision\n");
    char str[MAX_DP];
    sprintf(str, "Precision (bits):\nScant minimum:%ld",
                        (long)img->pcoords->recommend);
    gtk_label_set_text(GTK_LABEL(dl->plabel), str);

    sprintf(str, "Non-rounding precision :%ld",
                        (long)img->pcoords->gmp_precision);
    gtk_label_set_text(GTK_LABEL(dl->actual_prec), str);

    mp_update(0, dl);
}

void mp_update(GtkWidget* w, image_info_dialog* dl)
{
    (void)w;
    if (GTK_TOGGLE_BUTTON(dl->use_multi_prec)->active)
    {
        gtk_widget_set_sensitive(dl->precision, TRUE);
        gtk_widget_set_sensitive(dl->plabel, TRUE);
        gtk_widget_set_sensitive(dl->use_rounding, TRUE);
    }
    else
    {
        gtk_widget_set_sensitive(dl->precision, FALSE);
        gtk_widget_set_sensitive(dl->plabel, FALSE);
        gtk_widget_set_sensitive(dl->use_rounding, FALSE);
    }
}

static void image_dlg_destroy(GtkWidget* widget,
                               image_info_dialog* dl)
{
    (void)widget;
    g_free(dl);
}

void image_info_ok_cmd(GtkWidget* w, image_info_dialog* dl)
{
    DMSG("image_info_ok_cmd\n");
    image_info_apply_cmd(w, dl);
    gtk_widget_destroy(dl->dialog);
}

void image_info_apply_cmd(GtkWidget* widget, image_info_dialog* dl)
{
    (void)widget;
    int tc;
    DMSG("image_info_apply_cmd\n");

    mp_prec_t precision =
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->precision));

    bool changed = FALSE;
    bool use_multi_prec = gtk_toggle_button_get_active(
                                GTK_TOGGLE_BUTTON(dl->use_multi_prec));
    bool use_rounding =
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dl->use_rounding));

    int w = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->width));
    int h = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->height));
    int aa = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->aa));

    if (w != img->user_width
     || h != img->user_height
     || aa!= img->aa_factor)
    {
        changed = true;
        rth_ui_stop_render_and_wait((rthdata*)img->rth_ptr);
        image_info_set(img, w, h, aa);
        if (img->user_width < MIN_WINDOW_WIDTH)
        {
            gui_resize(MIN_WINDOW_WIDTH, img->user_height);
        }
        else
            gui_resize(img->user_width, img->user_height);
    }

    if (img->use_multi_prec != use_multi_prec
     || (use_multi_prec && img->use_rounding != use_rounding))
    {
        changed = true;
        image_info_set_multi_prec(img, use_multi_prec, use_rounding);
    }

    if (use_multi_prec && precision != img->precision)
    {
        changed = true;
        image_info_set_precision(img,   precision);
    }

    if (img->thread_count != (tc = gtk_spin_button_get_value_as_int(
                                    GTK_SPIN_BUTTON(dl->thread_count))))
    {
        changed = TRUE;
        image_info_threads_change(img, tc);
    }

    if (changed)
    {
        gui_start_rendering(img);
        gui_resize_preview();
        if (gui_is_julia_browsing())
        {
            gui_hide_julia_preview();
            gui_show_julia_preview();
            gui_start_rendering(j_pre);
        }
    }
}

void do_image_info_save_dialog(image_info* img)
{
    GtkWidget* dialog =
        gtk_file_chooser_dialog_new("Save Settings",
                        gui_window(),
                        GTK_FILE_CHOOSER_ACTION_SAVE,
                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                        GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                        NULL);

    const char* lud = image_info_get_last_used_dir();
    if (lud)
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), lud);

    const char* luf = image_info_get_last_used_filename();
    if (!luf)
        luf = "untitled.mdz";

    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), luf);

    gtk_file_chooser_set_do_overwrite_confirmation (
                        GTK_FILE_CHOOSER (dialog), TRUE);

    gui_file_chooser_add_filter(dialog, "MDZ files", "*.mdz");
    gui_file_chooser_add_filter(dialog, "All files", "*");

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        filename = gtk_file_chooser_get_filename(
                        GTK_FILE_CHOOSER(dialog));
        image_info_save_all(img, filename);
        g_free(filename);
    }
    gtk_widget_destroy (dialog);
}


static void file_data_toggled(GtkToggleButton* toggle, gpointer data)
{
    (void)data;
    DMSG("file_data_toggled\n");
    bool data_to_load = gtk_toggle_button_get_active(
                                GTK_TOGGLE_BUTTON(check_button_fractal));

    data_to_load |= gtk_toggle_button_get_active(
                                GTK_TOGGLE_BUTTON(check_button_palette));

    if (!data_to_load)
    {
        if (toggle == GTK_TOGGLE_BUTTON(check_button_fractal))
            gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON(check_button_palette), TRUE);
        else
            gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON(check_button_fractal), TRUE);
    }
}


int do_image_info_load_dialog(image_info* img)
{
    int good = 0;
    DMSG("do_image_info_load_dialog\n");
    GtkWidget*  frame = 0;
    GtkWidget*  hbox = 0;
    GtkWidget*  tmp = 0;

    GtkWidget* dl =
        gtk_file_chooser_dialog_new("Load Settings",
                        gui_window(),
                        GTK_FILE_CHOOSER_ACTION_SAVE,
                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                        NULL);

    const char* lud = image_info_get_last_used_dir();
    if (lud)
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dl), lud);

    frame = gtk_frame_new("Data to load");
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(frame), hbox);

    tmp = gtk_check_button_new_with_label("Fractal Settings");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp), TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), tmp, TRUE, TRUE, 0);
    g_signal_connect(GTK_OBJECT(tmp), "toggled",
                       G_CALLBACK(file_data_toggled), dl);
    gtk_widget_show(tmp);
    check_button_fractal = tmp;

    tmp = gtk_check_button_new_with_label("Palette Data");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp), TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), tmp, TRUE, TRUE, 0);
    g_signal_connect(GTK_OBJECT(tmp), "toggled",
                       G_CALLBACK(file_data_toggled), dl);
    gtk_widget_show(tmp);
    check_button_palette = tmp;

    gtk_widget_show(frame);
    gtk_widget_show(hbox);

    gui_file_chooser_add_filter(dl, "MDZ files", "*.mdz");
    gui_file_chooser_add_filter(dl, "All files", "*");

    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dl), frame);

    if (gtk_dialog_run(GTK_DIALOG(dl)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        int flags = 0;

        if (gtk_toggle_button_get_active(
                                GTK_TOGGLE_BUTTON(check_button_fractal)))
            flags |= MDZ_FRACTAL_SETTINGS;

        if (gtk_toggle_button_get_active(
                                GTK_TOGGLE_BUTTON(check_button_palette)))
            flags |= MDZ_PALETTE_DATA;

        filename = gtk_file_chooser_get_filename(
                                        GTK_FILE_CHOOSER(dl));

        gui_stop_rendering(img);

        DMSG("rendering stopped, proceeding to load image info...\n");

        good = image_info_load_all(img, flags, filename);
        g_free(filename);
    }

    gtk_widget_destroy (dl);
    return good;
}


void image_info_dlg_set(image_info* img, image_info_dialog* dl)
{
    DMSG("image_info_dlg_set\n");
    g_signal_handler_block(G_OBJECT(dl->width), width_sig);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->width),
                                              img->user_width);
    g_signal_handler_unblock(G_OBJECT(dl->width), width_sig);
    g_signal_handler_block(G_OBJECT(dl->height), height_sig);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->height),
                                              img->user_height);
    g_signal_handler_unblock(G_OBJECT(dl->height), height_sig);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->aa),  img->aa_factor);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dl->const_ra), TRUE);
    update_text(GTK_LABEL(dl->text),
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->width)),
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->height)),
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dl->aa)));

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dl->use_multi_prec),
                                                    img->use_multi_prec);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dl->use_rounding),
                                                    img->use_rounding);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->precision),
                                              img->precision);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dl->thread_count),
                                              img->thread_count);
}
