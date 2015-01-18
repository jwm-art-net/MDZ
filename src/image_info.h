#ifndef IMAGE_INFO_H
#define IMAGE_INFO_H

#include <stdlib.h>
#include <gtk/gtk.h>

#include <stdio.h>
#include <mpfr.h>

#include "coords.h"
#include "mdzfileio.h"
#include "random_palette.h"
#include "types.h"


#define MIN_PRECISION       2
#define MAX_PRECISION       99999999

#define MIN_DEPTH           1
#define MAX_DEPTH           INT32_MAX

#define MIN_BAILOUT         1
#define MAX_BAILOUT         INT32_MAX

#define DEFAULT_WIDTH       480
#define DEFAULT_HEIGHT      360
#define MAX_WIDTH           32767
#define MAX_HEIGHT          32767

#define DEFAULT_AAFACTOR    1
#define MAX_AA              8

#define DEFAULT_ASPECT      (1.0 + 1.0 / (double)3.0)



/*
    usage of mpfr for deeper precision than long double allows:

    there are two sets of data and functions. both sets have
    almost exactly the same name, except:

    the second set uses the mpfr_t type from the MPFR library.
    these variables are prefixed by p_ to differentiate them
    from their long double cousins.

    the second set of functions are also prefixed by p_.

    the first set do not have the p_ prefix and use the long
    double data type.

*/

typedef struct
{
    mpfr_t  c_re;
    mpfr_t  c_im;

} julia_info;




extern const char* fractal_str[];

typedef struct image_info
{
    mpfr_t  xmin, xmax, ymax, width;
    mpf_t   gxmin, gxmax, gymax, gwidth;

    /* saved mandel coords.                         */
    /* we need these when  switching back from      */
    /* julia mode                                   */
    mpfr_t  old_cx, old_cy, old_size;

    coords* pcoords; /* used for updating coords */

    depth_t depth;
    long bailout;

    int thread_count;
    int draw_lines;

    int*        raw_data;
    guint32*    rgb_data;

    gboolean j_pre;

    GtkWidget* drawing_area;    /* it's handy to keep this around */

    random_palette* rnd_pal;    /* again... it's handy */

    int real_width;             /* real size. differs from user_size */
    int real_height;            /* if anti-aliasing is used */
    int user_width;
    int user_height;

    double aspect;

    int aa_factor;

    int family;    /* mandel || julia */
    int fractal;    /* mandelbrot, burning ship, etc */

    double colour_scale;

    union
    {   /* different fractal types' parameters */
        julia_info julia;
    } u;

    bool    palette_ip;
    bool    zoom_new_win;

    bool        use_multi_prec;
    bool        use_rounding;
    mpfr_prec_t precision;
    bool        multi_prec_init_done;

    void* rth_ptr;

    int lines_drawn;

    bool ui_ref_center; /* UI uses center as location reference point */

} image_info;


image_info* image_info_create(int family, int fractal);
void    image_info_destroy( image_info* );
void    image_info_set(     image_info*,    int w, int h,
                                                int aa_factor );
void    image_info_clear_image( image_info*,    bool raw, bool rgb);
void    image_info_set_multi_prec( image_info*, bool use_multi_prec,
                                                bool use_rounding );
void    image_info_set_precision(  image_info*, mpfr_prec_t precision);
void    image_info_threads_change( image_info*, int thread_count);
void    image_info_reset_view(      image_info*);
void    get_center( image_info*,    mpfr_t x, mpfr_t y, mpfr_t mag);
int     image_info_save_all(    image_info*,    const char* filename);
int     image_info_f_save_all(  image_info*,    FILE* f);

/* see enum at top of mdzfileio.h for sect_flags */
int image_info_load_all(    image_info*,    int sect_flags,
                                            const char* filename);

/* these only work on the 'settings' section of the file...
    save_settings will bail with error (return 0) if the
    'settings' section is not immediately encountered.
*/

int image_info_save_settings(image_info*, FILE* fd);
int image_info_load_settings(image_info*, mdzfile*);

/* toggle between m-set & julia-set */
void image_info_switch_fractal(image_info*, int j_real_px, int j_imag_py);

#ifdef WITH_TMZ_CMDLINE
void image_tmz_save_settings(image_info * img, FILE* fd);
#endif

#endif
