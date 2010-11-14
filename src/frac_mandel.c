#include "frac_mandel.h"

#include <math.h>

depth_t frac_mandel(            depth_t depth,
                                long double wim,     long double wre,
                                long double c_im,    long double c_re,
                                long double wim2,    long double wre2  )
{
    depth_t wz;
    for (wz = 1; wz <= depth; ++wz)
    {
        wim = 2.0 * wre * wim + c_im;
        wre = wre2 - wim2 + c_re;
        wim2 = wim * wim;
        wre2 = wre * wre;
        if (wim2 + wre2 > 4.0F)
            return wz;
    }
    return 0;
}

depth_t frac_mandel_mpfr(
                                depth_t depth,
                                mpfr_t bail,
                                mpfr_t wim,     mpfr_t wre,
                                mpfr_t c_im,    mpfr_t c_re,
                                mpfr_t wim2,    mpfr_t wre2, mpfr_t t1)
{
    depth_t wz;
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
        if (mpfr_greater_p(t1, bail))
            return wz;
    }
    return 0;
}


depth_t frac_mandel_gmp(
                                depth_t depth,
                                mpf_t bail,
                                mpf_t wim,     mpf_t wre,
                                mpf_t c_im,    mpf_t c_re,
                                mpf_t wim2,    mpf_t wre2, mpf_t t1)
{
    depth_t wz;
    for (wz = 1; wz <= depth; wz++)
        {
        /* wim = 2.0 * wre * wim + c_im; */
        mpf_mul(   t1,     wre,    wim);
        mpf_mul_ui(t1,     t1,     2);
        mpf_add(   wim,    t1,     c_im);
        /* wre = wre2 - wim2 + c_re; */
        mpf_sub(   t1,     wre2,   wim2);
        mpf_add(   wre,    t1,     c_re);
        /* wim2 = wim * wim; */
        mpf_mul(   wim2,   wim,    wim);
        /* wre2 = wre * wre; */
        mpf_mul(   wre2,   wre,    wre);
        /* if ((wim2 + wre2) > frs_bail) */
        mpf_add(   t1,     wim2,   wre2);
        if (mpf_cmp(t1, bail) > 0)
            return wz;
    }
    return 0;
}
