#ifndef SETTING_H
#define SETTING_H


#include <stdio.h>
#include <stdbool.h>
#include <mpfr.h>

extern const char* options_no_yes[];
extern const char* options_false_true[];


void    setting_init();


char*   setting_get_str(    const char* buf,    const char* name);

int     setting_get_index(  const char* _buf,   const char* name,
                            const char** options);

bool    setting_get_long(   const char* buf,    const char* name,
                            long*   val,    long min,   long max);

bool    setting_get_double( const char* buf,    const char* name,
                            double* val,    double min, double max);


/* this will also init val */
bool    setting_get_mpfr_t( const char* buf, const char* name,
                                                        mpfr_t val);

#endif
