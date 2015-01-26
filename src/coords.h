#ifndef COORDS_H
#define COORDS_H

#include <stdbool.h>
#include <stdio.h>
#include <mpfr.h>
#include <gmp.h>


#define DEFAULT_PRECISION 80


#if MPFR_VERSION_MAJOR == 2 && MPFR_VERSION_MINOR < 4
int mpfr_div_d(mpfr_t rop, mpfr_t op1, double _op2, mpfr_rnd_t rnd);
int mpfr_mul_d(mpfr_t rop, mpfr_t op1, double _op2, mpfr_rnd_t rnd);
#endif

#if __GNU_MP_VERSION < 5
typedef unsigned long mp_bitcnt_t;
#endif

void mpfr_to_gmp(mpfr_t from, mpf_t to);
void precision_change(mpfr_t x, mp_prec_t p);
void precision_change_gmp(mpf_t x, mp_bitcnt_t p);


struct coords
{
    int img_width;
    int img_height;

    mpfr_t xmin;
    mpfr_t xmax;

    mpfr_t ymin;
    mpfr_t ymax;

    mpfr_t width;
    mpfr_t height;

    mpfr_t cx;
    mpfr_t cy;

    mpfr_t* size;
    mpfr_t  _size;

    double aspect;
    long bail;

    mp_prec_t  precision;
    mp_prec_t  recommend; /* scant minimum */

    mp_bitcnt_t gmp_precision; /* actual precision used by gmp */

    double init_cx;
    double init_cy;
    double init_size;
};


typedef struct coords coords;


coords* coords_new( int img_width, int img_height, long bail,
                    double init_cx, double init_cy, double init_width);

void    coords_set(coords*, int img_width, int img_height);

coords* coords_dup(const coords*);
coords* coords_cpy(coords* dest, const coords* src);

void    coords_free(coords*);

void    coords_reset(coords*);

int     coords_calculate_precision(coords* c);
void    coords_set_precision(coords*, mpfr_prec_t precision);

void    coords_rect_to_center(coords*);
void    coords_center_to_rect(coords*);

void    coords_to( coords* c, mpfr_t cx, mpfr_t cy);

void    coords_zoom(        coords* c,  double multiplier);
void    coords_zoom_to(     coords* c,  int px, int py, int pw);
void    coords_size(        coords*,    mpfr_t size);

void    coords_center_to(   coords* c,  int px, int py);

void    coords_reposition(  coords* c,  int p1x, int p1y,
                                        int p2x, int p2y);

/*  x,y is the pixel to be converted, on return, x,y is the result */
void    coords_pixel_to_coord(coords*, mpfr_t x, mpfr_t y);


void    coords_get_rect(coords*,    mpfr_t xmin, mpfr_t xmax,
                                    mpfr_t ymax, mpfr_t width);

void    coords_get_rect_gmp(coords*,    mpf_t xmin, mpf_t xmax,
                                        mpf_t ymax, mpf_t width);

void    coords_set_rect(coords*,    mpfr_t xmin, mpfr_t xmax,
                                                 mpfr_t ymax);

#ifdef DEBUG
void    coords_dump(const coords*, const char* msg);
#endif

#endif
