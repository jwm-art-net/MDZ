#include "mdzfileio.h"


#include <stdlib.h>
#include <string.h>


#include "debug.h"
#include "setting.h"


static const char* mdzfile_header = "mdz fractal settings";


static char* buf_tidy(const char* str)
{
    int i;
    int n = strlen(str);
    char* ret = malloc(n + 1);

    strcpy(ret, str);

    for (i = 0; i < n; ++i)
    {
        if (ret[i] == '\t')
            ret[i] = ' ';
        else if (ret[i] < ' ')
        {
            ret[i] = '\0';
            break;
        }
    }

    return ret;
}


bool mdzfile_read(mdzfile* mf)
{
    if (mf->eof || mf->write)
        return false;
    do
    {
        if (mf->test)
        {
            strncpy(mf->buf, mf->test, FBUFLEN);
            free(mf->test);
            mf->test = 0;
        }
        else
        {
            if (!fgets(mf->buf, FBUFLEN, mf->f))
                return !(mf->eof = true);
        }

        if (mf->line)
            free(mf->line);

        mf->line = buf_tidy(mf->buf);

    } while (mf->line[0] == '#');

    return true;
}


bool mdzfile_test_for_name(mdzfile* mf, const char* name)
{
    size_t nlen = 0;

    if (!mdzfile_read(mf))
    {
        DMSG("What is the meaning of this!?\n");
        return false;
    }

    nlen = strlen(name);
    mf->test = strdup(mf->line);

    if (strlen(mf->line) < nlen + 2 || strncmp(name, mf->line, nlen))
    {
        return false;
    }

    return true;
}


bool mdzfile_skip_to(mdzfile* mf, const char* line)
{
    while (mdzfile_read(mf))
        if (strcmp(mf->line, line) == 0)
            return true;

    return false;
}


int mdzfile_err(mdzfile* mf, const char* msg)
{
    fprintf(stderr, "Error in file:%s\n%s\n", mf->name, msg);
    mdzfile_close(mf);
    return 0;
}


mdzfile* mdzfile_read_open(const char* filename)
{
    int i;
    size_t hlen = strlen(mdzfile_header);
    char* version;
    FILE* f = fopen(filename, "r");

    if (!f)
    {
        fprintf(stderr,
                "Failed to open file '%s' for reading!\n",
                filename);
        return 0;
    }

    mdzfile* mf = malloc(sizeof(*mf));

    if (!mf)
    {
        fprintf(stderr,
                "Out of memory to read file '%s'!\n",
                filename);
        fclose(f);
        return 0;
    }

    mf->f = f;
    mf->name = malloc(strlen(filename) + 1);

    if (!mf->name)
    {
        fprintf(stderr,
                "Out of memory to read file '%s'!\n",
                filename);
        free(mf);
        fclose(f);
        return 0;
    }

    strcpy(mf->name, filename);

    for(i = 0; i < FBUFLEN; ++i)
        mf->buf[i] = 0;

    mf->write = false;
    mf->eof = false;
    mf->line = 0;
    mf->test = 0;

    if (!mdzfile_read(mf))
        return 0;

    if (strncmp(mf->line, mdzfile_header, hlen))
    {
        mdzfile_err(mf, "not a recognisable mdz file\n");
        return 0;
    }

    version = mf->line + hlen;

    while (*version == ' ')
        ++version;

    if (*version == '\0')
        version = "0.0.8";

    if (!setting_get_version(version,   &mf->version_maj,
                                        &mf->version_min,
                                        &mf->version_rev ))
    {
        mdzfile_err(mf, "error in mdz file header/version\n");
        return 0;
    }

    return mf;
}


void mdzfile_close(mdzfile* mf)
{
    fclose(mf->f);
    mf->write = false;
    if (mf->name)
        free(mf->name);
    if (mf->line)
        free(mf->line);
    free(mf);
}


char* mdzfile_get_str(mdzfile* mf, const char* name)
{
    if (!mdzfile_read(mf))
        return 0;
    return setting_get_str(mf->line, name);
}


int mdzfile_get_index(mdzfile* mf, const char* name, const char** options)
{
    if (!mdzfile_read(mf))
        return 0;
    return setting_get_index(mf->line, name, options);
}


bool mdzfile_get_long(mdzfile* mf, const char* name,
                            long* val, long min, long max)
{
    if (!mdzfile_read(mf))
        return 0;

    return setting_get_long(mf->line, name, val, min, max);
}


bool mdzfile_get_double(mdzfile* mf, const char* name,
                            double* val, double min, double max)
{
    if (!mdzfile_read(mf))
        return 0;
    return setting_get_double(mf->line, name, val, min, max);
}


/* this will also init val */
bool mdzfile_get_mpfr_t(mdzfile* mf, const char* name, mpfr_t val)
{
    if (!mdzfile_read(mf))
        return 0;
    return setting_get_mpfr_t(mf->line, name, val);
}

int mdzfile_get_name_index(mdzfile* mf, const char** names)
{
    if (!mdzfile_read(mf))
        return -1;

    char* p = mf->line;
    int r;

    while(*p == ' ')    /* skip space */
        ++p;

    while(*p != '\0' && *p != ' ')
        ++p;            /* find next space */

    if (*p == '\0')     /* no space, but we don't know... */
        return setting_get_index(mf->line, NULL, names);

    *p = '\0';
    r = setting_get_index(mf->line, NULL, names);
    *p = ' ';

    return r;
}

