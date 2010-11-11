#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cmdline.h"
#include "fractal.h"
#include "image_info.h"
#include "main.h"
#include "main_gui.h"
#include "my_png.h"
#include "palette.h"
#include "render.h"
#include "render_threads.h"
#include "setting.h"


image_info*      img;
random_palette   rnd_palette;
function_palette fun_palette;
char*            program_name = 0;


void duplicate(void);

void init_misc(void)
{
    if (!opts.width && !opts.height)
    {
        opts.width = img->user_width;
        if (opts.aspect)
            opts.height = img->user_width / opts.aspect;
        else
            opts.height = img->user_height;
    }
    else if (!opts.width)
    {
        if (opts.aspect)
            opts.width = opts.height * opts.aspect;
        else
            opts.width = img->user_height / img->aspect;
    }
    else if (!opts.height)
    {
        if (opts.aspect)
            opts.height = opts.width / opts.aspect;
        else
            opts.height = opts.width / img->aspect;
    }

    if (!opts.antialias)
        opts.antialias = 1;

    if (!opts.threads)
        opts.threads = DEFAULT_THREAD_COUNT;

    img->thread_count = opts.threads;

    image_info_set(img, opts.width, opts.height, opts.antialias);

    img->rnd_pal = &rnd_palette;

    fun_palette.offset = 0;
    fun_palette.stripe = 1;
    fun_palette.spread = 1;
}


int main(int argc, char** argv)
{
    program_name = argv[0];
    setting_init();
    int ret = process_args(argc, argv);
    if (ret < 0)
        goto quit2;

    srand(time(0));

    /* init random palette move to here from init_misc() */
    rnd_palette.r_strength = 1.0;
    rnd_palette.g_strength = 1.0;
    rnd_palette.b_strength = 1.0;

    rnd_palette.r_bands = 0.05;
    rnd_palette.g_bands = 0.08;
    rnd_palette.b_bands = 0.2;

    rnd_palette.offset = 0.0;
    rnd_palette.stripe = 1.0;
    rnd_palette.spread = 1.0;

    if (!palette_init())
        palette_randomize(&rnd_palette);

    img = image_info_create(MANDELBROT);

    init_misc();

    if (opts.dumpfile)
    {
        if (!image_info_load_all(img,
                                MDZ_FRACTAL_SETTINGS | MDZ_PALETTE_DATA,
                                opts.dumpfile))
        {
            perror("Failed to load dump file\n");
            ret = -1;
            goto quit1;
        }

        if (remove(opts.dumpfile) == -1)
        {
            perror("Can't delete temp file");
            /*  the code this is inherited from,
                exits here, but is there any real
                reason to do so?
            */
        }
    }
    else if (opts.settingsfile)
    {
        if (!image_info_load_all(img,
                                MDZ_FRACTAL_SETTINGS | MDZ_PALETTE_DATA,
                                opts.settingsfile))
        {
            fprintf(stderr, "Failed to load %s\n", opts.settingsfile);
            ret = -1;
            goto quit1;
        }
    }

    if (opts.palettefile)
    {
        if (!palette_load(opts.palettefile))
        {
            fprintf(stderr, "could not read palette file %s\n",
                            opts.palettefile);
            ret = -1;
            goto quit1;
        }
    }

    if (opts.logfile)
    {
        if (strcmp("-", opts.logfile) == 0)
            opts.logfd = stdout;
        else if (!(opts.logfd = fopen(opts.logfile, "w")))
        {
            fprintf(stderr, "could not open logfile %s for writing\n",
                            opts.logfile);
            ret = -1;
            goto quit1;
        }
    }

    if (opts.renderfile)
        render_to_file(img);
    else
        gui_init(&argc, &argv, img);

quit1:
    image_info_destroy(img);
    palette_free();

quit2:
    cleanup_opts();
    return ret;
}

void duplicate(void)
{
    char fname[] = "/tmp/mdz-XXXXXX";
    int fd;
    FILE* fp;
    pid_t result;
    fd = mkstemp(fname);

    if (fd == -1)
    {
        perror("Can't create temp file");
        return;
    }

    fp = fdopen(fd, "w+");
    if (fp == NULL)
    {
        perror("Can't create temp file\n");
        return;
    }

    if (!image_info_f_save_all(img, fp))
    {
        fprintf(stderr, "Could not write settings to temp file\n");
        return;
    }

    if (fclose(fp) != 0)
    {
        fprintf(stderr, "Failed to write settings to temp file\n");
        return;
    }
    result = fork();

    if (result == 0)
    {
        gui_close_display();
        execlp(program_name, program_name, "--"DUMP_COMMAND, fname, NULL);
        perror("Error while exec'ing program\n");
        return;
    }
    else if (result < 0)
    {
        perror("Could not fork\n");
        return;
    }

}
