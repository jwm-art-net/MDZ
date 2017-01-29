#include "coords.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


#include "cmdline.h"
#include "debug.h"
#include "my_mpfr_to_str.h"


void mpfr_to_gmp(mpfr_t from, mpf_t to)
{
    char* str = my_mpfr_to_str(from);
    mpf_set_str (to, str, 10);
    free(str);
}


static void coords_mpfr_clear(coords* c);


#if MPFR_VERSION_MAJOR < 2 || (MPFR_VERSION_MAJOR == 2 && MPFR_VERSION_MINOR < 4)
int mpfr_div_d(mpfr_t rop, mpfr_t op1, double _op2, mpfr_rnd_t rnd)
{
    int r;
    mpfr_t  op2;
    mpfr_init2(     op2,    mpfr_get_prec(op1));
    mpfr_set_d(     op2,    _op2,               rnd);
    r = mpfr_div(   rop,    op1,    op2,        rnd);
    mpfr_clear(op2);
    return r;
}

int mpfr_mul_d(mpfr_t rop, mpfr_t op1, double _op2, mpfr_rnd_t rnd)
{
    int r;
    mpfr_t  op2;
    mpfr_init2(     op2,    mpfr_get_prec(op1));
    mpfr_set_d(     op2,    _op2,               rnd);
    r = mpfr_mul(   rop,    op1,    op2,        rnd);
    mpfr_clear(op2);
    return r;
}
#endif


coords* coords_new( int img_width, int img_height,
                    double init_cx, double init_cy, double init_size)
{
    coords* c = malloc(sizeof(*c));

    if (!c)
        return 0;

    c->precision =  0;

    c->precision = DEFAULT_PRECISION;

    mpfr_init2(c->xmin,     c->precision);
    mpfr_init2(c->xmax,     c->precision);
    mpfr_init2(c->ymin,     c->precision);
    mpfr_init2(c->ymax,     c->precision);
    mpfr_init2(c->width,    c->precision);
    mpfr_init2(c->height,   c->precision);
    mpfr_init2(c->cx,       c->precision);
    mpfr_init2(c->cy,       c->precision);
    mpfr_init2(c->_size,    c->precision);

    coords_set(c, img_width, img_height);

    c->init_cx =    init_cx;
    c->init_cy =    init_cy;
    c->init_size =  init_size;

    mpfr_set_d( c->_size,   init_size,  GMP_RNDN);

    return c;
}


coords* coords_dup(const coords* c)
{
    coords* d = coords_new( c->img_width, c->img_height,
                            c->init_cx, c->init_cy, c->init_size);

    if (!d)
        return 0;

    coords_cpy(d, c);

    return d;
}


coords* coords_cpy(coords* dest, const coords* src)
{
    coords_set_precision(dest, src->precision);

    dest->img_width =   src->img_width;
    dest->img_height =  src->img_height;
    dest->aspect =      src->aspect;
    dest->init_cx =     src->init_cx;
    dest->init_cy =     src->init_cy;
    dest->init_size =   src->init_size;
    dest->recommend =   src->recommend;

    mpfr_set(dest->xmin,    src->xmin,      GMP_RNDN);
    mpfr_set(dest->xmax,    src->xmax,      GMP_RNDN);
    mpfr_set(dest->ymin,    src->ymin,      GMP_RNDN);
    mpfr_set(dest->ymax,    src->ymax,      GMP_RNDN);
    mpfr_set(dest->cx,      src->cx,        GMP_RNDN);
    mpfr_set(dest->cy,      src->cy,        GMP_RNDN);
    mpfr_set(dest->width,   src->width,     GMP_RNDN);
    mpfr_set(dest->height,  src->height,    GMP_RNDN);
    mpfr_set(dest->_size,   src->_size,     GMP_RNDN);

    dest->size = (dest->aspect > 1.0) ? &dest->width : &dest->height;

    return dest;
}


void coords_dump(const coords* c, const char* msg)
{
#if MPFR_VERSION_MAJOR > 2 || (MPFR_VERSION_MAJOR == 2 && MPFR_VERSION_MINOR >= 4)
    mpfr_printf("%s\nc->cx:%.Rf\tc->cy:%.Rf\tc->size:%.Rf\n",
                msg, c->cx,  c->cy, c->width);

    mpfr_printf("c->xmin:%.Rf\tc->xmax:%.Rf\tc->ymax:%.Rf\n",
                c->xmin,  c->xmax, c->ymax);

    printf("c->precision: %ld\n", (long)c->precision);
#endif
}


void coords_free(coords* c)
{
    if (!c)
        return;

    if (c->precision)
        coords_mpfr_clear(c);

    free(c);
}


int coords_calculate_precision(coords* c)
{
    int l, p;
    mpfr_t tmp;
    mpfr_t bail;
    mpfr_t px_size;
    mpfr_t precision;

    DMSG("Calculating recommended precision...\n");

    #ifndef NDEBUG
    coords_dump(c,"meh:");
    #endif

    mpfr_init2(tmp,         c->precision);
    mpfr_init2(bail,        c->precision);
    mpfr_init2(px_size,     c->precision);
    mpfr_init2(precision,   c->precision);

    mpfr_set_d( bail,       4.0,        GMP_RNDN);
    mpfr_div_si(px_size,    c->width,   c->img_width,   GMP_RNDN);
    mpfr_div(   tmp,        bail,       px_size,        GMP_RNDN);

    l = mpfr_log2(  precision,          tmp,            GMP_RNDN);
    p = (int)mpfr_get_si(   precision,                  GMP_RNDN);

    if (l < 0) /* precision was rounded down */
        ++p;

    c->recommend = p;

    mpfr_clear(tmp);
    mpfr_clear(bail);
    mpfr_clear(px_size);
    mpfr_clear(precision);

    mpfr_free_cache(); /* <-- keep valgrind happy over mpfr_log2 */
    DMSG("Recommended precision: %ld\n", c->recommend);
    return c->recommend;
}


void coords_reset(coords* c)
{
    coords_set_precision(c, DEFAULT_PRECISION);
    mpfr_set_d( c->cx,      c->init_cx,     GMP_RNDN);
    mpfr_set_d( c->cy,      c->init_cy,     GMP_RNDN);
    mpfr_set_d( c->_size,   c->init_size,   GMP_RNDN);
    mpfr_set(   *c->size,   c->_size,       GMP_RNDN);
    coords_calculate_precision(c);
}


void coords_set_precision(coords* c, mpfr_prec_t precision)
{
    mpf_t test;

    if (precision < DEFAULT_PRECISION)
        precision = DEFAULT_PRECISION;

    mpf_init2(test, precision);
    c->gmp_precision = mpf_get_prec(test);

    if ((unsigned long)c->gmp_precision > (unsigned long)precision)
        precision = c->gmp_precision;

    precision_change(c->xmin,   precision);
    precision_change(c->xmax,   precision);
    precision_change(c->ymin,   precision);
    precision_change(c->ymax,   precision);
    precision_change(c->cx,     precision);
    precision_change(c->cy,     precision);
    precision_change(c->width,  precision);
    precision_change(c->height, precision);
    precision_change(c->_size,  precision);

    c->precision = precision;
    c->size = (c->aspect > 1.0) ? &c->width : &c->height;

    mpf_clear(test);
}


void coords_rect_to_center(coords* c)
{
    DMSG("updating from rect to center\n");

    /* calc width, height */
    mpfr_sub(   c->width,   c->xmax,    c->xmin,        GMP_RNDN);
    mpfr_div_d( c->height,  c->width,   c->aspect,      GMP_RNDN);

    /* calc ymin */
    mpfr_sub(   c->ymin,    c->ymax,    c->height,      GMP_RNDN);

    /* calc center x */
    mpfr_add(   c->cx,      c->xmin,    c->xmax,        GMP_RNDN);
    mpfr_div_ui(c->cx,      c->cx,      2,              GMP_RNDN);

    /* calc center y */
    mpfr_add(   c->cy,      c->ymin,    c->ymax,        GMP_RNDN);
    mpfr_div_ui(c->cy,      c->cy,      2,              GMP_RNDN);
}


void coords_set(coords* c, int img_width, int img_height)
{
    c->img_width =  img_width;
    c->img_height = img_height;
    c->aspect = (double)img_width / img_height;
    c->size = (c->aspect > 1.0) ? &c->width : &c->height;
    mpfr_set(*c->size,  c->_size,  GMP_RNDN);
}


void coords_center_to_rect(coords* c)
{
    mpfr_t tmp;
    mpfr_init2(tmp, c->precision);

    DMSG("updating from center to rect\n");

    if (c->aspect > 1.0)
    {
        DMSG("wide image\n");

        mpfr_div_d( c->height,  c->width,   c->aspect,  GMP_RNDN);

        mpfr_div_ui(tmp,        c->width,   2,          GMP_RNDN);
        mpfr_sub(   c->xmin,    c->cx,      tmp,        GMP_RNDN);
        mpfr_add(   c->xmax,    c->xmin,    c->width,   GMP_RNDN);

        mpfr_div_d( tmp,        tmp,        c->aspect,  GMP_RNDN);
        mpfr_sub(   c->ymin,    c->cy,      tmp,        GMP_RNDN);
        mpfr_add(   c->ymax,    c->ymin,    c->height,  GMP_RNDN);
    }
    else
    {
        DMSG("tall image\n");

        mpfr_mul_d( c->width,   c->height,  c->aspect,  GMP_RNDN);

        mpfr_div_ui(tmp,        c->height,  2,          GMP_RNDN);
        mpfr_sub(   c->ymin,    c->cy,      tmp,        GMP_RNDN);
        mpfr_add(   c->ymax,    c->ymin,    c->height,  GMP_RNDN);

        mpfr_mul_d( tmp,        tmp,        c->aspect,  GMP_RNDN);
        mpfr_sub(   c->xmin,    c->cx,      tmp,        GMP_RNDN);
        mpfr_add(   c->xmax,    c->xmin,    c->width,   GMP_RNDN);
    }

    mpfr_clear(tmp);
}


void coords_get_rect(coords* c, mpfr_t xmin, mpfr_t xmax,
                                mpfr_t ymax, mpfr_t width)
{
    coords_center_to_rect(c);
    mpfr_set(xmin,  c->xmin, GMP_RNDN);
    mpfr_set(xmax,  c->xmax, GMP_RNDN);
    mpfr_set(ymax,  c->ymax, GMP_RNDN);
    mpfr_set(width, c->width,GMP_RNDN);
}


void coords_get_rect_gmp(coords* c, mpf_t xmin, mpf_t xmax,
                                    mpf_t ymax, mpf_t width)
{
    coords_center_to_rect(c);
    mpfr_to_gmp(c->xmin, xmin);
    mpfr_to_gmp(c->xmax, xmax);
    mpfr_to_gmp(c->ymax, ymax);
    mpfr_to_gmp(c->width, width);
}


void coords_set_rect(coords* c, mpfr_t xmin, mpfr_t xmax, mpfr_t ymax)
{
    mpfr_set(c->xmin,  xmin, GMP_RNDN);
    mpfr_set(c->xmax,  xmax, GMP_RNDN);
    mpfr_set(c->ymax,  ymax, GMP_RNDN);
    coords_rect_to_center(c);
}


void coords_to( coords* c, mpfr_t cx, mpfr_t cy)
{
    mpfr_set(c->cx, cx, GMP_RNDN);
    mpfr_set(c->cy, cy, GMP_RNDN);
}


void coords_zoom(coords* c, double multiplier)
{
    mpfr_mul_d( c->_size,   c->_size,   multiplier, GMP_RNDN);
    mpfr_set(   *c->size,   c->_size,               GMP_RNDN);

    if (c->aspect > 1.0)
        mpfr_div_d(c->height,  c->width,    c->aspect,  GMP_RNDN);
    else
        mpfr_mul_d(c->width,   c->height,   c->aspect,  GMP_RNDN);

    coords_calculate_precision(c);
    coords_center_to_rect(c);
}


void coords_zoom_to( coords* c, int px, int py, int pw)
{
    double z = (double)pw / c->img_width;
    coords_center_to(c, px, py);
    coords_zoom(c, z);
}


void coords_size(coords* c,  mpfr_t size)
{
    mpfr_set(c->_size, size, GMP_RNDN);
    mpfr_set(*c->size, size, GMP_RNDN);
    coords_calculate_precision(c);
}


void coords_center_to(coords* c, int px, int py)
{
    mpfr_t  cx, cy;

    mpfr_init2(cx,  c->precision);
    mpfr_init2(cy,  c->precision);

    mpfr_set_si(cx, px, GMP_RNDN);
    mpfr_set_si(cy, py, GMP_RNDN);

    coords_pixel_to_coord(c, cx, cy);

    mpfr_set(   c->cx,      cx,     GMP_RNDN);
    mpfr_set(   c->cy,      cy,     GMP_RNDN);

    mpfr_clear(cx);
    mpfr_clear(cy);
}


void coords_reposition(coords* c,   int _p1x, int _p1y,
                                    int _p2x, int _p2y)
{
    mpfr_t  p1x, p1y, p2x, p2y;
    mpfr_t  pxd, pyd;

    mpfr_init2(p1x,     c->precision);
    mpfr_init2(p1y,     c->precision);
    mpfr_init2(p2x,     c->precision);
    mpfr_init2(p2y,     c->precision);
    mpfr_init2(pxd,     c->precision);
    mpfr_init2(pyd,     c->precision);

    mpfr_set_si(p1x,    _p1x,   GMP_RNDN);
    mpfr_set_si(p1y,    _p1y,   GMP_RNDN);

    coords_pixel_to_coord(c, p1x, p1y);

    mpfr_set_si(p2x,    _p2x,   GMP_RNDN);
    mpfr_set_si(p2y,    _p2y,   GMP_RNDN);

    coords_pixel_to_coord(c, p2x, p2y);

    mpfr_sub(   pxd,    p2x,    p1x,    GMP_RNDN);
    mpfr_sub(   pyd,    p2y,    p1y,    GMP_RNDN);

    mpfr_add(   c->cx,  c->cx,  pxd,    GMP_RNDN);
    mpfr_add(   c->cy,  c->cy,  pyd,    GMP_RNDN);

    mpfr_clear(p1x);
    mpfr_clear(p1y);
    mpfr_clear(p2x);
    mpfr_clear(p2y);
    mpfr_clear(pxd);
    mpfr_clear(pyd);
}


void coords_pixel_to_coord(coords* c, mpfr_t x, mpfr_t y)
{
    mpfr_t tmp;
    mpfr_t tmp2;

    mpfr_init2(tmp,  mpfr_get_prec(x));
    mpfr_init2(tmp2, mpfr_get_prec(x));

    if (y != NULL)
    {
        mpfr_div_si(tmp2,   c->width,   c->img_width,   GMP_RNDN);
        mpfr_mul(   tmp,    tmp2,       y,              GMP_RNDN);
        mpfr_sub(   y,      c->ymax,    tmp,            GMP_RNDN);
    }

    if (x != NULL)
    {
        mpfr_div_si(tmp2,   x,          c->img_width,   GMP_RNDN);
        mpfr_mul(   tmp,    tmp2,       c->width,       GMP_RNDN);
        mpfr_add(   x,      tmp,        c->xmin,        GMP_RNDN);
    }

    mpfr_clear(tmp2);
    mpfr_clear(tmp);
}


void precision_change(mpfr_t x, mp_prec_t p)
{
    mpfr_t tmp;

    mpfr_init2(tmp, p);
    mpfr_set(tmp,   x,      GMP_RNDN);
    mpfr_set_prec(  x,      p);
    mpfr_set(       x,      tmp,    GMP_RNDN);

    mpfr_clear(tmp);
}


void precision_change_gmp(mpf_t x, mp_bitcnt_t p)
{
    mpf_t tmp;

    mpf_init2(tmp, p);
    mpf_set(tmp,   x);
    mpf_set_prec(  x,      p);
    mpf_set(       x,      tmp);

    mpf_clear(tmp);
}


static void coords_mpfr_clear(coords* c)
{
    mpfr_clear(c->xmin);
    mpfr_clear(c->xmax);
    mpfr_clear(c->ymin);
    mpfr_clear(c->ymax);
    mpfr_clear(c->width);
    mpfr_clear(c->height);
    mpfr_clear(c->cx);
    mpfr_clear(c->cy);
    mpfr_clear(c->_size);
}
