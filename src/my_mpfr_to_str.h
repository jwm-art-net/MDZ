#ifndef MY_MPFR_TO_STR_H
#define MY_MPFR_TO_STR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpfr.h>


#define MAX_DP 4096

/*  a wrapper for mpfr_sprintf (only mpfr >= 2.4)
*/

char* my_mpfr_to_str(mpfr_t n);


#endif
