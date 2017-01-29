/*
    gratiously copied:  from debug.h & debug.c
                        from non-sequencer
                        (C) 2008 Jonathan Moore Liles
    -jwm 2010
*/

#ifndef _DEBUG_H
#define _DEBUG_H


#ifdef __cplusplus
extern "C" {
#endif


#include <stddef.h>
#include <stdio.h>

#ifndef __GNUC__
    #define __FUNCTION__ NULL
#endif


typedef enum
{
    W_MESSAGE = 0,
    W_WARNING,
} warning_t;


void
warnf ( warning_t level,
        const char *file,
        const char *function, size_t line, const char *fmt, ... );


#ifndef NDEBUG
#define DMSG( fmt, args... ) \
    warnf( W_MESSAGE, __FILE__, __FUNCTION__, __LINE__, fmt, ## args )

#define DWARN( fmt, args... ) \
    warnf( W_WARNING, __FILE__, __FUNCTION__, __LINE__, fmt, ## args )

#else

#define DMSG( fmt, args... )
#define DWARN( fmt, args... )

#endif

/* these are always defined */
#define MESSAGE( fmt, args... ) \
    warnf( W_MESSAGE,__FILE__, __FUNCTION__, __LINE__, fmt, ## args )

#define WARNING( fmt, args... ) \
    warnf( W_WARNING,__FILE__, __FUNCTION__, __LINE__, fmt, ## args )


#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif


#endif
