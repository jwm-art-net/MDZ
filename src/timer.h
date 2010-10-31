#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

typedef struct
{
    struct timeval tv_start;
    struct timeval tv_end;
} Timer;

void timer_start(Timer* t);
void timer_stop(Timer* t);
long timer_get_elapsed(Timer* t);

#endif
