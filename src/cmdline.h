#ifndef CMDLINE_H
#define CMDLINE_H

#include <stdio.h>


typedef struct
{
    char*   logfile;
    char*   settingsfile;
    char*   palettefile;
    int     randompalette;
    char*   dumpfile;
    FILE*   logfd;
    int     width;
    int     height;
    double  aspect;
    int     antialias;
    char*   renderfile;
    int     threads;
} options;

extern options opts;

int process_args(int argc, char** argv);
void cleanup_opts();

#endif
