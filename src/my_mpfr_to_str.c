#include "my_mpfr_to_str.h"


char* my_mpfr_to_str(mpfr_t n)
{
    static char d0[MAX_DP + 1];

    #if MPFR_VERSION_MAJOR < 2 || (MPFR_VERSION_MAJOR == 2 && MPFR_VERSION_MINOR < 4)

    char* s;
    char* s0;
    char* d;

    mp_exp_t e;

    if (mpfr_nan_p(n))
    {
        s = strdup("@NaN@");
        return s;
    }

    if (mpfr_inf_p(n))
    {
        if (MPFR_SIGN(n) > 0)
            s = strdup("@Inf@");
        else
            s = strdup("@-Inf@");
        return s;
    }

    if (mpfr_zero_p(n))
    {
        if (MPFR_SIGN(n) > 0)
            s = strdup("0.0");
        else
            s = strdup("-0.0");
        return s;
    }

    s = mpfr_get_str(NULL, &e, 10, 0, n, GMP_RNDN);

    d = d0;
    s0 = s;

    /* for a=3.1416 we have s = "31416" and e = 1 */

    if (*s == '-')
        *d++ = *s++;

    /* outputs mantissa */
    *d++ = *s++; e--;   /* leading digit */
    *d++ = '.';

    while (*s != '\0')
        *d++ = *s++; /* rest of mantissa */

    mpfr_free_str(s0);

    if (e)
    {
        *d++ = 'e';
        d += sprintf(d, "%ld", (long) e);
    }

    *d='\0'; /* <-- oooh mummy */

    #else
    mpfr_snprintf(d0, MAX_DP, "%.Re", n);
    #endif

    return strdup(d0);
}
