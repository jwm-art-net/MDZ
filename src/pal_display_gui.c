#include "pal_display_gui.h"

#include "main_gui.h"

static int bandsize = 1.0;


static gboolean pal_display_expose_event(   GtkWidget *widget,
                                            GdkEventExpose *event,
                                            gpointer data)
{
    (void)event;(void)data;
    int i, ix;
    cairo_t* cr;
    double r, g, b;

    cr = gdk_cairo_create(widget->window);

    cairo_set_line_width (cr, 2.0);

    for (i = 0; i < pal_indexes; ++i)
    {
        ix = (i + pal_offset) % pal_indexes;

        r = RED(palette[ix]);
        g = GREEN(palette[ix]);
        b = BLUE(palette[ix]);

        cairo_set_source_rgb(cr, r / 256.0, g / 256.0, b / 256.0);
        cairo_move_to(cr, i * bandsize, 0);
        cairo_line_to(cr, i * bandsize, 31);
        cairo_stroke(cr);
    }

    cairo_destroy(cr);

    return TRUE;
}


static gboolean pal_display_motion_event(   GtkWidget *widget,
                                            GdkEventMotion *event,
                                            gpointer data)
{
    /*  note the GDK_BUTTON1_MOTION_MASK applied to the drawing_area
        means this callback is only called while mouse button 1
        is pressed.
    */

    (void)widget;
    int d;
    int cycle_dir_dummy = 0;
    pal_display* pd = (pal_display*)data;

    if (pd->org_x == -1)
        pd->org_x = event->x;

    pd->ptr_x = event->x;

    d = pd->org_x - event->x;

    if (!d)
        return TRUE;

    pd->org_x = event->x;

    palette_shift(pal_offset + d);
    gtk_widget_queue_draw(pd->drawing_area);

    do_palette_rotation(&cycle_dir_dummy);

    return TRUE;
}


static gboolean pal_display_button_event(   GtkWidget *widget,
                                            GdkEventButton *event,
                                            gpointer data)
{
    (void)widget;
    pal_display* pd = (pal_display*)data;

    if (event->button == 1 && event->type == GDK_BUTTON_PRESS)
        pd->org_x = -1;

    return TRUE;
}


pal_display* pal_display_new(GtkWidget* container)
{
    GtkWidget* tmp;

    pal_display* pd = malloc(sizeof(*pd));

    if (!pd)
        return 0;

    pd->ptr_x = 0;
    pd->org_x = 0;

    tmp = gtk_drawing_area_new();
    gtk_widget_set_size_request(tmp, pal_indexes * bandsize, 32);

    /*  GDK_POINT_MOTION_HINT_MASK reduces the number of motion
        events reacted to. For our use, this only has positive
        consequences (the palette is applied each time motion
        is detected).

        GDK_BUTTON1_MOTION_MASK means only motion events while
        button 1 is pressed will be detected.
    */

    gtk_widget_set_events (tmp, GDK_BUTTON_PRESS_MASK
                              | GDK_POINTER_MOTION_HINT_MASK
                              | GDK_BUTTON_RELEASE_MASK
                              | GDK_BUTTON1_MOTION_MASK
                              | GDK_EXPOSURE_MASK);

    g_signal_connect(GTK_OBJECT(tmp), "expose-event",
                    G_CALLBACK(pal_display_expose_event), pd);

    g_signal_connect(GTK_OBJECT(tmp), "motion-notify-event",
                    G_CALLBACK(pal_display_motion_event), pd);

    g_signal_connect(GTK_OBJECT(tmp), "button-press-event",
                    G_CALLBACK(pal_display_button_event), pd);

    gtk_container_add(GTK_CONTAINER(container), tmp);
    gtk_widget_show(tmp);
    pd->drawing_area = tmp;

    return pd;
}


void pal_display_free(pal_display* pd)
{
    free(pd);
}


void pal_display_update(pal_display* pd)
{
    gtk_widget_queue_draw(pd->drawing_area);
}




/************************************************************************
 ************************************************************************

    pal_affect

 ************************************************************************
 ************************************************************************/


static void pal_affect_changed_cb(GtkWidget* w, pal_affect* pa)
{
    int st, sp;

    st = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pa->stripe));
    sp = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pa->spread));

    if (w == pa->stripe)
    {
        if (st < sp)
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(pa->spread), st);
    }
    else if (w == pa->spread)
    {
        if (sp > st)
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(pa->spread), st);
    }
}


static GtkWidget* pal_affect_new_spin(  const char* label,
                                        double iniv,
                                        double minv,
                                        double maxv,
                                        pal_affect* pa,
                                        GtkWidget* table,
                                        int y)
{
    GtkWidget* tmp;
    GtkObject* adj;

    tmp = gtk_label_new(label);
    gtk_misc_set_alignment(GTK_MISC(tmp), 0.0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, y, y + 1);
    gtk_widget_show(tmp);

    adj = gtk_adjustment_new(iniv, minv, maxv, 1, 1, 0);
    tmp = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);

    g_signal_connect(GTK_OBJECT(tmp), "changed",
                       G_CALLBACK(pal_affect_changed_cb),
                       pa);

    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 1, 2, y, y + 1);
    gtk_widget_show(tmp);

    return tmp;
}


pal_affect* pal_affect_new(GtkWidget* container,    int* p_offset,
                                                    int* p_stripe,
                                                    int* p_spread)
{
    GtkWidget* frame;
    GtkWidget* table;
    int y = 0;

    pal_affect* pa = malloc(sizeof(*pa));

    if (!pa)
        return 0;

    frame = gtk_frame_new("Affect");
    gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
    gtk_container_add(GTK_CONTAINER(container), frame);

    table = gtk_table_new(3, 2, FALSE);
    gtk_container_add(GTK_CONTAINER(frame), table);

    pa->offset = pal_affect_new_spin("Offset", *p_offset, 0, 255,
                                                        pa, table, y++);

    pa->stripe = pal_affect_new_spin("Stripe", *p_stripe, 1, 255,
                                                        pa, table, y++);

    pa->spread = pal_affect_new_spin("Spread", *p_spread, 1, 255,
                                                        pa, table, y++);

    return pa;
}
