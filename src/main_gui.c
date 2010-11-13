#include "main_gui.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "colour_gui.h"
#include "coords_gui.h"
#include "debug.h"
#include "fractal.h"
#include "fractset_gui.h"
#include "image_info_gui.h"
#include "main.h"
#include "misc_gui.h"
#include "my_png.h"
#include "render.h"
#include "render_threads.h"
#include "reposition.h"
#include "timer.h"
#include "zoom.h"

#define TEXT_RECALC         "Recalc"
#define TEXT_STOP           "Stop"

#define JPRE_SIZE           160
#define JPRE_AAFACTOR       2

status_info stat;


image_info* j_pre =         NULL;
GtkWidget* drawing_area =   NULL;
GtkWidget* window =         NULL;
coords_dialog* coords_dlg = NULL;
palette_gui*  palgui =      NULL;

static fracset_dialog* fracgui = NULL;

static int gui_idle_draw_id;

static GtkWidget* j_pre_window =            NULL;
static GtkWidget* recalc_button_label =     NULL;
static GtkWidget* depth_spin =              NULL;
static GtkWidget* pbar =                    NULL;
static GtkWidget* switch_menu_cmd =         NULL;
static GtkWidget* zoom_new_win =            NULL;

    GtkWidget* menu_bar;

static image_info_dialog*   img_info_dlg =  NULL;


static gint idle_draw_callback(image_info* img);

static void gui_stop_rendering(image_info* img);
static void start_julia_browsing(void);
static void stop_julia_browsing(void);

static void update_markings(void);


static void quit(void);
static void redraw_image(image_info* img);

static void create_menus(GtkWidget* vbox);
static GtkWidget* menu_add(GtkWidget* menu, char* name, 
                        void (*func)(void));

static GdkRectangle horiz_intersect(GdkRectangle* a1, GdkRectangle* a2);

static gint expose_event(GtkWidget* widget, GdkEventExpose* event,
                         image_info* img);

static gint button_press_event(GtkWidget* widget, GdkEventButton* event);
static gint j_pre_delete(GtkWidget *widget, GdkEvent *event,
                        gpointer data);

/*  commenting this out until I figure out what it's supposed to do
static gint child_reaper(gpointer nothing);
*/
static void switch_fractal_type(void);


static void do_coords_dialog(void);
static void do_reset_zoom(void);
static void toggle_zoom_new_win(GtkWidget* widget);

static void do_pal_edit_dialog(void);
static void do_fracset_dialog(void);


void gui_sensitive_switch_menu(gboolean t)
    {   gtk_widget_set_sensitive(switch_menu_cmd, t);    }


void save_png_cmd(void)
{
    do_png_save(img);
}

void load_settings_cmd(void)
{
    if (do_image_info_load_dialog(img))
    {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(depth_spin),
                                                        img->depth);
        gtk_spin_button_update(GTK_SPIN_BUTTON(depth_spin));

        if (img->user_width < MIN_WINDOW_WIDTH)
            gtk_widget_set_size_request(drawing_area,
                                MIN_WINDOW_WIDTH, img->user_height);
        else
            gtk_widget_set_size_request(drawing_area,
                                img->user_width, img->user_height);

        if (img_info_dlg)
            image_info_dlg_set(img, img_info_dlg);

        if (palgui)
            palette_gui_update(palgui, img);

        gui_start_rendering(img);
    }
}

void save_settings_cmd(void)
{
    do_image_info_save_dialog(img);
}

void switch_fractal_type(void)
{
    if (img->family == FAMILY_MANDEL
     && stat.action != STAT_JULIA_BROWSING)
    {
        start_julia_browsing();
    }
    else
    {
        image_info_switch_fractal(img, 0, 0);
        gui_start_rendering(img);
    }
}


void gui_reapply_palette(void)
{
    if (img->aa_factor == 1)
        palette_apply(img, 0, 0, img->user_width, img->user_height);
    else
        do_anti_aliasing(img, 0, 0, img->user_width, img->user_height);
    redraw_image(img);
}


/* returns the horizontal intersection part of a1 and a2. if the
   rectangles don't overlap, it returns a rectangle with a width of 0.
*/
GdkRectangle horiz_intersect(GdkRectangle* a1, GdkRectangle* a2)
{
    GdkRectangle tmp;
    int ax1, ax2, bx1, bx2, cx1, cx2;
    
    tmp.width = 0;
    tmp.y = a1->y;
    tmp.height = a1->height;

    ax1 = a1->x;
    ax2 = a1->x + a1->width - 1;

    bx1 = a2->x;
    bx2 = a2->x + a2->width - 1;

    cx1 = (ax1 > bx1) ? ax1 : bx1; /* max */
    cx2 = (ax2 < bx2) ? ax2 : bx2; /* min */

    /* no overlap */
    if (cx2 < cx1)
        return tmp;

    tmp.x = cx1;
    tmp.width = cx2-cx1+1;
        
    return tmp;
}

void redraw_image(image_info* img)
{
    gdk_draw_rgb_32_image(img->drawing_area->window,
                          img->drawing_area->style->white_gc,
                          0, 0, img->user_width, img->user_height,
                          GDK_RGB_DITHER_NONE,
                          (guchar*)img->rgb_data,
                          img->user_width * sizeof(guint32));
}

gint do_palette_rotation(gpointer _dir)
{
    int dir = *((int*)_dir);

    if (dir > 0)
        palette_rotate_forward();
    else if (dir < 0)
        palette_rotate_backward();

    if (img->aa_factor == 1)
        palette_apply(img, 0, 0, img->user_width, img->user_height);
    else
        do_anti_aliasing(img, 0, 0, img->user_width, img->user_height);

    redraw_image(img);

    if (dir)
        palette_gui_rot_update(palgui);

    return TRUE;
}

void palette_load_cmd(void)
{
    GtkWidget* dialog =
        gtk_file_chooser_dialog_new("Open palette MAP file",
                        (GtkWindow*)window,
                        GTK_FILE_CHOOSER_ACTION_OPEN,
                        GTK_STOCK_CANCEL,   GTK_RESPONSE_CANCEL,
                        GTK_STOCK_OPEN,     GTK_RESPONSE_ACCEPT,
                        GTK_STOCK_APPLY,    GTK_RESPONSE_APPLY,
                        NULL);

    char** pal_paths = palette_get_paths();

    int n = 0;
    while(pal_paths[n])
    {
        if (!gtk_file_chooser_add_shortcut_folder((GtkFileChooser*)dialog,
                pal_paths[n], 0))
        {
            fprintf(stderr,"Could not add folder '%s'\n", pal_paths[n]);
        }
        ++n;
    }

    gui_file_chooser_add_filter(dialog, "MAP files", "*.map");
    gui_file_chooser_add_filter(dialog, "All files", "*");

    gint dlg_resp;

    do
    {
        dlg_resp = gtk_dialog_run(GTK_DIALOG(dialog));

        if ( dlg_resp == GTK_RESPONSE_ACCEPT
          || dlg_resp == GTK_RESPONSE_APPLY)
        {
            char *filename;

            filename = gtk_file_chooser_get_filename(
                                    GTK_FILE_CHOOSER(dialog));

            if (palette_load(filename) == FALSE)
            {
                fprintf(stderr, "Invalid palette file %s\n", filename);
            }
            else
            {
                if (img->aa_factor == 1)
                    palette_apply(img, 0, 0,    img->user_width,
                                                img->user_height);
                else
                    do_anti_aliasing(img, 0, 0, img->user_width, 
                                                img->user_height);
                redraw_image(img);

                if (palgui)
                    palette_gui_update(palgui, img);
            }
            g_free(filename);
        }

    } while (dlg_resp == GTK_RESPONSE_APPLY);

    gtk_widget_destroy (dialog);
}

void palette_save_cmd(void)
{
    GtkWidget* dialog =
        gtk_file_chooser_dialog_new("Save palette MAP file",
                        gui_window(),
                        GTK_FILE_CHOOSER_ACTION_SAVE,
                        GTK_STOCK_CANCEL,   GTK_RESPONSE_CANCEL,
                        GTK_STOCK_SAVE,     GTK_RESPONSE_ACCEPT,
                        NULL);
    gtk_file_chooser_set_do_overwrite_confirmation (
                        GTK_FILE_CHOOSER (dialog), TRUE);

    gui_file_chooser_add_filter(dialog, "MAP files", "*.map");
    gui_file_chooser_add_filter(dialog, "All files", "*");

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        filename = gtk_file_chooser_get_filename(
                        GTK_FILE_CHOOSER(dialog));
        palette_save(filename);
        g_free(filename);
    }
    gtk_widget_destroy (dialog);
}

void do_palette_randomize(random_palette* rnd_pal)
{
    palette_randomize(rnd_pal);
    if (img->aa_factor == 1)
        palette_apply(img, 0, 0, img->user_width, img->user_height);
    else
        do_anti_aliasing(img, 0, 0, img->user_width, img->user_height);
    redraw_image(img);
}

void do_palette_function(function_palette* fun_pal)
{
    palette_apply_func(fun_pal);
    if (img->aa_factor == 1)
        palette_apply(img, 0, 0, img->user_width, img->user_height);
    else
        do_anti_aliasing(img, 0, 0, img->user_width, img->user_height);
    redraw_image(img);
}

/* change preview window to reflect possible new aspect ratio */
void gui_resize_preview(void)
{
    int xw,yw;
    
    if (JPRE_SIZE/img->aspect < JPRE_SIZE) {
        xw = JPRE_SIZE;
        yw = JPRE_SIZE/img->aspect;
        if (yw == 0)
            yw = 1;
    }
    else {
        xw = JPRE_SIZE*img->aspect;
        if (xw == 0)
            xw = 1;
        yw = JPRE_SIZE;
    }
    
    image_info_set(j_pre, xw, yw, JPRE_AAFACTOR);
    gtk_widget_set_size_request(j_pre->drawing_area, xw, yw);
}


void do_image_info_dialog(void)
{
    if (img_info_dlg)
        return;

    image_info_dlg_new(&img_info_dlg, img);
    g_signal_connect(GTK_OBJECT(img_info_dlg->ok_button), "clicked",
                       G_CALLBACK(image_info_ok_cmd),
                       img_info_dlg);
    g_signal_connect(GTK_OBJECT(img_info_dlg->apply_button), "clicked",
                       G_CALLBACK(image_info_apply_cmd),
                       img_info_dlg);
}

void do_coords_dialog(void)
{
    if (coords_dlg)
        return;

    coords_dlg_new(&coords_dlg, img);

    g_signal_connect(GTK_OBJECT(coords_dlg->ok_button), "clicked",
                       G_CALLBACK(coords_ok_cmd),
                       coords_dlg);
    g_signal_connect(GTK_OBJECT(coords_dlg->apply_button), "clicked",
                       G_CALLBACK(coords_apply_cmd),
                       coords_dlg);
}

static void do_fracset_dialog(void)
{
    if (fracgui)
        return;

    fracset_dlg_new(&fracgui, img);

    g_signal_connect(GTK_OBJECT(fracgui->ok_button), "clicked",
                       G_CALLBACK(fracset_dlg_ok_cmd),
                       fracgui);
    g_signal_connect(GTK_OBJECT(fracgui->apply_button), "clicked",
                       G_CALLBACK(fracset_dlg_apply_cmd),
                       fracgui);
}


static void do_reset_zoom(void)
{
    image_info_reset_view(img);

    gtk_spin_button_set_value((GtkSpinButton*)depth_spin, img->depth);

    if (img_info_dlg)
        image_info_dlg_set(img, img_info_dlg);

    gui_start_rendering(img);

    if (stat.action == STAT_JULIA_BROWSING)
    {
        gtk_widget_hide(j_pre_window);
        gtk_widget_show(j_pre_window);
        gui_start_rendering(j_pre);
    }
}


void do_pal_edit_dialog(void)
{
    if (palgui)
        return;
    palette_gui_new(&palgui, img);
}

/*  commenting this out until I figure out what it's supposed to do

--  ie, what are these child processes? not the duplicate or
    zoom-in-new-window windows - because they're not killed when
    this is called - and anyway, i don't want them killed just because
    i close the window which spawned them, so wtf!?!?

--  gfract in c heritage

gint child_reaper(gpointer nothing)
{
    printf("in child_reaper\n");
    // wait until all dead child processes are cleaned up
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    return TRUE;
}
*/

GtkWidget* menu_add(GtkWidget* menu, char* name, void (*func)(void))
{
    GtkWidget* item;

    if (name != NULL)
    {
        item = gtk_menu_item_new_with_label(name);
        g_signal_connect_object(GTK_OBJECT(item), "activate",
                        G_CALLBACK(func), NULL, G_CONNECT_SWAPPED);
    }
    else
        /* just add a separator line to the menu */
        item = gtk_menu_item_new();

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_widget_show(item);

    return item;
}

void menu_bar_add(GtkWidget* menubar, GtkWidget* submenu, char* name)
{
    GtkWidget* temp = gtk_menu_item_new_with_label(name);
    gtk_widget_show(temp);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(temp), submenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), temp);
}

void create_menus(GtkWidget* vbox)
{
    GtkWidget* menu;

    menu_bar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);
    gtk_widget_show(menu_bar);

    menu = gtk_menu_new();
    menu_add(menu, "Load Settings", load_settings_cmd);
    menu_add(menu, "Save Settings", save_settings_cmd);
    menu_add(menu, NULL, NULL);
    menu_add(menu, "Duplicate", duplicate);
    menu_add(menu, NULL, NULL);
    menu_add(menu, "Exit", quit);
    menu_bar_add(menu_bar, menu, "File");

    menu = gtk_menu_new();
    menu_add(menu, "Save as PNG", save_png_cmd);
    menu_add(menu, NULL, NULL);
    menu_add(menu, "Fractal",           do_fracset_dialog);
    menu_add(menu, "Attributes...",     do_image_info_dialog);
    menu_add(menu, "Coordinates...",    do_coords_dialog);
    menu_add(menu, NULL, NULL);
    switch_menu_cmd = menu_add(menu, "Switch fractal type",
                             switch_fractal_type);
    menu_add(menu, NULL, NULL);
    menu_add(menu, "Reset Zoom", do_reset_zoom);

    zoom_new_win =
        gtk_check_menu_item_new_with_label("Zoom in new window");
    gtk_check_menu_item_set_active(
        GTK_CHECK_MENU_ITEM(zoom_new_win), img->zoom_new_win);
    g_signal_connect(GTK_OBJECT(zoom_new_win), "toggled",
        G_CALLBACK(toggle_zoom_new_win), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), zoom_new_win);
    gtk_widget_show(zoom_new_win);

    menu_bar_add(menu_bar, menu, "Image");

    menu = gtk_menu_new();

    menu_add(menu, "Colours", do_pal_edit_dialog);
    menu_add(menu, NULL, NULL);
    menu_add(menu, "Load MAP", palette_load_cmd);
    menu_add(menu, "Save MAP", palette_save_cmd);

    menu_bar_add(menu_bar, menu, "Palette");

    menu = gtk_hseparator_new();
    gtk_widget_show(menu);
    gtk_box_pack_start(GTK_BOX(vbox), menu, FALSE, FALSE, 0);
}

GtkWidget* gui_create_pixmap(GtkWidget* widget, char** xpm_data)
{
    GdkPixmap* pixmap;
    GdkBitmap* mask;
    GtkWidget* pwid;

    pixmap = gdk_pixmap_create_from_xpm_d(widget->window, &mask,
             &(gtk_widget_get_style(widget)->bg[GTK_STATE_NORMAL]),
             (gchar**)xpm_data);
    g_assert(pixmap != NULL);
    pwid = gtk_image_new_from_pixmap(pixmap, mask);
    gtk_widget_show(pwid);

    return pwid;
}

gint idle_draw_callback(image_info* img)
{
    rthdata* rth = (rthdata*)img->rth_ptr;
    int linesdone = rth_process_lines_rendered(rth);
    if (!linesdone)
        return TRUE;

    int aa = img->aa_factor;
    int y;
    int miny = rth->min_line_drawn;
    int maxy = miny + rth->line_draw_count + 1;
    int undrawn = 0;
    int width = img->user_width;

    if (maxy >= img->user_height)
        maxy = img->user_height;

    if (linesdone > 0 && maxy > linesdone)
        maxy = linesdone;

    char* ld = &rth->lines_drawn[miny];

    if (stat.ptr_in_drawable)
        update_markings();

    for (y = miny; y < maxy; ++y, ++ld)
    {
        if (*ld == 1)
        {
            *ld = 2;
            if (!undrawn)
                rth->min_line_drawn = y;

            if (aa == 1)
                palette_apply(img, 0, y, width, 1);
            else
                do_anti_aliasing(img, 0, y, width, 1);

            gdk_draw_rgb_32_image(img->drawing_area->window,
                                  img->drawing_area->style->white_gc,
                                  0, y, width, 1,
                                  GDK_RGB_DITHER_NONE,
                                  (guchar*)&img->rgb_data[y * width],
                                  width * sizeof(guint32));
        }
        else if (*ld == 0)
            undrawn = 1;
    }

    if (stat.ptr_in_drawable)
        update_markings();

    if (!img->j_pre)
        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(pbar),
                (gfloat)y / (gfloat)img->user_height);

    if (y >= img->user_height)
    {
        rth_ui_stop_timer(rth);
        gui_stop_rendering(img);
        if (opts.logfd && img != j_pre)
            fprintf(opts.logfd, "render-time %.3fs\n",
                            rth_ui_get_render_time(rth));
        return FALSE;
    }
    return TRUE;
}

void gui_start_rendering(image_info* img)
{
    DMSG("gui_start_rendering");

    coords_get_rect(img->pcoords,   img->xmin, img->xmax,
                                    img->ymax, img->width);

    coords_get_rect_gmp(img->pcoords,   img->gxmin, img->gxmax,
                                        img->gymax, img->gwidth);

    if (!img->j_pre)
    {
        gtk_spin_button_update( GTK_SPIN_BUTTON(depth_spin));
        img->depth = gtk_spin_button_get_value_as_int(
                                GTK_SPIN_BUTTON(depth_spin));
        if (opts.logfd)
        {
            image_info_save_settings(img, opts.logfd);
            #ifdef WITH_TMZ_CMDLINE
            image_tmz_save_settings(img, opts.logfd);
            #endif
        }

        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar), 1.0);
        gtk_widget_show(pbar);
        gtk_label_set_text(GTK_LABEL(recalc_button_label), TEXT_STOP);

        /*  there's a good change the render has different coordinates
            to what are shown in the coords dialog. but even if not,
            there's no harm done in updating it anyway...
        */

        if (coords_dlg)
            coords_dlg_set(img, coords_dlg);

        if (img_info_dlg)
            image_dlg_update_recommended_precision(img_info_dlg, img);
    }

    rth_ui_start_render((rthdata*)img->rth_ptr);
    rth_ui_wait_until_started((rthdata*)img->rth_ptr);

    gui_idle_draw_id = g_idle_add((GtkFunction)idle_draw_callback, img);
}

void gui_stop_rendering(image_info* img)
{
    DMSG("gui_stop_rendering");
    rth_ui_stop_render((rthdata*)img->rth_ptr);

    if (gui_idle_draw_id != -1)
    {
        g_source_remove(gui_idle_draw_id);
        gui_idle_draw_id = -1;
        if (!img->j_pre)
        {
            gtk_widget_hide(pbar);
            gtk_label_set_text(
                GTK_LABEL(recalc_button_label), TEXT_RECALC);
        }
    }
}

void start_julia_browsing(void)
{
    stat.action = STAT_JULIA_BROWSING;
    j_pre->fractal = img->fractal;
    gtk_widget_show(j_pre_window);
    gtk_widget_set_sensitive(switch_menu_cmd, FALSE);

    zoom_gui_sensitive_zoom_in_button(FALSE);
    zoom_gui_sensitive_zoom_out_button(FALSE);
    zoom_gui_sensitive_zoom_in_1click_button(FALSE);

    repos_gui_sensitive_reposition_button(FALSE);
}

void stop_julia_browsing(void)
{
    gui_stop_rendering(j_pre);
    stat.action = STAT_NORMAL;
    gtk_widget_hide(j_pre_window);
    gtk_widget_set_sensitive(switch_menu_cmd, TRUE);

    zoom_gui_sensitive_zoom_in_button(TRUE);
    zoom_gui_sensitive_zoom_out_button(TRUE);
    zoom_gui_sensitive_zoom_in_1click_button(TRUE);

    repos_gui_sensitive_reposition_button(TRUE);
}

gint j_pre_delete(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    (void)widget; (void)event; (void)data;
    stop_julia_browsing();
    return TRUE;
}

void gui_draw_center_lines(int draw_cl)
{
    int x = 0, y = 0;
    GdkLineStyle linestyle = GDK_LINE_ON_OFF_DASH;
    if (!draw_cl)
    {
        x = stat.fp_x;
        y = stat.fp_y;
    }
    else
    {
        x = stat.cl_x;
        y = stat.cl_y;
        if (stat.action == STAT_REPOSITIONING && stat.reposition_p2)
            linestyle = GDK_LINE_SOLID;
    }
    gdk_gc_set_function(drawing_area->style->white_gc, GDK_XOR);
    gdk_gc_set_line_attributes(drawing_area->style->white_gc,
            1, linestyle, GDK_CAP_BUTT, GDK_JOIN_MITER);
    gdk_draw_line(drawing_area->window, drawing_area->style->white_gc,
            0, y, img->user_width, y);
    gdk_draw_line(drawing_area->window, drawing_area->style->white_gc,
            x, 0, x, img->user_height);
    gdk_gc_set_function(drawing_area->style->white_gc, GDK_COPY);
}

void recalc_button(GtkWidget* widget)
{
    (void)widget;
    if (gui_idle_draw_id == -1)
        gui_start_rendering(img);
    else
        gui_stop_rendering(img);
}

void toggle_zoom_new_win(GtkWidget* widget)
{
    (void)widget;
    img->zoom_new_win = gtk_check_menu_item_get_active(
        GTK_CHECK_MENU_ITEM(zoom_new_win));
}

gint button_press_event(GtkWidget* widget, GdkEventButton* event)
{
    (void)widget;
    /* ignore double and triple clicks */
    if ( (event->type == GDK_2BUTTON_PRESS) ||
         (event->type == GDK_3BUTTON_PRESS) )
        return TRUE;

    /* don't react to pressing the other button if we're
       zooming in or out */
    if ((stat.zoom_timer_id != -1) &&
        ((event->button == 1) || (event->button == 3)) )
        return TRUE;

    if (stat.action == STAT_ZOOMING)
        zoom_gui_process_zooming_click(event->button);

    else if (stat.action == STAT_ZOOMING_1CLICK)
        zoom_gui_process_zooming_1click_click(event->button);

    else if (stat.action == STAT_JULIA_BROWSING)
    {
        if (event->button == 1)
        {
            stop_julia_browsing();
            image_info_switch_fractal(img, event->x, event->y);
            gui_start_rendering(img);
        }
    }
    else if (stat.action == STAT_REPOSITIONING)
        repos_gui_process_repositioning_click(event);

    return TRUE;
}

gint button_release_event(GtkWidget* widget, GdkEventButton* event)
{
    (void)widget; (void)event;
    zoom_gui_kill_timers();
    return TRUE;
}

gint motion_event(GtkWidget* widget, GdkEventMotion* event)
{
    (void)widget;
    if (stat.action == STAT_JULIA_BROWSING)
    {
        mpfr_set_d(j_pre->u.julia.c_re, event->x,   GMP_RNDN);
        mpfr_set_d(j_pre->u.julia.c_im, event->y,   GMP_RNDN);

        coords_pixel_to_coord(img->pcoords, j_pre->u.julia.c_re,
                                            j_pre->u.julia.c_im  );
        gui_start_rendering(j_pre);
    }
    else if (stat.action == STAT_ZOOMING)
    {
        zoom_gui_draw_zoom_box();
        stat.z_x = event->x;
        stat.z_y = event->y;
        zoom_gui_draw_zoom_box();
    }
    else if (stat.action == STAT_REPOSITIONING)
    {
        gui_draw_center_lines(TRUE);
        stat.cl_x = event->x;
        stat.cl_y = event->y;
        gui_draw_center_lines(TRUE);
    }
    else if (stat.action == STAT_ZOOMING_1CLICK)
    {
        zoom_gui_draw_zoom_box();
        stat.z_x = event->x;
        stat.z_y = event->y;
        zoom_gui_draw_zoom_box();
    }
    return TRUE;
}

gint notify_enter_leave_event(GtkWidget* widget, GdkEventCrossing* event)
{
    (void)widget;
    update_markings();
    if (event->type == GDK_ENTER_NOTIFY)
        stat.ptr_in_drawable = 1;
    else
        stat.ptr_in_drawable = 0;
    return TRUE;
}

void update_markings(void)
{
    switch(stat.action)
    {
    case STAT_ZOOMING:
    case STAT_ZOOMING_1CLICK:
        zoom_gui_draw_zoom_box();
        break;
    case STAT_REPOSITIONING:
        if (stat.reposition_p2)
            gui_draw_center_lines(FALSE);
        gui_draw_center_lines(TRUE);
    default:
        break;
    }
}

gint expose_event(GtkWidget* widget, GdkEventExpose* event,
                  image_info* img)
{
    GdkRectangle pic_area, padding_area,tmp;

    if (stat.ptr_in_drawable)
        update_markings();

    /* sometimes gtk gives us invalid exposes when we're changing
       image size, and when that happens, draw_image below segfaults
       when it's trying to access rgb_data at offsets way past the
       current image size.

       so check for that and don't do anything on invalid exposes. */
    if ((event->area.y + event->area.height - 1)
            > (img->user_height - 1))
    {
        return TRUE;
    }

    tmp.x = 0;
    tmp.width = img->user_width;
    pic_area = horiz_intersect(&event->area, &tmp);

    tmp.x = img->user_width;
    tmp.width = USHRT_MAX;
    padding_area = horiz_intersect(&event->area, &tmp);

    if (pic_area.width != 0)
    {
        gdk_draw_rgb_32_image(widget->window,
                              widget->style->white_gc,
                              pic_area.x, pic_area.y,
                              pic_area.width, pic_area.height,
                              GDK_RGB_DITHER_NONE,
                              (guchar*)&(img->rgb_data[pic_area.y *
                                                      img->user_width
                                                      + pic_area.x]),
                              img->user_width*4);
    }

    if (padding_area.width != 0)
    {
        gdk_draw_rectangle(widget->window,
                           widget->style->white_gc,
                           TRUE,
                           padding_area.x, padding_area.y,
                           padding_area.width, padding_area.height);
    }

    if (stat.ptr_in_drawable)
        update_markings();

    return TRUE;
}

void quit(void)
{
    gtk_main_quit();
}

int gui_init(int* argc, char*** argv, image_info* img_)
{
    GtkWidget* vbox;
    GtkWidget* hbox;
    GtkWidget* button;
    GtkWidget* tmp;
    GtkObject* adj;

    char title[80];
    snprintf(title, 79, "mdz-%s", VERSION);

    if (!gtk_init_check(argc, argv))
    {
        fprintf(stderr, "Failed to initilize GUI!\n");
        return FALSE;
    }

    img = img_;

    j_pre = image_info_create(FAMILY_JULIA, MANDELBROT);

    stat.action = STAT_NORMAL;
    stat.ptr_in_drawable = 0;
    stat.reposition_p2 = FALSE;
    stat.zoom_timer_id = -1;

/*  commenting this out until I figure out what it's supposed to do
    g_timeout_add(10*1000, child_reaper, NULL);
*/

    image_info_set(j_pre,
                        JPRE_SIZE,
                        JPRE_SIZE / img->aspect, JPRE_AAFACTOR);

    /* main window */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_widget_realize(window);

    /* preview window */
    j_pre_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(GTK_OBJECT(j_pre_window), "delete_event",
                       G_CALLBACK(j_pre_delete), NULL);
    gtk_window_set_title(GTK_WINDOW(j_pre_window), "Preview");
    gtk_window_set_resizable(GTK_WINDOW(j_pre_window), FALSE);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_widget_show (vbox);

    create_menus(vbox);
    g_signal_connect (GTK_OBJECT (window), "destroy",
                        G_CALLBACK (quit), NULL);

    /* toolbar stuff */

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    zoom_gui_create_buttons(GTK_BOX(hbox));

    repos_gui_create_buttons(GTK_BOX(hbox));

    /* depth label */
    button = gtk_label_new("Iters");
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
    gtk_widget_show(button);

    /* depth spin-button */
    adj = gtk_adjustment_new((gfloat)img->depth,
                                MIN_DEPTH,
                                MAX_DEPTH, 1, 16, 0.0);
    depth_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0.0, 0.0);
    gtk_widget_set_tooltip_text(depth_spin, "max iterations");
    gtk_box_pack_start(GTK_BOX(hbox), depth_spin, FALSE, FALSE, 0);
    gtk_widget_show(depth_spin);

    /* recalc button */
    button = gtk_button_new();
    recalc_button_label = gtk_label_new(TEXT_STOP);
    gtk_misc_set_alignment(GTK_MISC(recalc_button_label),
                           0.5, 0.5);
    gtk_container_add(GTK_CONTAINER(button), recalc_button_label);
    gtk_widget_show(recalc_button_label);
    g_signal_connect(GTK_OBJECT(button), "clicked",
                       G_CALLBACK(recalc_button), NULL);
    gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
    gtk_widget_show(button);

    /* progress bar */
    pbar = gtk_progress_bar_new();
    gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
                                     GTK_PROGRESS_LEFT_TO_RIGHT);
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(pbar), 
                               (gfloat)1.0 ); /* img->real_height); */
    gtk_widget_set_size_request(pbar, 75, 0);
    gtk_box_pack_end(GTK_BOX(hbox), pbar, FALSE, FALSE, 0);

    /* separator */
    button = gtk_hseparator_new();
    gtk_widget_show(button);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    /* main window drawing area */
    tmp = gtk_drawing_area_new();
    gtk_widget_set_events (tmp, GDK_BUTTON_PRESS_MASK
                           | GDK_BUTTON_RELEASE_MASK
                           | GDK_POINTER_MOTION_MASK
                           | GDK_ENTER_NOTIFY_MASK
                           | GDK_LEAVE_NOTIFY_MASK
                           | GDK_EXPOSURE_MASK);
    gtk_widget_set_size_request(tmp, 
                        (img->user_width >= MIN_WINDOW_WIDTH)
                        ? img->user_width : MIN_WINDOW_WIDTH,
                        img->user_height);
    gtk_box_pack_start(GTK_BOX(vbox), tmp, TRUE, TRUE, 0);
    gtk_widget_show(tmp);
    g_signal_connect(GTK_OBJECT(tmp), "button_press_event",
                        G_CALLBACK(button_press_event), NULL);
    g_signal_connect(GTK_OBJECT(tmp), "button_release_event",
                        G_CALLBACK(button_release_event), NULL);
    g_signal_connect(GTK_OBJECT(tmp), "expose_event",
                       G_CALLBACK(expose_event), img);
    g_signal_connect(GTK_OBJECT(tmp), "motion_notify_event",
                       G_CALLBACK(motion_event), NULL);
    g_signal_connect(GTK_OBJECT(tmp), "enter_notify_event",
                       G_CALLBACK(notify_enter_leave_event), NULL);
    g_signal_connect(GTK_OBJECT(tmp), "leave_notify_event",
                       G_CALLBACK(notify_enter_leave_event), NULL);


    img->drawing_area = drawing_area = tmp;

    /* preview window drawing area */
    tmp = gtk_drawing_area_new();
    gtk_widget_set_events (tmp, GDK_EXPOSURE_MASK);
    gtk_widget_set_size_request(tmp,    j_pre->user_width,
                                        j_pre->user_height);
    gtk_container_add(GTK_CONTAINER(j_pre_window), tmp);
    g_signal_connect(GTK_OBJECT(tmp), "expose_event",
                     G_CALLBACK(expose_event), j_pre);
    gtk_widget_show(tmp);
    j_pre->drawing_area = tmp;

    rth_ui_init((rthdata*)img->rth_ptr);
    rth_ui_init((rthdata*)j_pre->rth_ptr);

    gui_start_rendering(img);
    gtk_widget_show(window);

    gtk_main();

    rth_ui_quit((rthdata*)j_pre->rth_ptr);
    rth_ui_quit((rthdata*)img->rth_ptr);

    image_info_destroy(j_pre);

    return TRUE;
}

void gui_close_display()
{
    close(ConnectionNumber(gdk_display));
}

void gui_resize(int width, int height)
{
    gtk_widget_set_size_request(drawing_area, width, height);
}

gboolean gui_is_julia_browsing(void)
{
    return stat.action == STAT_JULIA_BROWSING;
}

void gui_show_julia_preview(void)
{
    gtk_widget_show(j_pre_window);
}

void gui_hide_julia_preview(void)
{
    gtk_widget_hide(j_pre_window);
}

GtkWindow* gui_window()
{
    return (GtkWindow*)window;
}
