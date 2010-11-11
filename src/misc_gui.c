#include "misc_gui.h"


void gui_file_chooser_add_filter(GtkWidget* chooser, const char* name,
                                                     const char* pattern)
{
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, pattern);
    gtk_file_filter_set_name(filter, name);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
}


GtkWidget* gui_button_new(const char* label, button_clicked_cb cb,
                                                  GtkWidget* box,
                                                  gpointer data)
{
    GtkWidget* tmp;
    tmp = gtk_button_new_with_label(label);
    g_signal_connect(GTK_OBJECT(tmp), "clicked", G_CALLBACK(cb), data);
    gtk_box_pack_start(GTK_BOX(box), tmp, FALSE, FALSE, 0);
    return tmp;
}


GtkWidget* gui_slider_new(const char* label,    double iniv,
                                                double minv,
                                                double maxv,
                                                double stepv,
                                                GtkWidget* table,
                                                int y,
                                                int slider_span        )
{
    GtkWidget* tmp;

    tmp = gtk_label_new(label);
    gtk_misc_set_alignment(GTK_MISC(tmp), 1.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, y, y + 1);
    gtk_widget_show(tmp);

    tmp = gtk_hscale_new_with_range(minv, maxv, stepv);
    gtk_range_set_value(GTK_RANGE(tmp), iniv);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp,
                            1, 1 + slider_span, y, y + 1);
    gtk_widget_show(tmp);

    return tmp;
}
