#include "fractal.h"
#include "render_threads.h"


#include "frac_mandel.h"
#include "frac_burning_ship.h"
#include "frac_generalized_celtic.h"
#include "frac_variant.h"


const char* family_str[FAMILY_XXX_LAST] =
{
    "mandelbrot",
    "julia",
    0
};


const char* fractal_str[FRACTAL_XXX_LAST] =
{
    "mandelbrot",
    "burning ship",
    "generalized celtic",
    "mandel-celtic hybrid",
    0
};


int fractal_calculate_line(image_info* img, int line)
{
    int ix = 0;
    int mx = 0;
    int chk_px = ((rthdata*)img->rth_ptr)->check_stop_px;

    int img_width = img->real_width;
    int* raw_data = &img->raw_data[line * img_width];

    long double x,y;
    long double x2, y2;
    long double c_re = 0, c_im = 0;

    /* Working */
    long double wre,  wim;
    long double wre2, wim2;
    depth_t depth = img->depth;
    long double bail = img->bail;

    long double xmin, xmax, ymax, width;
    long double j_c_re = 0, j_c_im = 0;

    xmin = mpfr_get_ld(img->xmin,   GMP_RNDN);
    xmax = mpfr_get_ld(img->xmax,   GMP_RNDN);
    ymax = mpfr_get_ld(img->ymax,   GMP_RNDN);
    width = xmax - xmin;

    if (img->family == FAMILY_JULIA)
    {
        j_c_re = mpfr_get_ld(img->u.julia.c_re, GMP_RNDN);
        j_c_im = mpfr_get_ld(img->u.julia.c_im, GMP_RNDN);
    }

    y = ymax - (width / (long double)img_width)
                * (long double)line;
    y2 = y * y;

    while (ix < img_width)
    {
        mx += chk_px;
        if (mx > img_width)
            mx = img_width;
        for (; ix < mx; ++ix, ++raw_data)
        {
            x = (ix / (long double)img_width) * width + xmin;
            x2 = x * x;
            wre = x;
            wim = y;
            wre2 = x2;
            wim2 = y2;
            switch (img->family)
            {
            case FAMILY_MANDEL:
                c_re = x;
                c_im = y;
                break;
            case FAMILY_JULIA:
                c_re = j_c_re;
                c_im = j_c_im;
                break;
            }
            switch(img->fractal)
            {
            case BURNING_SHIP:
                *raw_data = frac_burning_ship(depth, bail, wim, wre,
                                                c_im, c_re,
                                                wim2, wre2);
                break;
            case GENERALIZED_CELTIC:
                *raw_data = frac_generalized_celtic(depth, bail, wim, wre,
                                                c_im, c_re,
                                                wim2, wre2);
                break;
            case VARIANT:
                *raw_data = frac_variant(depth, bail, wim, wre,
                                                c_im, c_re,
                                                wim2, wre2);
                break;
            case MANDELBROT:
            default:
                *raw_data = frac_mandel(depth,  bail, wim, wre,
                                                c_im, c_re,
                                                wim2, wre2);
            }
        }
        if (rth_render_should_stop((rthdata*)img->rth_ptr))
            return 0;
    }
    return 1;
}


int fractal_mpfr_calculate_line(image_info* img, int line)
{
    int ret = 1;
    int ix = 0;
    int mx = 0;
    int chk_px = ((rthdata*)img->rth_ptr)->check_stop_px;
    int img_width = img->real_width;
    int* raw_data = &img->raw_data[line * img_width];

    depth_t depth = img->depth;

    mpfr_t  x,      y;
    mpfr_t  x2,     y2;
    mpfr_t  c_re,   c_im;

/*  working variables:      */
    mpfr_t  wre,    wim;
    mpfr_t  wre2,   wim2;

    mpfr_t  frs_bail;
    mpfr_t  width,  img_rw,    img_xmin;
    mpfr_t  t1;

    mpfr_init2(x,       img->precision);
    mpfr_init2(y,       img->precision);
    mpfr_init2(x2,      img->precision);
    mpfr_init2(y2,      img->precision);
    mpfr_init2(c_re,    img->precision);
    mpfr_init2(c_im,    img->precision);
    mpfr_init2(wre,     img->precision);
    mpfr_init2(wim,     img->precision);
    mpfr_init2(wre2,    img->precision);
    mpfr_init2(wim2,    img->precision);
    mpfr_init2(frs_bail,img->precision);
    mpfr_init2(width,   img->precision);
    mpfr_init2(img_rw,  img->precision);
    mpfr_init2(img_xmin,img->precision);
    mpfr_init2(t1,      img->precision);

    mpfr_set_si(frs_bail,   img->bail,    GMP_RNDN);
    mpfr_set_si(img_rw,     img_width,    GMP_RNDN);
    mpfr_set(   img_xmin,   img->xmin,    GMP_RNDN);
    mpfr_set(   width,      img->width,   GMP_RNDN);

/*  y = img->ymax - ((img->xmax - img->xmin)
                / (long double)img->real_width)
                * (long double)img->lines_done; */
    mpfr_div(       t1,     width,      img_rw,     GMP_RNDN);

    mpfr_mul_si(    t1,     t1,         line,       GMP_RNDN);
    mpfr_sub(       y,      img->ymax,  t1,         GMP_RNDN);
    mpfr_mul(       y2,     y,          y,          GMP_RNDN);

    while (ix < img_width)
    {
        mx += chk_px;
        if (mx > img_width)
            mx = img_width;
        for (; ix < mx; ++ix, ++raw_data)
        {
/*          x = ((long double)ix / (long double)img->real_width)
                * (img->xmax - img->xmin) + img->xmin;              */

            mpfr_si_div(t1,  ix,    img_rw,     GMP_RNDN);

            mpfr_mul(x,      t1,    width,      GMP_RNDN);
            mpfr_add(x,      x,     img_xmin,   GMP_RNDN);

            mpfr_mul(   x2,     x,      x,      GMP_RNDN);
            mpfr_set(   wre,    x,              GMP_RNDN);
            mpfr_set(   wim,    y,              GMP_RNDN);
            mpfr_set(   wre2,   x2,             GMP_RNDN);
            mpfr_set(   wim2,   y2,             GMP_RNDN);

            switch (img->family)
            {
            case FAMILY_MANDEL:
                mpfr_set(c_re,  x,  GMP_RNDN);
                mpfr_set(c_im,  y,  GMP_RNDN);
                break;
            case FAMILY_JULIA:
                mpfr_set(c_re,  img->u.julia.c_re,  GMP_RNDN);
                mpfr_set(c_im,  img->u.julia.c_im,  GMP_RNDN);
                break;
            }
            switch(img->fractal)
            {
            case BURNING_SHIP:
                *raw_data = frac_burning_ship_mpfr(
                                                depth, frs_bail,
                                                    wim, wre,
                                                    c_im, c_re,
                                                    wim2, wre2, t1);
                break;
            case GENERALIZED_CELTIC:
                *raw_data = frac_generalized_celtic_mpfr(
                                                depth, frs_bail,
                                                    wim, wre,
                                                    c_im, c_re,
                                                    wim2, wre2, t1);
                break;
            case VARIANT:
                *raw_data = frac_variant_mpfr(
                                                depth, frs_bail,
                                                    wim, wre,
                                                    c_im, c_re,
                                                    wim2, wre2, t1);
                break;
            case MANDELBROT:
            default:
                *raw_data = frac_mandel_mpfr(depth, frs_bail,
                                                    wim, wre,
                                                    c_im, c_re,
                                                    wim2, wre2, t1);
            }
        }
        if (rth_render_should_stop((rthdata*)img->rth_ptr))
        {
            ret = 0;
            break;
        }
    }
    mpfr_clear(x);
    mpfr_clear(y);
    mpfr_clear(x2);
    mpfr_clear(y2);
    mpfr_clear(c_re);
    mpfr_clear(c_im);
    mpfr_clear(wre);
    mpfr_clear(wim);
    mpfr_clear(wre2);
    mpfr_clear(wim2);
    mpfr_clear(frs_bail);
    mpfr_clear(width);
    mpfr_clear(img_rw);
    mpfr_clear(t1);
    return ret;
}


int fractal_gmp_calculate_line(image_info* img, int line)
{
    int ret = 1;
    int ix = 0;
    int mx = 0;
    int chk_px = ((rthdata*)img->rth_ptr)->check_stop_px;
    int img_width = img->real_width;
    int* raw_data = &img->raw_data[line * img_width];

    depth_t depth = img->depth;

    mpf_t  x,      y;
    mpf_t  x2,     y2;
    mpf_t  c_re,   c_im;

/*  working variables:      */
    mpf_t  wre,    wim;
    mpf_t  wre2,   wim2;

    mpf_t  frs_bail;
    mpf_t  width,  img_rw,    img_xmin;
    mpf_t  t1;

    mpf_init2(x,       img->precision);
    mpf_init2(y,       img->precision);
    mpf_init2(x2,      img->precision);
    mpf_init2(y2,      img->precision);
    mpf_init2(c_re,    img->precision);
    mpf_init2(c_im,    img->precision);
    mpf_init2(wre,     img->precision);
    mpf_init2(wim,     img->precision);
    mpf_init2(wre2,    img->precision);
    mpf_init2(wim2,    img->precision);
    mpf_init2(frs_bail,img->precision);
    mpf_init2(width,   img->precision);
    mpf_init2(img_rw,  img->precision);
    mpf_init2(img_xmin,img->precision);
    mpf_init2(t1,      img->precision);

    mpf_set_si(frs_bail,   img->bail);
    mpf_set_si(img_rw,     img_width);
    mpf_set(   img_xmin,   img->gxmin);
    mpf_set(   width,      img->gwidth);

/*  y = img->ymax - ((img->xmax - img->xmin)
                / (long double)img->real_width)
                * (long double)img->lines_done; */
    mpf_div(       t1,     width,      img_rw);

    mpf_mul_ui(    t1,     t1,         line);
    mpf_sub(       y,      img->gymax, t1);
    mpf_mul(       y2,     y,          y);

    while (ix < img_width)
    {
        mx += chk_px;
        if (mx > img_width)
            mx = img_width;
        for (; ix < mx; ++ix, ++raw_data)
        {
/*          x = ((long double)ix / (long double)img->real_width)
                * (img->xmax - img->xmin) + img->xmin;              */

            mpf_ui_div(t1,  ix,    img_rw);

            mpf_mul(x,      t1,    width);
            mpf_add(x,      x,     img_xmin);

            mpf_mul(   x2,     x,      x);
            mpf_set(   wre,    x);
            mpf_set(   wim,    y);
            mpf_set(   wre2,   x2);
            mpf_set(   wim2,   y2);

            switch (img->family)
            {
            case FAMILY_MANDEL:
                mpf_set(c_re,  x);
                mpf_set(c_im,  y);
                break;
            case FAMILY_JULIA:
                mpfr_to_gmp(img->u.julia.c_re, c_re);
                mpfr_to_gmp(img->u.julia.c_im, c_im);
                break;
            }
            switch(img->fractal)
            {
            case BURNING_SHIP:
                *raw_data = frac_burning_ship_gmp(
                                                depth, frs_bail,
                                                    wim, wre,
                                                    c_im, c_re,
                                                    wim2, wre2, t1);
                break;
            case GENERALIZED_CELTIC:
                *raw_data = frac_generalized_celtic_gmp(
                                                depth,  frs_bail,
                                                    wim, wre,
                                                    c_im, c_re,
                                                    wim2, wre2, t1);
                break;
            case VARIANT:
                *raw_data = frac_variant_gmp(
                                                depth, frs_bail,
                                                    wim, wre,
                                                    c_im, c_re,
                                                    wim2, wre2, t1);
                break;
            case MANDELBROT:
            default:
                *raw_data = frac_mandel_gmp(depth,  frs_bail,
                                                    wim, wre,
                                                    c_im, c_re,
                                                    wim2, wre2, t1);
            }
        }
        if (rth_render_should_stop((rthdata*)img->rth_ptr))
        {
            ret = 0;
            break;
        }
    }
    mpf_clear(x);
    mpf_clear(y);
    mpf_clear(x2);
    mpf_clear(y2);
    mpf_clear(c_re);
    mpf_clear(c_im);
    mpf_clear(wre);
    mpf_clear(wim);
    mpf_clear(wre2);
    mpf_clear(wim2);
    mpf_clear(frs_bail);
    mpf_clear(width);
    mpf_clear(img_rw);
    mpf_clear(t1);
    return ret;
}
