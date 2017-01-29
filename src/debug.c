/*
    gratiously copied:  from debug.h & debug.c
                        from non-sequencer
                        (C) 2008 Jonathan Moore Liles

    modified by james w. morris 2010
*/

#include "debug.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>


/*
static size_t src_dir_len = 0;
*/

void
warnf(  warning_t level,
        const char *file,
        const char *function, size_t line, const char *fmt, ... )
{
    #ifdef NDEBUG
    (void)file;
    (void)function;
    (void)line;
    #endif

    #ifdef RTH_NDEBUG
    if (strcmp(file, "render_threads.c") == 0)
        return;
    #endif

    va_list args;
    static const char *level_tab[] = {
        "message", "\033[1;32m",
        "warning", "\07\033[1;33m",
    };

    FILE *fp = W_MESSAGE == level ? stdout : stderr;

    #ifndef NDEBUG

    /*
    if (!src_dir_len)
        src_dir_len = strlen(SRC_DIR);
    */

    if ( file )
    {
        /*
        const char* fstr = file + src_dir_len + 1;
        fprintf( fp, "%35s", fstr );
        */
        fprintf( fp, "%35s", file);
    }

    if ( line )
        fprintf( fp, ":%4zi", line );

    if ( function )
        fprintf( fp, " %30s()", function );

    fprintf( fp, ": " );
    #endif

    if ( ((unsigned)( level << 1 ) + 1 ) <
        ( sizeof( level_tab ) / sizeof( level_tab[0] ) ) )
    {
        fprintf( fp, "%s", level_tab[( level << 1 ) + 1] );
    }

    if ( fmt )
    {
        va_start( args, fmt );
        vfprintf( fp, fmt, args );
        va_end( args );
    }

    fprintf( fp, "\033[0m" );
}
