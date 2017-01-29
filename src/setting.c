#include "setting.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "debug.h"


const char* options_no_yes[] = { "no", "yes", 0 };
const char* options_false_true[] = {"false", "true", 0 };

static char* sysloc = 0;


void setting_init()
{
    sysloc = setlocale(LC_NUMERIC, 0);
}


/*
http://stackoverflow.com/questions/2171527/how-do-i-remove-the-difference-in-locale-between-gui-and-commandline-interfaces-o
*/


char* setting_get_str(const char* buf, const char* name)
{
    const char* opt;
    char* ret = 0;
    size_t nlen = 0;
    size_t olen;

    if (name)
    {
        nlen = strlen(name);

        if (strlen(buf) < nlen + 2 || strncmp(name, buf, nlen))
            return false;
    }

    opt = buf + nlen;

    while (*opt == ' ')
        ++opt;

    olen = strlen(opt);

    ret = malloc(olen + 1);
    strcpy(ret, opt);

    return ret;
}


int setting_get_index(  const char* buf,   const char* name,
                        const char** options)
{
    size_t nlen = 0;
    int i;
    const char* opt = 0;

    if (name)
    {
        nlen = strlen(name);

        if (strlen(buf) < nlen + 2 || strncmp(name, buf, nlen))
            return false;
    }

    for (i = 0; options[i] != 0; ++i)
    {
        opt = buf + nlen;

        while (*opt == ' ')
            ++opt;

        if (strcmp(opt, options[i]) == 0)
            return i;
    }

    fprintf(stderr, "unknown option:%s for %s\n", opt, name);
    fprintf(stderr, "available options:");

    for (i = 0; options[i] != 0; ++i)
        fprintf(stderr, "%s ", options[i]);

    fprintf(stderr, "\n");

    return -1;
}


bool setting_get_long(  const char* buf,   const char* name,
                        long* val,  long min,   long max)
{
    size_t nlen = 0;
    int r;

    if (name)
    {
        nlen = strlen(name);

        if (strlen(buf) < nlen + 1 || strncmp(name, buf, nlen))
            return false;
    }

    setlocale(LC_NUMERIC, "C");
    r = sscanf(buf + nlen, "%ld", val);
    setlocale(LC_NUMERIC, sysloc);

    if (r != 1)
     return false;

    if (!min && !max)
        return true;

    if (*val < min || *val > max)
        return false;

    return true;
}


bool setting_get_version(const char* buf,   int* major,
                                            int* minor,
                                            int* revision)
{
    int r, maj, min, rev;
    setlocale(LC_NUMERIC, "C");
    r = sscanf(buf, "%d.%d.%d", &maj, &min, &rev);
    setlocale(LC_NUMERIC, sysloc);

    if (r == 3)
    {
        *major = maj;
        *minor = min;
        *revision = rev;
        return true;
    }

    return false;
}


bool setting_get_double(  const char* buf,   const char* name,
                            double* val,    double min,    double max)
{
    size_t nlen = 0;
    int r;

    if (name)
    {
        nlen = strlen(name);

        if (strlen(buf) < nlen + 1 || strncmp(name, buf, nlen))
            return false;
    }

    setlocale(LC_NUMERIC, "C");
    r = sscanf(buf + nlen, "%lf", val);
    setlocale(LC_NUMERIC, sysloc);

    if (r != 1)
        return false;

    if (!min && !max)
        return true;

    if (*val < min || *val > max)
        return false;

    return true;
}


bool setting_get_mpfr_t(const char* buf, const char* name, mpfr_t val)
{
    size_t nlen = 0;
    int r;

    if (name)
    {
        nlen = strlen(name);

        if (strlen(buf) < nlen + 1 || strncmp(name, buf, nlen))
            return false;
    }

    setlocale(LC_NUMERIC, "C");
    r = mpfr_set_str(val, buf + nlen, 10, GMP_RNDN);
    setlocale(LC_NUMERIC, sysloc);

    if (r)
        return false;

    return true;
}
