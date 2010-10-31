#ifndef REPOSITION_H
#define REPOSITION_H

#include <gtk/gtk.h>

void repos_gui_create_buttons(GtkBox* box);
void repos_gui_sensitive_reposition_button(gboolean t);

void repos_gui_process_repositioning_click(GdkEventButton* event);

#endif
