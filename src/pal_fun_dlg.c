#include "pal_fun_dlg.h"
#include "main_gui.h"
#include "misc_gui.h"


static void pal_fun_buttonfunc(GtkButton*, gpointer data);

static void set_fun_palette(palette_fun_dialog* dl);


void palette_fun_dlg_new(palette_fun_dialog** ptr, function_palette* fp)
{
    GtkWidget* vbox;
    GtkWidget* frame;
    GtkWidget* box;

    palette_fun_dialog* dl;

    dl = g_malloc(sizeof(palette_fun_dialog));
    dl->fun_pal = fp;
    *ptr = dl;

    dl->box = vbox = gtk_vbox_new(FALSE, 0);

    frame = gtk_frame_new("Functions");

    gtk_container_set_border_width(GTK_CONTAINER(frame),4);
    gtk_container_add(GTK_CONTAINER(vbox), frame);

    box = gtk_vbox_new(FALSE, 0);

    dl->ex_rg_button =
        gui_button_new("R <-> G",       pal_fun_buttonfunc, box, dl);
    dl->ex_gb_button =
        gui_button_new("G <-> B",       pal_fun_buttonfunc, box, dl);
    dl->ex_br_button =
        gui_button_new("B <-> R",       pal_fun_buttonfunc, box, dl);
    dl->rot_rgb_button =
        gui_button_new("Rotate RGB",    pal_fun_buttonfunc, box, dl);
    dl->inv_rgb_button =
        gui_button_new("Invert RGB",    pal_fun_buttonfunc, box, dl);
    dl->inv_r_button =
        gui_button_new("Invert Red",    pal_fun_buttonfunc, box, dl);
    dl->inv_g_button =
        gui_button_new("Invert Green",  pal_fun_buttonfunc, box, dl);
    dl->inv_b_button =
        gui_button_new("Invert Blue",   pal_fun_buttonfunc, box, dl);

    gtk_container_add(GTK_CONTAINER(frame), box);

    dl->pa = pal_affect_new(dl->box,    &dl->fun_pal->offset,
                                        &dl->fun_pal->stripe,
                                        &dl->fun_pal->spread);

    gtk_widget_show_all(dl->box);
}


static void pal_fun_buttonfunc(GtkButton* b, gpointer data)
{
    GtkWidget* w = GTK_WIDGET(b);
    palette_fun_dialog* dl = (palette_fun_dialog*)data;

    set_fun_palette(dl);

    if (w == dl->ex_rg_button)          dl->fun_pal->func = PF_EX_RG;
    else if (w == dl->ex_gb_button)     dl->fun_pal->func = PF_EX_GB;
    else if (w == dl->ex_br_button)     dl->fun_pal->func = PF_EX_BR;
    else if (w == dl->rot_rgb_button)   dl->fun_pal->func = PF_ROT_RGB;
    else if (w == dl->inv_rgb_button)   dl->fun_pal->func = PF_INV_RGB;
    else if (w == dl->inv_r_button)     dl->fun_pal->func = PF_INV_R;
    else if (w == dl->inv_g_button)     dl->fun_pal->func = PF_INV_G;
    else if (w == dl->inv_b_button)     dl->fun_pal->func = PF_INV_B;

    do_palette_function(dl->fun_pal);

    palette_gui_rot_update(palgui);
}

static void set_fun_palette(palette_fun_dialog* dl)
{
    dl->fun_pal->offset = atoi(gtk_entry_get_text(
                                            GTK_ENTRY(dl->pa->offset)));

    dl->fun_pal->stripe = atoi(gtk_entry_get_text(
                                            GTK_ENTRY(dl->pa->stripe)));

    dl->fun_pal->spread = atoi(gtk_entry_get_text(
                                            GTK_ENTRY(dl->pa->spread)));
}
