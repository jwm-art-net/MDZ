#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <png.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <libgen.h>


#include "palette.h"
#include "my_png.h"
#include "main_gui.h"
#include "misc_gui.h"
#include "last_used.h"

/* should be using autoconf... */
#if PNG_LIBPNG_VER <= 10002
#error Your libpng is too old. Upgrade to at least version 1.0.3
#endif


void save_png_file(image_info* img, char* filename)
{
    FILE* fp;
    png_struct* png_ptr;
    png_info* info_ptr;

    bool interlace;       /* do we want interlacing. FALSE for now */
    int i;

    unsigned char** row_p = NULL;

    interlace = FALSE;

    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Can't open file %s: %s\n", filename,
                strerror(errno));
        return;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL,
                                      NULL);
    if (png_ptr == NULL)
    {
        fprintf(stderr, "Can't create png_ptr structure\n");
        fclose(fp);

        return;
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (info_ptr == NULL)
    {
        fprintf(stderr, "Can't create info_ptr structure\n");
        fclose(fp);
        png_destroy_write_struct(&png_ptr, NULL);

        return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fprintf(stderr, "Internal error in libpng\n");

        if (row_p)
            free(row_p);

        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return;
    }

    png_init_io(png_ptr, fp);

    /* possible compression level setting here */
    /* png_set_compression_level(png_ptr, 1-9); */

    png_set_IHDR(png_ptr, info_ptr, img->user_width, img->user_height,
            8,  PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);
    png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

    /* initialize row pointers */
    row_p = malloc(sizeof(guchar*) * img->user_height);

    for (i=0; i < img->user_height; i++)
        row_p[i] = (guchar*)&(img->rgb_data[i * img->user_width]);

    /* write image */
    png_write_image(png_ptr, row_p);

    png_write_end(png_ptr, info_ptr);

    free(row_p);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

void do_png_save(image_info* img)
{
    GtkWidget* dialog =
        gtk_file_chooser_dialog_new("Save as PNG",
                        gui_window(),
                        GTK_FILE_CHOOSER_ACTION_SAVE,
                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                        GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                        NULL);

    char* free_luf = 0;

    const char* lud = last_used_suggest_dir(LU_PNG);

    if (lud)
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), lud);

    const char* luf = last_used_suggest_filename(LU_PNG, NULL);

    if (luf)
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), luf);

    gui_file_chooser_add_filter(dialog, "PNG files", "*.png");
    gui_file_chooser_add_filter(dialog, "All files", "*");

    gtk_file_chooser_set_do_overwrite_confirmation (
                        GTK_FILE_CHOOSER (dialog), TRUE);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        filename = gtk_file_chooser_get_filename(
                        GTK_FILE_CHOOSER(dialog));
        save_png_file(img, filename);
        last_used_set_file(LU_PNG, filename);
        g_free(filename);
    }
    if (free_luf)
        free(free_luf);
    gtk_widget_destroy (dialog);
}

