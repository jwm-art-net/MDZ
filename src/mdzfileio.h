#ifndef MDZFILEIO_H
#define MDZFILEIO_H

#define FBUFLEN 4096


#include <stdio.h>
#include <mpfr.h>
#include <stdbool.h>


enum /* mdz file section flags */
{
    MDZ_FRACTAL_SETTINGS =  0x0001,
    MDZ_PALETTE_DATA =      0x0002
};

enum /* mdz_flags  */
{
    MDZ_NORMAL =    0x0000,
    MDZ_DUPLICATE = 0x0001
};

struct mdzfile
{
    FILE* f;
    char* name;
    char buf[FBUFLEN];
    char* line;
    bool write;
    bool eof;
    char* test;

    int version_maj;
    int version_min;
    int version_rev;
    int flags;
};

typedef struct mdzfile mdzfile;

extern const char* mdz_file_header;
extern const char* mdz_dump_header;


mdzfile*    mdzfile_read_open(const char* filename);
void        mdzfile_close(mdzfile*);
int         mdzfile_err(mdzfile*, const char* msg);
bool        mdzfile_read(mdzfile*);

bool        mdzfile_skip_to(mdzfile*, const char* line);

char*   mdzfile_get_str(    mdzfile*,  const char* name);

int     mdzfile_get_index(  mdzfile*,  const char* name,
                            const char** options);

bool    mdzfile_get_long(   mdzfile*,  const char* name,
                            long*   val,    long min,   long max);

bool    mdzfile_get_double( mdzfile*,  const char* name,
                            double* val,    double min, double max);

/* this will also init val */
bool    mdzfile_get_mpfr_t( mdzfile*,  const char* name, mpfr_t val);

int     mdzfile_get_name_index( mdzfile*, const char** names);

bool    mdzfile_test_for_name(  mdzfile*, const char* str);

#endif
