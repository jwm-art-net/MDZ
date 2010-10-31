#include "timer.h"

void timer_start(Timer* t)
{
    gettimeofday(&t->tv_start, 0);
}

void timer_stop(Timer* t)
{
    gettimeofday(&t->tv_end, 0);
}

/* return elapsed time in microseconds */
long timer_get_elapsed(Timer* t)
{
    if (t->tv_start.tv_sec == t->tv_end.tv_sec)
        return t->tv_end.tv_usec - t->tv_start.tv_usec;
    else
        return (t->tv_end.tv_sec - t->tv_start.tv_sec) *
            1e6 + (t->tv_end.tv_usec - t->tv_start.tv_usec);
}
