#ifndef ZOOM_H
#define ZOOM_H

#include <gtk/gtk.h>

void zoom_gui_create_buttons(GtkBox* box);

void zoom_gui_process_zooming_click(int button);
void zoom_gui_process_zooming_1click_click(int button);
void zoom_gui_kill_timers(void);

void zoom_gui_sensitive_zoom_in_button(gboolean t);
void zoom_gui_sensitive_zoom_out_button(gboolean t);
void zoom_gui_sensitive_zoom_in_1click_button(gboolean t);

void zoom_gui_draw_zoom_box(void);

#endif
