#include "render_threads.h"

#include "debug.h"
#include "timer.h"

#include <string.h>


typedef enum RTH_RENDER_STATUS
{
    RT_STOP =       0x0002,
    RT_QUIT =       0x0004,
    RT_RENDERING =  0x0008,
} rth_status;


struct rthpridata
{
    char initialized;

    short thread_count;

    pthread_t*  threads;
    char        start;
    char        started;
    rth_status   status;

    char*   lines_rendered;

    int     next_line;

    int     check_stop_px_feedback;

    int     (*next_line_cb)(image_info* img, int line);

    int     min_line_rendered;
    int     total_lines_rendered;

    pthread_t   start_watch_thread;
    pthread_t   render_thread;

    pthread_mutex_t start_mutex;
    pthread_mutex_t started_mutex;
    pthread_mutex_t status_mutex;
    pthread_mutex_t next_line_mutex;
    pthread_mutex_t lines_rendered_mutex;
    pthread_mutex_t check_stop_px_mutex;

    pthread_cond_t  start_cond;
    pthread_cond_t  started_cond;
    pthread_cond_t  lines_rendered_cond;
    pthread_cond_t  check_stop_px_cond;

    Timer timing_info;
};


#ifndef NDEBUG
static void rth_status_dump(rthdata* rth)
{
    if (rth->data->status & RT_STOP) { DMSG("RT_STOP\n"); }
    if (rth->data->status & RT_QUIT) { DMSG("RT_QUIT\n"); }
    if (rth->data->status & RT_RENDERING) { DMSG("RT_RENDERING\n"); }
}
#endif


static void *   rth_init_start_watch(void * ptr);
static void *   rth_create_render(void * ptr);
static void *   rth_render(void * ptr);
static int      rth_next_line(rthdata* rth);


static pthread_attr_t       rth_attr;


rthdata* rth_create()
{
    DMSG("rth_create\n");

    rthdata* rth = malloc(sizeof(rthdata));

    if (!rth)
        return 0;

    memset(rth, 0, sizeof(rthdata));
    rth->data = malloc(sizeof(rthpridata));

    if (!rth->data)
    {
        free(rth);
        return 0;
    }

    memset(rth->data, 0, sizeof(rthpridata));

    DMSG("Created rthdata\n");

    return rth;
}


int rth_init(rthdata* rth, int thread_count,
                           int line_draw_count,
                           image_info * img)
{
    DMSG("rth_init\n");

    if (img)
        rth->img = img;

    rthpridata* data = rth->data;

    if (thread_count && thread_count != data->thread_count)
    {
        data->thread_count = thread_count;
        free(data->threads);
        data->threads = malloc(sizeof(pthread_t) * thread_count);

        if (!data->threads) {
            DMSG("failed to allocate thread data\n");
            return 0;
        }
    }

    data->start =  0;
    data->status = 0;

    free(data->lines_rendered);
    free(rth->lines_drawn);

    data->lines_rendered =
        malloc(img->user_height * sizeof(*data->lines_rendered));
    rth->lines_drawn =
        malloc(img->user_height * sizeof(*rth->lines_drawn));

    if (!data->lines_rendered || !rth->lines_drawn)
        return 0;

    data->next_line = 0;
    data->min_line_rendered = 0;
    rth->min_line_drawn =    0;
    rth->line_draw_count =   line_draw_count;

    if (!data->initialized)
    {
        data->initialized = 1;
        pthread_attr_init(&rth_attr);
        pthread_attr_setdetachstate(&rth_attr, PTHREAD_CREATE_JOINABLE);

        pthread_mutex_init(&data->start_mutex,          NULL);
        pthread_mutex_init(&data->started_mutex,        NULL);
        pthread_mutex_init(&data->status_mutex,         NULL);
        pthread_mutex_init(&data->next_line_mutex,      NULL);
        pthread_mutex_init(&data->lines_rendered_mutex, NULL);
        pthread_mutex_init(&data->check_stop_px_mutex,  NULL);

        pthread_cond_init(&data->start_cond,            NULL);
        pthread_cond_init(&data->started_cond,          NULL);
        pthread_cond_init(&data->lines_rendered_cond,   NULL);
        pthread_cond_init(&data->check_stop_px_cond,    NULL);
    }

    DMSG("initialized rthdata\n");

    return 1;
}

int rth_ui_init(rthdata* rth)
{
    DMSG("rth_ui_init (start watch thread)\n");

    int rc_err = pthread_create(&rth->data->start_watch_thread,
                                &rth_attr,
                                rth_init_start_watch,
                                (void *)rth);
    if (rc_err)
    {
        fprintf(stderr, "Failed to create start_watch thread\n");
        return 0;
    }
    return 1;
}

void * rth_init_start_watch(void * ptr)
{
    rthdata* rth = (rthdata*)ptr;
    rthpridata* data = rth->data;
    int rc_err;

    int rendering;

    int quit = 0;

    DMSG("starting start_watch thread\n");

    for(;;)
    {
        DMSG("waiting on start mutex\n");

        pthread_mutex_lock(&data->start_mutex);
        if (!data->start)
            pthread_cond_wait(&data->start_cond, &data->start_mutex);
        data->start = 0;
        pthread_mutex_unlock(&data->start_mutex);

        pthread_mutex_lock(&data->started_mutex);
        data->started = 0;
        pthread_mutex_unlock(&data->started_mutex);

        DMSG("start mutex unblocked\n");

        pthread_mutex_lock(&data->status_mutex);

        quit = data->status & RT_QUIT;

        if (data->status & RT_RENDERING)
        {
            DMSG("rendering, setting stop\n");
            data->status = RT_STOP;
            rendering = 1;
        }
        else
        {
            DMSG("not currently rendering\n");
            rendering = 0;
        }

        #ifndef NDEBUG
        rth_status_dump(rth);
        #endif

        pthread_mutex_unlock(&data->status_mutex);

        if (rendering)
        {
            DMSG("waiting for render thread to finish\n");
            pthread_join(data->render_thread, NULL);
        }

        if (quit)
        {
            DMSG("quiting start_watch thread\n");
            pthread_exit(0);
        }

        rc_err = pthread_create(&data->render_thread,
                                &rth_attr,
                                rth_create_render,
                                (void*)rth);
        if (rc_err)
        {
            DMSG("*** failed to create render thread ***\n");
            pthread_exit(0);
        }
    }
}


void * rth_create_render(void * ptr)
{
    DMSG("creating main render thread\n");

    rthdata* rth = (rthdata*)ptr;
    rthpridata* data = rth->data;

    if (!data->lines_rendered)
    {
        DMSG("*** ERROR: lines rendered buffer not allocated ***\n");
    }

    pthread_mutex_lock(&data->lines_rendered_mutex);
    memset(data->lines_rendered, 0, rth->img->user_height);
    data->total_lines_rendered = 0;
    data->min_line_rendered = 0;
    pthread_mutex_unlock(&data->lines_rendered_mutex);

    pthread_mutex_lock(&data->status_mutex);
    data->status = RT_RENDERING;
    #ifdef DEBUG
    rth_status_dump(rth);
    #endif
    pthread_mutex_unlock(&data->status_mutex);

    pthread_mutex_lock(&data->next_line_mutex);
    data->next_line = 0;
    pthread_mutex_unlock(&data->next_line_mutex);

    int rc_err, i;

    if (!rth->check_stop_px)
        rth->check_stop_px = 64;

    rth->thread_count = data->thread_count;
    int rh = rth->img->real_height;
    while (rth->thread_count > rh / 2)
        rth->thread_count /= 2;

    DMSG("creating threads: count %d", rth->thread_count);

    timer_start(&data->timing_info);

    for (i = 0; i < rth->thread_count; ++i)
    {
        rc_err = pthread_create(&data->threads[i],
                                &rth_attr,
                                rth_render, (void*)rth);
        if (rc_err)
        {
            fprintf(stderr, "\nrender thread #%d creation failed: %d\n",
                            i, rc_err);
            return 0;
        }
        DMSG("render thread created\n");
    }

    pthread_mutex_lock(&data->started_mutex);
    data->started = 1;
    pthread_cond_signal(&data->started_cond);
    pthread_mutex_unlock(&data->started_mutex);

    DMSG("waiting for render threads to finish\n");

    for (i = 0; i < rth->thread_count; ++i)
        pthread_join(data->threads[i], NULL);

    timer_stop(&data->timing_info);

    DMSG("render threads finished\n");

    pthread_mutex_lock(&data->status_mutex);
    data->status = RT_STOP;
    #ifdef DEBUG
    rth_status_dump(rth);
    #endif
    pthread_mutex_unlock(&data->status_mutex);

    pthread_exit(0);
}


void * rth_render(void * ptr)
{
    DMSG("starting main render loop\n");
    rthdata* rth = (rthdata*)ptr;
    rthpridata* data = rth->data;
    int quit = 0;

    while (rth_next_line(rth) && !quit)
    {
        pthread_mutex_lock(&data->status_mutex);
        quit = (data->status & RT_STOP) | (data->status & RT_QUIT);
        pthread_mutex_unlock(&data->status_mutex);
    }

    pthread_exit(0);
}


int rth_next_line(rthdata* rth)
{
    rthpridata* data = rth->data;
    int line;
    int aa = rth->img->aa_factor;

    pthread_mutex_lock(&data->next_line_mutex);
    line = data->next_line;
    data->next_line += aa;
    pthread_mutex_unlock(&data->next_line_mutex);

    if (line >= rth->img->real_height)
        return 0;

    int i;
    for (i = 0; i < aa; ++i)
        if (!(data->next_line_cb)(rth->img, line + i))
            return 0;

    pthread_mutex_lock(&data->lines_rendered_mutex);

    data->lines_rendered[line / aa] = 1;
    data->total_lines_rendered += aa;
    pthread_cond_signal(&data->lines_rendered_cond);

    if (data->total_lines_rendered == rth->img->real_height)
    {
        pthread_mutex_unlock(&data->lines_rendered_mutex);
        return 0;
    }

    pthread_mutex_unlock(&data->lines_rendered_mutex);
    return 1;
}


void rth_ui_start_render(rthdata* rth)
{
    rthpridata* data = rth->data;
    memset(rth->lines_drawn, 0, rth->img->user_height);
    DMSG("rth_start_render, setting start\n");
    rth->min_line_drawn = 0;
    pthread_mutex_lock(&data->start_mutex);
    pthread_cond_signal(&data->start_cond);
    data->start = 1;
    pthread_mutex_unlock(&data->start_mutex);
}


void rth_ui_stop_render(rthdata* rth)
{
    DMSG("rth_stop_render\n");
    pthread_mutex_lock(&rth->data->status_mutex);
    rth->data->status = RT_STOP | (rth->data->status & RT_RENDERING);
    #ifdef DEBUG
    rth_status_dump(rth);
    #endif
    pthread_mutex_unlock(&rth->data->status_mutex);
}


void rth_ui_stop_render_and_wait(rthdata* rth)
{
    DMSG("rth_stop_render_and_wait\n");
    pthread_mutex_lock(&rth->data->status_mutex);

    if (rth->data->status & RT_STOP) {
        #ifdef DEBUG
        rth_status_dump(rth);
        #endif

        pthread_mutex_unlock(&rth->data->status_mutex);
        return;
    }

    rth->data->status = RT_STOP | (rth->data->status & RT_RENDERING);
    #ifdef DEBUG
    rth_status_dump(rth);
    #endif
    pthread_mutex_unlock(&rth->data->status_mutex);
    pthread_join(rth->data->render_thread, NULL);
}


void rth_ui_quit(rthdata* rth)
{
    DMSG("rth_quit\n");
    rthpridata* data = rth->data;
    /* set status to quit and signal start watch to wake up.. */
    pthread_mutex_lock(&data->status_mutex);
    data->status |= RT_QUIT; /* preserve rendering status!! */
    #ifdef DEBUG
    rth_status_dump(rth);
    #endif
    pthread_cond_signal(&data->start_cond);
    pthread_mutex_unlock(&data->status_mutex);

    pthread_join(data->start_watch_thread, NULL);

    free(data->threads);
    free(data->lines_rendered);
    free(rth->lines_drawn);
}


void rth_ui_wait_until_started(rthdata* rth)
{
    DMSG("***** waiting for started condition *****\n");
    rthpridata* data = rth->data;
    pthread_mutex_lock(&data->started_mutex);
    if (!rth->data->started)
        pthread_cond_wait(&data->started_cond, &data->started_mutex);
    pthread_cond_signal(&data->started_cond);
    pthread_mutex_unlock(&data->started_mutex);
    DMSG("***** got started condition *****\n");
}


void rth_set_next_line_cb(rthdata* rth,
                             int (*next_line_cb)(image_info*, int))
{
    rth->data->next_line_cb = next_line_cb;
}


int rth_process_lines_rendered(rthdata* rth)
{
    int linesdone;
    int height = rth->img->user_height;
    rthpridata* data = rth->data;

    pthread_mutex_lock(&data->lines_rendered_mutex);
    linesdone = data->total_lines_rendered / rth->img->aa_factor;
    pthread_mutex_unlock(&data->lines_rendered_mutex);

    int miny = data->min_line_rendered;

    if (miny == linesdone)
        return 0;

    int maxy = miny + rth->line_draw_count + 1;

    if (maxy > height)
        maxy = height;

    if (maxy > linesdone)
        maxy = linesdone;

    if (miny < maxy)
    {
        struct timeval tv;
        struct timespec timeout;
        gettimeofday(&tv, NULL);
        timeout.tv_nsec = tv.tv_usec * 1000 + 500 * 1000;
        timeout.tv_sec = tv.tv_sec;

        pthread_mutex_lock(&data->lines_rendered_mutex);
        pthread_cond_timedwait(&data->lines_rendered_cond,
                               &data->lines_rendered_mutex,
                               &timeout);

        char* lr = &data->lines_rendered[miny];
        char* ld = &rth->lines_drawn[miny];
        int unrendered = 0;
        int i;

        for (i = miny; i < maxy; ++i, ++lr, ++ld)
        {
            if (*lr == 1)
            {
                *lr = 2;
                *ld = 1;
                if (!unrendered)
                   data->min_line_rendered = i;
            }
            else if (*lr == 0)
                unrendered = 1;
        }
        pthread_mutex_unlock(&data->lines_rendered_mutex);
    }
    return (linesdone < height) ? linesdone : -1;
}

int rth_render_should_stop(rthdata* rth)
{
    int ret;
    pthread_mutex_lock(&rth->data->status_mutex);
    ret = !!(rth->data->status & RT_STOP);
    pthread_mutex_unlock(&rth->data->status_mutex);
    #ifdef DEBUG
    if (ret) {
        DMSG("rth_render_should_stop == TRUE\n");
    }
    #endif
    return ret;
}

int rth_ui_wait_for_line_done(rthdata* rth)
{
    int linesdone;
    rthpridata* data = rth->data;

    pthread_mutex_lock(&data->lines_rendered_mutex);
    if (data->total_lines_rendered == rth->img->real_height)
    {
        pthread_mutex_unlock(&data->lines_rendered_mutex);
        return -1;
    }
    pthread_cond_wait(&data->lines_rendered_cond,
                      &data->lines_rendered_mutex);
    linesdone = data->total_lines_rendered / rth->img->aa_factor;
    pthread_mutex_unlock(&data->lines_rendered_mutex);

    return (linesdone < rth->img->user_height) ? linesdone : -1;
}

void rth_ui_stop_timer(rthdata* rth)
{
    timer_stop(&rth->data->timing_info);
}

double rth_ui_get_render_time(rthdata* rth)
{
    timer_stop(&rth->data->timing_info);
    return timer_get_elapsed(&rth->data->timing_info) / (double)1e6;
}
