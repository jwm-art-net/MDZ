#ifndef RENDER_THREADS_H
#define RENDER_THREADS_H

#include <pthread.h>

#include "image_info.h"

#define DEFAULT_THREAD_COUNT 2
#define MAX_THREAD_COUNT 512

typedef enum RTH_LINE_STATUS
{
    LN_UNPROCESSED,
    LN_PROCESSED,
    LN_FINISHED,
} line_status;

typedef struct rthpridata rthpridata;

typedef struct RTH_DATA
{
    image_info* img;
    char*   lines_drawn;
    int     min_line_drawn;
    int     line_draw_count;
    int     thread_count;

    int check_stop_px;

    rthpridata* data;

} rthdata;


/****** ----------------------------------------------------------------
 ****** must use rth_create() to create an instance of rthdata.
 ****** and then call init on the instance returned.
 ****** don't call init during render!
 ******/

rthdata* rth_create();

int     rth_init(rthdata* rth,
                 int thread_count,  int line_draw_count,
                 image_info * img);

/****** ----------------------------------------------------------------
 ****** ui control: these are used by the ui for controlling
 ****** the rendering threads. rth_ui_start creates a watch thread
 ****** which simply waits for the gui to tell it start up the rendering
 ****** threads. this is done by ui_start_render, and can be stopped
 ****** by the ui_stop and ui_quit functions.
 ******/
int     rth_ui_init(rthdata* rth);
void    rth_ui_start_render(rthdata* rth);
void    rth_ui_stop_render(rthdata* rth);
void    rth_ui_stop_render_and_wait(rthdata* rth);
void    rth_ui_quit(rthdata* rth);
void    rth_ui_stop_timer(rthdata* rth);
void    rth_ui_wait_until_started(rthdata* rth);
double  rth_ui_get_render_time(rthdata* rth);

/****** ----------------------------------------------------------------
 ****** rth_set_next_line_cb will, for now, be automatically called
 ****** when the mpfr setting is turned on/off. we only have one
 ****** fractal type, but two different methods of calculating it -
 ****** using MPFR for arbitrary precision, or using the native hardware
 ****** floating point routines (aka long double).
 ******/
void    rth_set_next_line_cb(rthdata* rth,
                             int (*next_line_cb)(image_info*, int));

/****** ----------------------------------------------------------------
 ****** rth_process_lines_rendered sets indices of the lines_drawn
 ****** buffer to 1 if the line has been rendered.
 ****** it returns 0 if no lines have been rendered since the last
 ****** time it was called.
 ****** otherwise it returns the total number of lines in the image
 ****** that have been rendered.
 ****** it returns -1 when the image is complete
 ******/

int     rth_process_lines_rendered(rthdata* rth);

int     rth_render_should_stop(rthdata* rth); /* ?, 0 no, 1 yes */

int     rth_ui_wait_for_line_done(rthdata* rth);
#endif
