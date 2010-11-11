#include "fractal.h"
#include "render_threads.h"

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
    depth_t wz;
    long double wre,  wim;
    long double wre2, wim2;
    depth_t depth = img->depth;

    long double xmin, xmax, ymax, width;
    long double j_c_re = 0, j_c_im = 0;

    xmin = mpfr_get_ld(img->xmin,   GMP_RNDN);
    xmax = mpfr_get_ld(img->xmax,   GMP_RNDN);
    ymax = mpfr_get_ld(img->ymax,   GMP_RNDN);
    width = xmax - xmin;

    if (img->fr_type == JULIA)
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
            switch (img->fr_type)
            {
            case MANDELBROT:
                c_re = x;
                c_im = y;
                break;
            case JULIA:
                c_re = j_c_re;
                c_im = j_c_im;
                break;
            }
            for (wz = 1; wz <= depth; wz++)
            {
                wim = 2.0 * wre * wim+ c_im;
                wre = wre2 - wim2 + c_re;
                wim2 = wim * wim;
                wre2 = wre * wre;
                if (wim2 + wre2 > 4.0F)
                    break;
            }
            if (wz == depth + 1)
                wz = 0;
            *raw_data = wz;
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

    depth_t wz;
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

    mpfr_set_si(frs_bail,   4,          GMP_RNDN);
    mpfr_set_si(img_rw,     img_width,  GMP_RNDN);
    mpfr_set(   img_xmin,   img->xmin,  GMP_RNDN);
    mpfr_set(   width,      img->pcoords->width,    GMP_RNDN);

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

            switch (img->fr_type)
            {
            case MANDELBROT:
                mpfr_set(c_re,  x,  GMP_RNDN);
                mpfr_set(c_im,  y,  GMP_RNDN);
                break;
            case JULIA:
                mpfr_set(c_re,  img->u.julia.c_re,  GMP_RNDN);
                mpfr_set(c_im,  img->u.julia.c_im,  GMP_RNDN);
                break;
            }

            for (wz = 1; wz <= depth; wz++)
            {
                /* wim = 2.0 * wre * wim + c_im; */
                mpfr_mul(   t1,     wre,    wim,    GMP_RNDN);
                mpfr_mul_si(t1,     t1,     2,      GMP_RNDN);
                mpfr_add(   wim,    t1,     c_im,   GMP_RNDN);
                /* wre = wre2 - wim2 + c_re; */
                mpfr_sub(   t1,     wre2,   wim2,   GMP_RNDN);
                mpfr_add(   wre,    t1,     c_re,   GMP_RNDN);
                /* wim2 = wim * wim; */
                mpfr_mul(   wim2,   wim,    wim,    GMP_RNDN);
                /* wre2 = wre * wre; */
                mpfr_mul(   wre2,   wre,    wre,    GMP_RNDN);
                /* if ((wim2 + wre2) > frs_bail) */
                mpfr_add(   t1,     wim2,   wre2,   GMP_RNDN);
                if (mpfr_greater_p(t1, frs_bail))
                    break;
            }
            if (wz == depth + 1)
                wz = 0;
            *raw_data = wz;
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
