#include "image_info.h"

#include "debug.h"
#include "palette.h"
#include "render_threads.h"
#include "setting.h"
#include "fractal.h"
#include "my_png.h"

#include <math.h>
#include <string.h>
#include <locale.h>

#define DEFAULT_DEPTH           300
#define DEFAULT_COLOUR_SCALE    1.0


static const char* file_header = "mdz fractal settings";
static char* last_used_dir = 0;
static char* last_used_filename = 0;
static char* last_used_filename_path = 0;

const char* coords_str[] =  { "cx", "xmin", 0 };
const char* paloff_str =  "palette-offset";

static void set_last_used(const char* path);

image_info * image_info_create(int family, int fractal)
{
    double cx =    -0.5;
    double cy =     0.0;
    double size =   4.0;

    image_info * img = malloc(sizeof(image_info));
    memset(img, 0, sizeof(image_info));

    img->family =       family;
    img->fractal =      fractal;
    img->real_width =   img->user_width =    DEFAULT_WIDTH;
    img->real_height =  img->user_height =   DEFAULT_HEIGHT;
    img->aspect =        DEFAULT_ASPECT;
    img->thread_count = 0; /* it gets set by opts sooner or later */
    img->draw_lines = 64;

    if (family == FAMILY_JULIA)
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

    mpf_init2(  img->gxmin, DEFAULT_PRECISION);
    mpf_init2(  img->gxmax, DEFAULT_PRECISION);
    mpf_init2(  img->gymax, DEFAULT_PRECISION);
    mpf_init2(  img->gwidth, DEFAULT_PRECISION);

    img->pcoords = coords_new(img->real_width, img->real_height,
                                                   cx, cy, size);
    image_info_reset_view(img);

    return img;
}

void image_info_destroy(image_info* img)
{
    DMSG("freeing image_info data!\n");
    mpfr_clear( img->xmin );
    mpfr_clear( img->xmax );
    mpfr_clear( img->ymax );
    mpfr_clear( img->width );

    mpf_clear(  img->gxmin );
    mpf_clear(  img->gxmax );
    mpf_clear(  img->gymax );
    mpf_clear(  img->gwidth );

    mpfr_clear( img->old_cx );
    mpfr_clear( img->old_cy );
    mpfr_clear( img->old_size );

    mpfr_clear( img->u.julia.c_re );
    mpfr_clear( img->u.julia.c_im );

    g_free(img->rgb_data);
    g_free(img->raw_data);

    coords_free(img->pcoords);
}

void image_info_set(image_info* img, int w, int h, int aa_factor)
{
    DMSG("image_info_set\n");
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

    image_info_threads_change(img,  img->thread_count);
    image_info_set_multi_prec(img,  img->use_multi_prec,
                                    img->use_rounding   );
}


void image_info_switch_fractal(image_info* img, int real_px, int imag_py)
{
    DMSG("image_info_switch_fractal\n");

    if (img->family == FAMILY_JULIA)
    {
        img->family = FAMILY_MANDEL;

        /* restore M-Set coordinates from before switch */
        coords_to(img->pcoords, img->old_cx, img->old_cy);
        coords_size(img->pcoords, img->old_size);

        /* restore M-Set initial zoom position */
        img->pcoords->init_cx = -0.5;
    }
    else if (img->family == FAMILY_MANDEL)
    {
        img->family = FAMILY_JULIA;

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
    DMSG("image_info_threads_change\n");

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

    DMSG("calling thread init...\n");
    if (!rth_init((rthdata*)img->rth_ptr, img->thread_count,
                                          img->draw_lines,   img))
    {
        fprintf(stderr, "failed to initialize multi-threading data\n");
    }
}


void image_info_clear_image(image_info* img, bool raw, bool rgb)
{
    int i;

    if (raw)
    {
        for (i=0; i < img->real_width*img->real_height; i++)
            img->raw_data[i] = UINT_MAX;
    }

    if (rgb)
    {
        for (i=0; i < img->user_width*img->user_height; i++)
            img->rgb_data[i] = palette[0];
    }
}


void image_info_set_multi_prec(image_info* img, bool use_multi_prec,
                                                bool use_rounding)
{
    DMSG("image_info_set_multi_prec\n");

    rthdata* rth = (rthdata*)img->rth_ptr;

    if (!img->rth_ptr)
        return; /* to prevent seg fault happening */

    img->use_multi_prec = use_multi_prec;

    if (use_multi_prec)
    {
        img->use_rounding = use_rounding;

        if (use_rounding)
            rth_set_next_line_cb(rth, fractal_mpfr_calculate_line);
        else
            rth_set_next_line_cb(rth, fractal_gmp_calculate_line);
    }
    else
        rth_set_next_line_cb(rth, fractal_calculate_line);
}


void image_info_set_precision(image_info * img, mpfr_prec_t precision)
{
    DMSG("image_info_set_precision\n");

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

    precision_change_gmp(   img->gxmin, precision );
    precision_change_gmp(   img->gxmax, precision );
    precision_change_gmp(   img->gymax, precision );
    precision_change_gmp(   img->gwidth, precision );

    coords_set_precision(   img->pcoords,   precision   );
}



void image_info_reset_view(image_info* img)
{
    img->depth =        DEFAULT_DEPTH;

    img->precision =    DEFAULT_PRECISION;
    img->colour_scale = DEFAULT_COLOUR_SCALE;
    img->palette_ip =   FALSE;
    img->zoom_new_win = FALSE;

    image_info_set_multi_prec(img, FALSE, FALSE);

    coords_reset(img->pcoords);

    img->ui_ref_center = TRUE;
    image_info_reset_last_used_filename();
    my_png_reset_last_used_filename();
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
    set_last_used(filename);
    return 1;
}


int image_info_f_save_all(image_info * img, FILE* fd)
{
    fprintf(fd, "%s %s\n", file_header, VERSION);

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
    const char* center = (img->ui_ref_center) ? "" : "#";
    const char* corner = (img->ui_ref_center) ? "#" : "";

    fprintf(fd, "# http://jwm-art.net/mdz/\n");

    setlocale(LC_NUMERIC, "C");

    fprintf(fd, "settings\n");
    fprintf(fd, "family %s\n",              family_str[img->family]);
    fprintf(fd, "fractal %s\n",             fractal_str[img->fractal]);
    fprintf(fd, "depth %ld\n",              img->depth);
    fprintf(fd, "aspect %0.20lf\n",         img->aspect);
    fprintf(fd, "colour-scale %0.20lf\n",   img->colour_scale);

    fprintf(fd, "colour-interpolate %s\n",  (img->palette_ip)
                                            ? "yes"
                                            : "no");

    fprintf(fd, "multi-precision %s\n",     (img->use_multi_prec)
                                            ? "yes"
                                            : "no");

    fprintf(fd, "multi-rounding %s\n",      (img->use_rounding)
                                            ? "yes"
                                            : "no");

    fprintf(fd, "precision %d\n",           (unsigned int)img->precision);

    fprintf(fd, "%scx ", center);
    mpfr_out_str (fd, 10, 0,    img->pcoords->cx, GMP_RNDN);
    fprintf(fd, "\n%scy ",      center);
    mpfr_out_str (fd, 10, 0,    img->pcoords->cy, GMP_RNDN);
    fprintf(fd, "\n%ssize ",    center);
    mpfr_out_str (fd, 10, 0,    *img->pcoords->size, GMP_RNDN);

    fprintf(fd, "\n%sxmin ",    corner);
    mpfr_out_str (fd, 10, 0,    img->xmin, GMP_RNDN);
    fprintf(fd, "\n%sxmax ",    corner);
    mpfr_out_str (fd, 10, 0,    img->xmax, GMP_RNDN);
    fprintf(fd, "\n%symax ",    corner);
    mpfr_out_str (fd, 10, 0,    img->ymax, GMP_RNDN);

    if (img->family && FAMILY_JULIA)
    {
        fprintf(fd, "\njulia-real ");
        mpfr_out_str(fd, 10, 0, img->u.julia.c_re, GMP_RNDN);
        fprintf(fd, "\njulia-imag ");
        mpfr_out_str(fd, 10, 0, img->u.julia.c_im, GMP_RNDN);
    }



    fprintf(fd, "\n");

    fprintf(fd, "palette-offset %d\n", pal_offset);

    fprintf(fd, "r-strength %lf\n", img->rnd_pal->r_strength);
    fprintf(fd, "r-bands %lf\n",    img->rnd_pal->r_bands);
    fprintf(fd, "g-strength %lf\n", img->rnd_pal->g_strength);
    fprintf(fd, "g-bands %lf\n",    img->rnd_pal->g_bands);
    fprintf(fd, "b-strength %lf\n", img->rnd_pal->b_strength);
    fprintf(fd, "b-bands %lf\n",    img->rnd_pal->b_bands);
    fprintf(fd, "rnd-offset %d\n",  img->rnd_pal->offset);
    fprintf(fd, "rnd-stripe %d\n",  img->rnd_pal->stripe);
    fprintf(fd, "rnd-spread %d\n",  img->rnd_pal->spread);

    setlocale(LC_NUMERIC, loc);

    return 1;
}

#ifdef WITH_TMZ_CMDLINE
void image_tmz_save_settings(image_info * img, FILE* fd)
{
    double size;
    int  digits, grid;
    char format[20];
    char* loc = setlocale(LC_NUMERIC, 0);

    setlocale(LC_NUMERIC, "C");

    size = mpfr_get_d(*img->pcoords->size, GMP_RNDN);

    /* Calculate size */
    if (mpfr_cmp(img->pcoords->width, img->pcoords->height) > 0) {
        /* width is greater */
        grid = img->real_width;
    } else {
        /* height is greater */
        grid = img->real_height;
    }

    digits = ((int) log10(4.0 / size)) + 4;

    /* Output */
    sprintf(format, "%%%d.%dRf", digits, digits-2);
    fprintf(fd, "TMZ command line:\n  ./tc ");
    mpfr_fprintf(fd, format, img->pcoords->cx);
    fprintf(fd, " ");
    mpfr_fprintf(fd, format, img->pcoords->cy);
    fprintf(fd, " %6.4e -nmax %d -grid %d\n", size, img->depth, grid);

    setlocale(LC_NUMERIC, loc);
}
#endif

int image_info_load_settings(image_info * img, mdzfile* mf)
{
    long precision;
    long depth;
    double aspect;
    int use_multi_prec;
    int use_rounding;
    int w,h;
    int family, fractal;
    double colsc;
    int colip = 0;
    int coord_type = 0;
    int ret = 0;
    long paloff;

    double rnd_rs, rnd_rb, rnd_gs, rnd_gb, rnd_bs, rnd_bb;
    long rnd_offset, rnd_stripe, rnd_spread;
    int rndpal = 0;


    DMSG("image_info_load_settings\n");

    mpfr_t cx, cy, size, xmin, xmax, ymax, j_re, j_im;

    if(mf->version_maj == 0 && mf->version_min == 0)
    {
        family = mdzfile_get_index(mf, "fractal", family_str);

        if (family == -1)
            return mdzfile_err(mf, "error in fractal setting");

        fractal = MANDELBROT;
    }
    else /***** MDZ versions 0.1.0 and above *****/
    {
        family = mdzfile_get_index(mf, "family", family_str);
        if (family == -1)
            return mdzfile_err(mf, "error in family setting");

        fractal = mdzfile_get_index(mf, "fractal", fractal_str);
        if (fractal == -1)
            return mdzfile_err(mf, "error in fractal setting");
    }

    if (!mdzfile_get_long(mf, "depth", &depth, MIN_DEPTH, MAX_DEPTH))
        return mdzfile_err(mf, "Error in depth setting");

    if (!mdzfile_get_double(mf, "aspect", &aspect, 1e-20f, 1e20f))
        return mdzfile_err(mf, "Error in aspect setting");

    if (!mdzfile_get_double(mf, "colour-scale", &colsc, 1e-20f, 1e20f))
        return mdzfile_err(mf, "Errror in colour-scale setting");

    colip = mdzfile_get_index(mf, "colour-interpolate", options_no_yes);
    if (colip == -1)
        return mdzfile_err(mf, "Errror in colour-interpolate setting");

    if(mf->version_maj == 0 && mf->version_min == 0)
    {
        use_multi_prec = mdzfile_get_index(mf, "mpfr", options_no_yes);

        if (use_multi_prec == -1)
            return mdzfile_err(mf, "Errror in mpfr setting");

        use_rounding = 1; /* old versions always used MPFR */
    }
    else /***** MDZ versions 0.1.0 and above *****/
    {
        use_multi_prec = mdzfile_get_index(mf, "multi-precision",
                                                    options_no_yes);
        if (use_multi_prec == -1)
            return mdzfile_err(mf, "Errror in multi-precision setting");

        use_rounding = mdzfile_get_index(mf, "multi-rounding",
                                                    options_no_yes);
        if (use_rounding == -1)
            return mdzfile_err(mf, "Errror in multi-rounding setting");
    }

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

    img->ui_ref_center = (coord_type == 0) ? true : false;

    if (img->ui_ref_center)
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

    if (family == FAMILY_JULIA)
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

    if ((mdzfile_test_for_name(mf, paloff_str)))
    {
        /*if (!setting_get_long(mf->line, paloff_str, &paloff, 0, 255)) */
        if (!mdzfile_get_long(mf, paloff_str, &paloff, 0, 255))
        {
            mdzfile_err(mf, "Error in palette offset setting");
            goto fail;
        }

        pal_offset = (int)paloff;
    }
    else
        pal_offset = 0;

    if ((mdzfile_test_for_name(mf, "r-strength")))
    {
        if (!mdzfile_get_double(mf, "r-strength", &rnd_rs, 0.0, 1.0))
        {
            mdzfile_err(mf, "Error in r-strength setting");
            goto fail;
        }
        if (!mdzfile_get_double(mf, "r-bands", &rnd_rb, 0.0, 1.0))
        {
            mdzfile_err(mf, "Error in r-bands setting");
            goto fail;
        }
        if (!mdzfile_get_double(mf, "g-strength", &rnd_gs, 0.0, 1.0))
        {
            mdzfile_err(mf, "Error in g-strength setting");
            goto fail;
        }
        if (!mdzfile_get_double(mf, "g-bands", &rnd_gb, 0.0, 1.0))
        {
            mdzfile_err(mf, "Error in g-bands setting");
            goto fail;
        }
        if (!mdzfile_get_double(mf, "b-strength", &rnd_bs, 0.0, 1.0))
        {
            mdzfile_err(mf, "Error in b-strength setting");
            goto fail;
        }
        if (!mdzfile_get_double(mf, "b-bands", &rnd_bb, 0.0, 1.0))
        {
            mdzfile_err(mf, "Error in b-bands setting");
            goto fail;
        }
        if (!mdzfile_get_long(mf, "rnd-offset", &rnd_offset, 0, 255))
        {
            mdzfile_err(mf, "Error in rnd-offset setting");
            goto fail;
        }
        if (!mdzfile_get_long(mf, "rnd-stripe", &rnd_stripe, 0, 255))
        {
            mdzfile_err(mf, "Error in rnd-stripe setting");
            goto fail;
        }
        if (!mdzfile_get_long(mf, "rnd-spread", &rnd_spread, 0, 255))
        {
            mdzfile_err(mf, "Error in rnd-spread setting");
            goto fail;
        }
        rndpal = 1;
    }

    if (!(w = img->user_width))
        w = DEFAULT_WIDTH;
    h = w / aspect;

    image_info_set(img, w, h, img->aa_factor);
    img->family =       family;
    img->fractal =      fractal;
    img->depth =        depth;
    image_info_set_multi_prec(img, use_multi_prec, use_rounding);
    img->colour_scale = colsc;
    img->palette_ip =   colip;
    coords_set_precision(img->pcoords, precision);
    image_info_set_precision(img, precision);

    if (rndpal) {
        img->rnd_pal->r_strength = rnd_rs;
        img->rnd_pal->g_strength = rnd_gs;
        img->rnd_pal->b_strength = rnd_bs;
        img->rnd_pal->r_bands = rnd_rb;
        img->rnd_pal->g_bands = rnd_gb;
        img->rnd_pal->b_bands = rnd_bb;
        img->rnd_pal->offset = rnd_offset;
        img->rnd_pal->stripe = rnd_stripe;
        img->rnd_pal->spread = rnd_spread;
    }

    if (img->ui_ref_center)
    {
        coords_to(img->pcoords, cx, cy);
        coords_size(img->pcoords, size);
    }
    else
    {
        coords_set_rect(img->pcoords, xmin, xmax, ymax);
        coords_calculate_precision(img->pcoords);
    }

    if (family == FAMILY_JULIA)
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
    DMSG("image_info_load_palette\n");
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
    DMSG("image_info_load_all\n");
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
            fprintf(stderr, "Expected settings section\n"
                            "Got '%s' instead.\n",
                            mf->line );
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
            fprintf(stderr, "Expected palette section\n"
                            "Got '%s' instead.\n",
                            mf->line );
            mdzfile_close(mf);
            return 0;
        }
    }

    set_last_used(fn);

    mdzfile_close(mf);
    return 1;
}


static void set_last_used(const char* path)
{
    if (!path)
        return;

    if (last_used_dir && strcmp(last_used_dir, path) != 0) {
        free(last_used_dir);
        last_used_dir = 0;
    }
    if (!last_used_dir) {
        last_used_dir = strdup(path);
        last_used_dir = dirname(last_used_dir);
    }
    if (last_used_filename && strcmp(last_used_filename, path) != 0) {
        free(last_used_filename);
        last_used_filename = 0;
    }
    if (!last_used_filename) {
        last_used_filename_path = strdup(path);
        last_used_filename = basename(last_used_filename_path);
    }
}

const char* image_info_get_last_used_dir(void)
{
    return last_used_dir;
}

const char* image_info_get_last_used_filename(void)
{
    return last_used_filename;
}

void image_info_reset_last_used_filename(void)
{
    if (last_used_filename_path) {
        free(last_used_filename_path);
        last_used_filename_path = 0;
        last_used_filename = 0;
    }
}

void image_info_cleanup(void)
{
    if (last_used_dir) {
        free(last_used_dir);
        last_used_dir = 0;
    }

    image_info_reset_last_used_filename();
}
