#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <png.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#include "externs.h"
#include "palette.h"
#include "my_png.h"
#include "main_gui.h"
#include "misc_gui.h"

/* should be using autoconf... */
#if PNG_LIBPNG_VER <= 10002
#error Your libpng is too old. Upgrade to at least version 1.0.3
#endif


void save_png_file(image_info* img, char* filename)
{
    FILE* fp;
    png_struct* png_ptr;
    png_info* info_ptr;

    /*  by making gboolean pal volatile, we prevent the warning:
        variable 'pal' might be clobbered by 'longjmp' or 'vfork'.
    */
    volatile bool pal;             /* TRUE if img is palettized */
    bool interlace;       /* do we want interlacing. FALSE for now */
    int i;

    png_color*      png_pal = NULL;
    unsigned char*  pal_data = NULL;
    unsigned char** row_p = NULL;
    
    if (img->aa_factor == 1 && img->colour_scale == 1.0)
        pal = TRUE;
    else
        pal = FALSE;
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

        if (png_pal)
            free(png_pal);

        if (row_p)
            free(row_p);

        if (pal_data)
            free(pal_data);

        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return;
    }

    png_init_io(png_ptr, fp);

    /* possible compression level setting here */
    /* png_set_compression_level(png_ptr, 1-9); */

    png_set_IHDR(png_ptr, info_ptr, img->user_width, img->user_height, 8,
                 (pal)
                    ? PNG_COLOR_TYPE_PALETTE
                    : PNG_COLOR_TYPE_RGB,
                 (interlace)
                    ? PNG_INTERLACE_ADAM7
                    : PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    /* write palette */
    if (pal)
    {
        png_pal = malloc(sizeof(png_color) * pal_indexes);
        for (i = 0; i < pal_indexes; i++)
        {
            png_pal[i].red = RED(palette[i]);
            png_pal[i].green = GREEN(palette[i]);
            png_pal[i].blue = BLUE(palette[i]);
        }
        png_set_PLTE(png_ptr, info_ptr, png_pal, pal_indexes);
    }

    png_write_info(png_ptr, info_ptr);
    if (!pal)
        png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

    /* convert data to palette index format */
    if (pal)
    {
        int pixels = img->user_width * img->user_height;
        guchar* dst;
        int* src;
        
        pal_data = malloc(pixels);
        dst = pal_data;
        src = img->raw_data;

        for (i=0; i < pixels; i++)
            *dst++ = (guint32)(*src++) % (pal_indexes - 1);
    }

    /* initialize row pointers */
    row_p = malloc(sizeof(guchar*) * img->user_height);

    if (pal)
    {
        for (i=0; i < img->user_height; i++)
            row_p[i] = &(pal_data[i * img->user_width]);
    }
    else
    {
        for (i=0; i < img->user_height; i++)
            row_p[i] = (guchar*)&(img->rgb_data[i * img->user_width]);
    }

    /* write image */
    png_write_image(png_ptr, row_p);

    png_write_end(png_ptr, info_ptr);
    if (pal)
    {
        free(png_pal);
        free(pal_data);
    }
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

    file_chooser_add_filter(dialog, "PNG files", "*.png");
    file_chooser_add_filter(dialog, "All files", "*");

    gtk_file_chooser_set_do_overwrite_confirmation (
                        GTK_FILE_CHOOSER (dialog), TRUE);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        filename = gtk_file_chooser_get_filename(
                        GTK_FILE_CHOOSER(dialog));
        save_png_file(img, filename);
        g_free(filename);
    }
    gtk_widget_destroy (dialog);
}
