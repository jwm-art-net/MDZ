#include "render.h"

#include "cmdline.h"
#include "fractal.h"
#include "my_png.h"
#include "palette.h"
#include "render_threads.h"


int render_to_file(image_info* img)
{
    FILE* fd_test;
    if (!(fd_test = fopen(opts.renderfile, "w")))
    {
        fprintf(stderr, "test to open %s for writing failed\n",
                        opts.renderfile);
        return 0;
    }
    fclose(fd_test);

    if (opts.logfd)
        image_info_save_settings(img, opts.logfd);

    rthdata* rth = (rthdata*)img->rth_ptr;


    int aa = img->aa_factor;
    int y = 0;
    int miny;
    int maxy;
    int undrawn;
    int width = img->user_width;

    coords_get_rect(img->pcoords,   img->xmin, img->xmax,
                                    img->ymax, img->width);
    #ifdef WITH_GMP
    coords_get_rect_gmp(img->pcoords,   img->gxmin, img->gxmax,
                                        img->gymax, img->gwidth);
    #endif

    rth_ui_init(rth);

    rth_ui_start_render(rth);

    printf("calculating...\n");

    int linesdone = 0;
    char* ld = 0;
    double rendertime = 0;

    do
    {
        rth_ui_wait_for_line_done(rth);
        linesdone = rth_process_lines_rendered(rth);
        if (linesdone)
        {
            undrawn = 0;
            miny = rth->min_line_drawn;
            maxy = miny + rth->line_draw_count + 1;

            if (maxy >= img->user_height)
                maxy = img->user_height;

            if (linesdone > 0 && maxy > linesdone)
                maxy = linesdone;

            ld = &rth->lines_drawn[miny];

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
                }
                else if (*ld == 0)
                    undrawn = 1;
            }
            rendertime = rth_ui_get_render_time(rth);
            printf("%4d of %4d lines done", linesdone > 0
                                                ? linesdone
                                                : img->user_height,
                                            img->user_height);
            printf(" [time taken: %.3f]", rendertime);
            printf("\033[A \n");
        }
        
    } while(y < img->user_height);

    printf("\n");

    save_png_file(img, opts.renderfile);

    if (opts.logfd)
    {
        fprintf(opts.logfd, "render-time %.3fs\n", rendertime);
        fprintf(opts.logfd, "saved-image %s\n", opts.renderfile);
    }

    return 1;
}


void do_anti_aliasing(image_info* img,
                      int x0, int y0, int width, int height)
{

    int x,y;

    int         aa =    img->aa_factor;
    gboolean    ip =    img->palette_ip;
    double      sc =    img->colour_scale;
    int         raw_width = img->real_width;
    int         yoff;
    int         xoff;
    int aaaa = aa * aa;
    int xi;
    int yi;

    guint32 r, g, b;
    guint32 c;
    guint32* rgb;
    int* raw;

    for (y = y0; y < y0 + height; ++y)
    {
        yoff = y * aa;
        rgb = &img->rgb_data[y * img->user_width + x0];

        for (x = x0; x < x0 + width; ++x, ++rgb)
        {
            xoff = x * aa;
            r = g = b = 0;

            for (yi = yoff; yi < yoff + aa; ++yi)
            {
                raw = &img->raw_data[yi * raw_width + xoff];

                for (xi = xoff; xi < xoff + aa; ++xi, ++raw)
                {
                    c = get_pixel_colour(*raw * sc, ip);
                    r += RED(c);
                    g += GREEN(c);
                    b += BLUE(c);
                }
            }

            r /= aaaa;
            g /= aaaa;
            b /= aaaa;
            *rgb = RGB(r, g, b);
        }
    }
}

