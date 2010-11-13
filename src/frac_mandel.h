#ifndef FRAC_MANDEL_H
#define FRAC_MANDEL_H


#include <gmp.h>
#include <mpfr.h>


#include "types.h"


depth_t frac_mandel(depth_t depth,
                long double wim,     long double wre,
                long double c_im,    long double c_re,
                long double wim2,    long double wre2  );


depth_t frac_mandel_mpfr(depth_t depth,
                mpfr_t t1,          /* t1 == temporary */
                mpfr_t frs_bail,
                mpfr_t wim,     mpfr_t wre,
                mpfr_t c_im,    mpfr_t c_re,
                mpfr_t wim2,    mpfr_t wre2  );


depth_t frac_mandel_gmp(depth_t depth, mpf_t frs_bail,
                mpf_t t1,          /* t1 == temporary */
                mpf_t wim,     mpf_t wre,
                mpf_t c_im,    mpf_t c_re,
                mpf_t wim2,    mpf_t wre2  );


#endif
