#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG
  #include <stdio.h>
  #include <string.h>
  #define DMSG( s ) \
    printf("DEBUG: %20s %4d\t%s\n", __FILE__, __LINE__, s );

  #define DMSG_D( s, d ) \
    printf("DEBUG: %20s %4d\t%s %d\n", __FILE__, __LINE__, s, d);

/*  #define DMSG( s ) \
    if (strcmp(__FILE__, "render_threads.c"))   \
        printf("DEBUG: %20s %4d\t%s\n", __FILE__, __LINE__, s );

  #define DMSG_D( s, d ) \
    if (strcmp(__FILE__, "render_threads.c"))   \
        printf("DEBUG: %20s %4d\t%s %d\n", __FILE__, __LINE__, s, d);
*/
#else
  #define DMSG( s )
  #define DMSG_D( s, d )
#endif

#endif
