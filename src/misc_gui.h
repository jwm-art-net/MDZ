#ifndef MISC_GUI_H
#define MISC_GUI_H


#include <gtk/gtk.h>


typedef void (*button_clicked_cb)(GtkButton*, gpointer user_data);


void gui_file_chooser_add_filter(GtkWidget* chooser, const char* name,
                                                    const char* pattern);


GtkWidget* gui_button_new(const char* label, button_clicked_cb,
                                                GtkWidget* box,
                                                gpointer user_data);

GtkWidget* gui_slider_new(const char* label,    double iniv,
                                                double minv,
                                                double maxv,
                                                double stepv,
                                                GtkWidget* table,
                                                int y,
                                                int slider_span     );

#endif

