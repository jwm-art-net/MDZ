#ifndef FRAC_GENERALIZED_CELTIC_H
#define FRAC_GENERALIZED_CELTIC_H


#include <gmp.h>
#include <mpfr.h>


#include "types.h"


depth_t frac_generalized_celtic(depth_t depth,       long double bail,
                                long double wim,     long double wre,
                                long double c_im,    long double c_re,
                                long double wim2,    long double wre2  );


depth_t frac_generalized_celtic_mpfr(
                                depth_t depth,
                                mpfr_t bail,
                                mpfr_t wim,     mpfr_t wre,
                                mpfr_t c_im,    mpfr_t c_re,
                                mpfr_t wim2,    mpfr_t wre2, mpfr_t t1);


depth_t frac_generalized_celtic_gmp(
                                depth_t depth,
                                mpf_t bail,
                                mpf_t wim,     mpf_t wre,
                                mpf_t c_im,    mpf_t c_re,
                                mpf_t wim2,    mpf_t wre2, mpf_t t1);


#endif
