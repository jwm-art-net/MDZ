#include "cmdline.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "render_threads.h"

#define HAS_ARG 1

options opts;

static struct option longopts[] = {
    {   DUMP_COMMAND,       HAS_ARG,    0,  'd' },
    {   "load-settings",    HAS_ARG,    0,  'l' },
    {   "load-palette",     HAS_ARG,    0,  'P' },
    {   "random-palette",   0,          0,  'p' },
    {   "log-file",         HAS_ARG,    0,  'L' },
    {   "width",            HAS_ARG,    0,  'w' },
    {   "height",           HAS_ARG,    0,  'h' },
    {   "aspect-ratio",     HAS_ARG,    0,  'a' },
    {   "render",           HAS_ARG,    0,  'R' },
    {   "anti-alias",       HAS_ARG,    0,  'A' },
    {   "threads",          HAS_ARG,    0,  't' },
    {   0,                  0,          0,  0   }
};

void show_usage(char* argvzero)
{
    printf("usage:  %s [options]\n", argvzero);
    printf("options:\n");
    printf("    -l, --load-settings [FILE]  saved settings file to load\n");
    printf("    -P, --load-palette [FILE]   .map palette file to load\n");
    printf("    -p, --random-palette        generate a random palette\n");
    printf("    -L, --log-file [FILE]       log settings to file\n");
    printf("    -w, --width [n]             set image width to n\n");
    printf("    -h, --height [n]            set image height to n\n");
    printf("    -a  --aspect [n.nnn]        set aspect ration to n.nnn\n");
    printf("    -R, --render [FILE]         just render an image to file\n");
    printf("    -A, --anti-alias [n]        set anti-aliasing factor to n\n");
    printf("    -t, --threads [n]           set number of threads to n.\n");
    printf("NOTE: Not all combinations of width, height, aspect ratio, and\n"
           "      settings file make sense. The settings files only store\n"
           "      the aspect ratio of an image, not it's dimensions.\n");
    printf("mdz Mandelbrot Deep Zoom\n\n");
    printf("(c) James W. Morris 2009-2010 http://jwm-art.net/mdz/\n");
    printf("This software is licensed using the GNU GPLv3\n\n");
}


int oom(const char* arg, const char* val)
{
    fprintf(stderr, "out of memory processing %s=%s\n"
                    "aborting command line processing!",
                    arg, val);
    return -1;
}

int process_args(int argc, char** argv)
{
    memset(&opts, 0, sizeof(options));
    opts.width = 0;
    opts.height = 0;
    opts.aspect = 0;
    opts.antialias = 0;

    int optcount = 0;
    while (longopts[optcount].name)
        ++optcount;

    char shortopts[optcount * 2 + 1];
    char* sop = shortopts;
    struct option * lop = longopts;

    for (; lop < &longopts[optcount]; ++lop)
    {
        *sop++ = lop->val;
        if (lop->has_arg)
            *sop++ = ':';
    }

    for (;;)
    {
        int c = getopt_long(argc, argv, shortopts, longopts, 0);

        if (c == -1)
            break;

        switch(c)
        {
        case 'd':
            opts.dumpfile = strdup(optarg);
            if (!opts.dumpfile)
                return oom(longopts[optind].name, optarg);
            break;

        case 'l':
            if (!(opts.settingsfile = strdup(optarg)))
                return oom(longopts[optind].name, optarg);
            break;

        case 'P':
            if (!(opts.palettefile = strdup(optarg)))
                return oom(longopts[optind].name, optarg);
            break;

        case 'p':
            opts.randompalette = 1;
            break;

        case 'L':
            if (!(opts.logfile = strdup(optarg)))
                return oom(longopts[optind].name, optarg);
            break;

        case 'w':
            opts.width = atoi(optarg);
            if (opts.width < 1 || opts.width > MAX_WIDTH)
            {
                fprintf(stderr, "width out of range 1 ~ %d", MAX_WIDTH);
                return -1;
            }
            break;

        case 'h':
            opts.height = atoi(optarg);
            if (opts.height < 1 || opts.height > MAX_WIDTH)
            {
                fprintf(stderr, "height out of range 1 ~ %d", MAX_HEIGHT);
                return -1;
            }
            break;

        case 'a':
            opts.aspect = atof(optarg);
            if (opts.aspect <= 0)
            {
                fprintf(stderr, "aspect-ratio must be above zero");
                return -1;
            }
            break;

        case 'A':
            opts.antialias = atoi(optarg);
            if (opts.antialias < 1 || opts.antialias > MAX_AA)
            {
                fprintf(stderr, "anti-alias out of range 1 ~ %d", MAX_HEIGHT);
                return -1;
            }
            break;

        case 'R':
            if (!(opts.renderfile = strdup(optarg)))
                return oom(longopts[optind].name, optarg);
            break;

        case 't':
            opts.threads = atoi(optarg);
            if (opts.threads < 1 || opts.threads > MAX_THREAD_COUNT)
            {
                fprintf(stderr, "thread count out of range 1 ~ %d",
                                MAX_THREAD_COUNT);
                return -1;
            }
            break;

        default:
            show_usage(argv[0]);
            return -1;
        }
    }
    if (opts.width && opts.height && opts.aspect)
        fprintf(stderr, "Ignoring specified aspect-ratio\n");
    return 0;
}

void cleanup_opts()
{
    if (opts.logfile)
    {
        if (strcmp("-", opts.logfile) != 0)
        {
            if (opts.logfd)
                fclose(opts.logfd);
        }
        free(opts.logfile);
    }
    if (opts.settingsfile)
        free(opts.settingsfile);
    if (opts.palettefile)
        free(opts.palettefile);
    if (opts.dumpfile)
        free(opts.dumpfile);
    if (opts.renderfile)
        free(opts.renderfile);
}
