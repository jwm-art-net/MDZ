#include "image_info.h"

#include "debug.h"
#include "externs.h"
#include "palette.h"
#include "render_threads.h"
#include "setting.h"
#include "fractal.h"

#include <math.h>
#include <string.h>
#include <locale.h>

#define DEFAULT_DEPTH           300
#define DEFAULT_COLOUR_SCALE    1.0


static const char* file_header = "mdz fractal settings";

const char* fractal_str[] = { "mandelbrot", "julia", 0 };

const char* coords_str[] = { "cx", "xmin", 0 };


image_info * image_info_create(fractal_type fr)
{
    double cx =    -0.5;
    double cy =     0.0;
    double size =   4.0;

    image_info * img = malloc(sizeof(image_info));
    memset(img, 0, sizeof(image_info));

    img->fr_type =      fr;
    img->real_width =   img->user_width =    DEFAULT_WIDTH;
    img->real_height =  img->user_height =   DEFAULT_HEIGHT;
    img->aspect =        DEFAULT_ASPECT;
    img->thread_count = 0; /* it gets set by opts sooner or later */
    img->draw_lines = 64;

    if (fr == JULIA)
    {
        img->j_pre = TRUE;
        img->thread_count = 2;
        img->draw_lines = 2;
        cx = 0.0;
    }

    mpfr_init2( img->xmin,  DEFAULT_PRECISION);
    mpfr_init2( img->xmax,  DEFAULT_PRECISION);
    mpfr_init2( img->ymax,  DEFAULT_PRECISION);
    mpfr_init2( img->width, DEFAULT_PRECISION);

    mpfr_init2( img->old_cx,    DEFAULT_PRECISION);
    mpfr_init2( img->old_cy,    DEFAULT_PRECISION);
    mpfr_init2( img->old_size,  DEFAULT_PRECISION);

    mpfr_init2( img->u.julia.c_re, DEFAULT_PRECISION);
    mpfr_init2( img->u.julia.c_im, DEFAULT_PRECISION);

    img->pcoords = coords_new(img->real_width, img->real_height,
                                                   cx, cy, size);
    image_info_reset_view(img);

    return img;
}

void image_info_destroy(image_info* img)
{
    DMSG("freeing image_info data!");
    mpfr_clear( img->xmin );
    mpfr_clear( img->xmax );
    mpfr_clear( img->ymax );
    mpfr_clear( img->width );

    mpfr_clear( img->old_cx );
    mpfr_clear( img->old_cy );
    mpfr_clear( img->old_size );

    mpfr_clear( img->u.julia.c_re );
    mpfr_clear( img->u.julia.c_im );

    g_free(img->rgb_data);
    g_free(img->raw_data);
}

void image_info_set(image_info* img, int w, int h, int aa_factor)
{
    gboolean same_size;

    if ((img->user_width == w)
     && (img->user_height == h)
     && (img->rgb_data != NULL))
        same_size = TRUE;
    else
        same_size = FALSE;

    if (!same_size)
        free(img->rgb_data);

    if (img->raw_data)
        free(img->raw_data);

    img->user_width =   w;
    img->user_height =  h;
    img->aa_factor =    (aa_factor < 1) ? 1 : aa_factor;
    img->aspect =       (double)w/h;

    coords_set(img->pcoords, img->user_width, img->user_height);

    img->real_width =   w * aa_factor;
    img->real_height =  h * aa_factor;

    if (!same_size)
        img->rgb_data =
            malloc(img->user_width * img->user_height * sizeof(guint32));

    img->raw_data =
        malloc(img->real_width * img->real_height * sizeof(int));

    image_info_clear_image(img, TRUE, !same_size);

    image_info_threads_change(img, img->thread_count);
    image_info_set_mpfr(img, img->using_mpfr);
}

void image_info_mpfr_init(image_info * img)
{
    img->using_mpfr = TRUE;
}


void image_info_switch_fractal(image_info* img, int real_px, int imag_py)
{
    if (img->fr_type == JULIA)
    {
        img->fr_type = MANDELBROT;

        /* restore M-Set coordinates from before switch */
        coords_to(img->pcoords, img->old_cx, img->old_cy);
        coords_size(img->pcoords, img->old_size);

        /* restore M-Set initial zoom position */
        img->pcoords->init_cx = -0.5;
    }
    else if (img->fr_type == MANDELBROT)
    {
        img->fr_type = JULIA;

        mpfr_set_d(img->u.julia.c_re,   real_px,   GMP_RNDN);
        mpfr_set_d(img->u.julia.c_im,   imag_py,   GMP_RNDN);

        coords_pixel_to_coord(img->pcoords, img->u.julia.c_re,
                                            img->u.julia.c_im  );

        mpfr_set(img->old_cx,   img->pcoords->cx,   GMP_RNDN);
        mpfr_set(img->old_cy,   img->pcoords->cy,   GMP_RNDN);
        mpfr_set(img->old_size, *img->pcoords->size,GMP_RNDN);

        /* adjust initial zoom position for zoom-resets */
        img->pcoords->init_cx = 0.0;
        coords_reset(img->pcoords);
    }
}


void image_info_threads_change(image_info* img, int thread_count)
{
    img->thread_count = thread_count;

    if (!img->rth_ptr)
    {
        img->rth_ptr = (void*)rth_create();
        if (!img->rth_ptr)
        {
            fprintf(stderr, "failed to create multi-threading data\n");
            return;
        }
    }

    DMSG("calling thread init...");
    if (!rth_init((rthdata*)img->rth_ptr, img->thread_count,
                                            img->draw_lines, img))
    {
        fprintf(stderr, "failed to initialize multi-threading data\n");
    }
}


void image_info_mpfr_change(image_info * img, mp_prec_t precision)
{
    if (img->precision == precision)
        return;

    img->precision = precision;

    if (precision < DEFAULT_PRECISION)
        precision = DEFAULT_PRECISION;

    precision_change(   img->xmin,          precision   );
    precision_change(   img->xmax,          precision   );
    precision_change(   img->ymax,          precision   );
    precision_change(   img->width,         precision   );
    precision_change(   img->old_cx,        precision   );
    precision_change(   img->old_cy,        precision   );
    precision_change(   img->old_size,      precision   );
    precision_change(   img->u.julia.c_re,  precision   );
    precision_change(   img->u.julia.c_re,  precision   );
}

void image_info_clear_image(image_info* img, gboolean raw, gboolean rgb)
{
    int i;

    if (raw) {
        for (i=0; i < img->real_width*img->real_height; i++)
            img->raw_data[i] = UINT_MAX;
    }
    if (rgb) {
        for (i=0; i < img->user_width*img->user_height; i++)
            img->rgb_data[i] = palette[0];
    }
}

void image_info_set_mpfr(image_info* img, gboolean use_mpfr)
{
    rthdata* rth = (rthdata*)img->rth_ptr;

    if (!img->rth_ptr)
        return; /* for when called via image_info_create */

    if (use_mpfr)
    {
        img->using_mpfr = TRUE;
        rth_set_next_line_cb(rth, fractal_mpfr_calculate_line);
    }
    else
    {
        img->using_mpfr = FALSE;
        rth_set_next_line_cb(rth, fractal_calculate_line);
    }
}


void image_info_reset_view(image_info* img)
{
    img->depth =        DEFAULT_DEPTH;

    img->precision =    DEFAULT_PRECISION;
    img->colour_scale = DEFAULT_COLOUR_SCALE;
    img->palette_ip =   FALSE;
    img->zoom_new_win = FALSE;

    image_info_set_mpfr(img, FALSE);

    coords_reset(img->pcoords);
}

int image_info_save_all(image_info * img, const char * filename)
{
    FILE* fd = fopen(filename, "w");
    if (!fd)
    {
        fprintf(stderr, "Failed open file: %s for writing\n", filename);
        fclose(fd);
        return 0;
    }
    if (!image_info_f_save_all(img, fd))
    {
        fprintf(stderr, "Error writing settings file: %s\n", filename);
        fclose(fd);
        return 0;
    }
    fclose(fd);
    return 1;
}


int image_info_f_save_all(image_info * img, FILE* fd)
{
    fprintf(fd, "%s\n", file_header);

    if (!image_info_save_settings(img, fd))
    {
        fprintf(stderr, "Failed to save all settings to file\n");
        fclose(fd);
        return 0;
    }

    fprintf(fd, "palette\n");
    const char* pfn = palette_get_filename();
    if (pfn)
        fprintf(fd, "file %s\n", pfn);
    else
    {
        fprintf(fd, "data\n");
        palette_write(fd);
    }
    return 1;
}


int image_info_save_settings(image_info * img, FILE* fd)
{
    char* loc = setlocale(LC_NUMERIC, 0);

    setlocale(LC_NUMERIC, "C");

    fprintf(fd, "settings\n");
    fprintf(fd, "fractal %s\n", fractal_str[img->fr_type]);
    fprintf(fd, "depth %d\n", img->depth);
    fprintf(fd, "aspect %0.20lf\n", img->aspect);
    fprintf(fd, "colour-scale %0.20lf\n", img->colour_scale);
    fprintf(fd, "colour-interpolate %s\n", (img->palette_ip)
                                            ? "yes" : "no");
    fprintf(fd, "mpfr %s\n", (img->using_mpfr) ? "yes" : "no");
    fprintf(fd, "precision %d\n", (unsigned int)img->precision);

    fprintf(fd, "cx ");
    mpfr_out_str (fd, 10, 0, img->pcoords->cx, GMP_RNDN);
    fprintf(fd, "\ncy ");
    mpfr_out_str (fd, 10, 0, img->pcoords->cy, GMP_RNDN);
    fprintf(fd, "\nsize ");
    mpfr_out_str (fd, 10, 0, *img->pcoords->size, GMP_RNDN);

    fprintf(fd, "\n#xmin ");
    mpfr_out_str (fd, 10, 0, img->xmin, GMP_RNDN);
    fprintf(fd, "\n#xmax ");
    mpfr_out_str (fd, 10, 0, img->xmax, GMP_RNDN);
    fprintf(fd, "\n#ymax ");
    mpfr_out_str (fd, 10, 0, img->ymax, GMP_RNDN);

    if (img->fr_type == JULIA)
    {
        fprintf(fd, "\njulia-real ");
        mpfr_out_str(fd, 10, 0, img->u.julia.c_re, GMP_RNDN);
        fprintf(fd, "\njulia-imag ");
        mpfr_out_str(fd, 10, 0, img->u.julia.c_im, GMP_RNDN);
    }

    fprintf(fd, "\n");

    setlocale(LC_NUMERIC, loc);

    return 1;
}

#ifdef WITH_TMZ_CMDLINE
void image_tmz_save_settings(image_info * img, FILE* fd)
{
    mpfr_t wid, hei, cx, cy;
    double size;
    int  digits, grid;
    char format[20];
    char* loc = setlocale(LC_NUMERIC, 0);

    setlocale(LC_NUMERIC, "C");

    mpfr_init2(wid, img->precision); mpfr_init2(hei, img->precision);
    mpfr_init2(cx, img->precision);  mpfr_init2(cy, img->precision);

    /* Calculate width and height */
    mpfr_sub(wid, img->xmax, img->xmin, GMP_RNDN);
    mpfr_div_d(hei, wid, img->aspect, GMP_RNDN);

    /* cx is average of xmin and xmax */
    mpfr_add(cx, img->xmax, img->xmin, GMP_RNDN);
    mpfr_div_ui(cx, cx, 2, GMP_RNDN);


    /* calculate ymin */
    mpfr_sub(cy, img->ymax, hei, GMP_RNDN);
    /* cy is average of ymin and ymax */
    mpfr_add(cy, cy, img->ymax, GMP_RNDN);
    mpfr_div_ui(cy, cy, 2, GMP_RNDN);

    /* Calculate size */
    if (mpfr_cmp(wid, hei) > 0) {
        /* width is greater */
        size = mpfr_get_d(wid, GMP_RNDN);
        grid = img->real_width;
    } else {
        /* height is greater */
        size = mpfr_get_d(hei, GMP_RNDN);
        grid = img->real_height;
    }
    digits = ((int) log10(4.0 / size)) + 4;

    /* Output */
    sprintf(format, "%%%d.%dRf", digits, digits-2);
    fprintf(fd, "TMZ command line:\n  ./tc ");
    mpfr_fprintf(fd, format, cx);
    fprintf(fd, " ");
    mpfr_fprintf(fd, format, cy);
    fprintf(fd, " %6.4e -nmax %d -grid %d\n", size, img->depth, grid);

    mpfr_clear(wid); mpfr_clear(hei); mpfr_clear(cx); mpfr_clear(cy);

    setlocale(LC_NUMERIC, loc);
}
#endif

int image_info_load_settings(image_info * img, mdzfile* mf)
{
    long precision;
    long depth;
    double aspect;
    int usempfr = 0;
    int w,h;
    int fr_type;
    double colsc;
    int colip = 0;
    int coord_type = 0;
    int ret = 0;

    mpfr_t cx, cy, size, xmin, xmax, ymax, j_re, j_im;

    if ((fr_type = mdzfile_get_index(mf, "fractal", fractal_str)) == -1)
        return mdzfile_err(mf, "error in fractal setting");

    if (!mdzfile_get_long(mf, "depth", &depth, MIN_DEPTH, MAX_DEPTH))
        return mdzfile_err(mf, "Error in depth setting");

    if (!mdzfile_get_double(mf, "aspect", &aspect, 1e-20f, 1e20f))
        return mdzfile_err(mf, "Error in aspect setting");

    if (!mdzfile_get_double(mf, "colour-scale", &colsc, 1e-20f, 1e20f))
        return mdzfile_err(mf, "Errror in colour-scale setting");

    colip = mdzfile_get_index(mf, "colour-interpolate", options_no_yes);
    if (colip == -1)
        return mdzfile_err(mf, "Errror in colour-interpolate setting");

    if ((usempfr = mdzfile_get_index(mf, "mpfr", options_no_yes)) == -1)
        return mdzfile_err(mf, "Errror in mpfr setting");

    if (!mdzfile_get_long(mf, "precision", &precision, 80, 99999999))
        return mdzfile_err(mf, "Errror in precision setting");

    mpfr_init2(cx,      precision);
    mpfr_init2(cy,      precision);
    mpfr_init2(size,    precision);
    mpfr_init2(xmin,    precision);
    mpfr_init2(xmax,    precision);
    mpfr_init2(ymax,    precision);
    mpfr_init2(j_re,    precision);
    mpfr_init2(j_im,    precision);

    if ((coord_type = mdzfile_get_name_index(mf, coords_str)) == -1)
    {
        mdzfile_err(mf, "Error in coordinates setting");
        goto fail;
    }

    if (coord_type == 0)
    {   /* cx, cy, size */
        if (!setting_get_mpfr_t(mf->line, "cx", cx)) {
            mdzfile_err(mf, "Error in cx setting");
            goto fail;
        }
        if (!mdzfile_get_mpfr_t(mf, "cy", cy)) {
            mdzfile_err(mf, "Error in cy setting");
            goto fail;
        }
        if (!mdzfile_get_mpfr_t(mf, "size", size)) {
            mdzfile_err(mf, "Error in size setting");
            goto fail;
        }
    }
    else
    {   /* minx, maxx, maxy */
        if (!setting_get_mpfr_t(mf->line, "xmin", xmin)) {
            mdzfile_err(mf, "Error in xmin setting");
            goto fail;
        }
        if (!mdzfile_get_mpfr_t(mf, "xmax", xmax)) {
            mdzfile_err(mf, "Error in xmax setting");
            goto fail;
        }
        if (!mdzfile_get_mpfr_t(mf, "ymax", ymax)) {
            mdzfile_err(mf, "Error in ymax setting");
            goto fail;
        }
    }

    if (fr_type == JULIA)
    {
        if (!mdzfile_get_mpfr_t(mf, "julia-real", j_re))
        {
            mdzfile_err(mf, "Error in julia-real setting");
            goto fail;
        }

        if (!mdzfile_get_mpfr_t(mf, "julia-imag", j_im))
        {
            mdzfile_err(mf, "Error in julia-imag setting");
            goto fail;
        }
    }

    if (!(w = img->user_width))
        w = DEFAULT_WIDTH;
    h = w / aspect;

    image_info_set(img, w, h, 1);
    img->fr_type =      fr_type;
    img->depth =        depth;
    image_info_set_mpfr(img, usempfr);
    img->colour_scale = colsc;
    img->palette_ip =   colip;

    coords_set_precision(img->pcoords, precision);
    image_info_mpfr_change(img, precision);

    if (coord_type == 0)
    {
        coords_to(img->pcoords, cx, cy);
        coords_size(img->pcoords, size);
    }
    else
    {
        coords_set_rect(img->pcoords, xmin, xmax, ymax);
    }

    if (fr_type == JULIA)
    {
        mpfr_set(img->u.julia.c_re, j_re, GMP_RNDN);
        mpfr_set(img->u.julia.c_im, j_im, GMP_RNDN);
    }

    ret = 1;

fail:
    mpfr_clear(cx);
    mpfr_clear(cy);
    mpfr_clear(size);
    mpfr_clear(xmin);
    mpfr_clear(xmax);
    mpfr_clear(ymax);
    mpfr_clear(j_re);
    mpfr_clear(j_im);

    return ret;
}


int image_info_load_palette(mdzfile* mf)
{
    char* palfile = 0;

    if (!mdzfile_read(mf))
        return mdzfile_err(mf, "palette missing data\n");

    if (strcmp(mf->line, "data") == 0)
        return palette_read(mf->f);

    if ((palfile = setting_get_str(mf->line, "file")))
        return palette_load(palfile);

    return mdzfile_err(mf, "palette data error\n");
}


int image_info_load_all(image_info * img, int sect_flags, const char * fn)
{
    if (!fn)
        return 0;

    mdzfile* mf = mdzfile_read_open(fn);

    if (!mf)
        return 0;

    if (sect_flags & MDZ_FRACTAL_SETTINGS)
    {
        if (mdzfile_read(mf))
        {
            if (!image_info_load_settings(img, mf))
                return 0;
        }
        else
        {
            fprintf(stderr, "Expected settings section\n");
            mdzfile_close(mf);
            return 0;
        }
    }

    if (sect_flags & MDZ_PALETTE_DATA)
    {
        if (mdzfile_skip_to(mf, "palette"))
        {
            if (!image_info_load_palette(mf))
                return 0;
        }
        else
        {
            fprintf(stderr, "Expected palette section\n");
            mdzfile_close(mf);
            return 0;
        }
    }

    mdzfile_close(mf);
    return 1;
}
