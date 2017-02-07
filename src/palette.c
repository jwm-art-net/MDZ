#include "palette.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "last_used.h"
#include "main_gui.h"

#define BUF_SIZE 256

#define DEFAULT_PALETTE "default.map"

//guint32* palette = NULL;

int pal_offset = 0;
int pal_indexes = 256;


static const char* pal_prefix[] = {
    "/usr/local/share/",
    "/usr/share/",
    NULL
};

static const char* pal_path[] = {
    "gfract/palettes/",
    "gkII/palettes/",
    "gkii/palettes/",
    "gnofract4d/maps/",
    "fractint/maps/",
    NULL
};

static char* pal_paths[12] = { 0 };






int palette_init(void)
{
    int p1;
    int p2;
    int done = 0;
    char* path = 0;
    int p1len;
    int p2len;
    char* file = 0;
    int filelen = strlen(DEFAULT_PALETTE);
    FILE* fd;
    int pp = 0;

    char* palfile = 0;

    palette = malloc(sizeof(*palette) * pal_indexes);

    if (!palette)
        return 0;

    for (p1 = 0; p1 < pal_indexes; ++p1)
        palette[p1] = 0;

    for (p1 = 0; pal_prefix[p1] != NULL && !done; ++p1)
    {
        p1len = strlen(pal_prefix[p1]);
        for (p2 = 0; pal_path[p2] != NULL; ++p2)
        {
            p2len = strlen(pal_path[p2]);
            path = malloc((p1len + p2len + 1) * sizeof(char));
            strcpy(path, pal_prefix[p1]);
            strcat(path, pal_path[p2]);
            file = malloc((p1len + p2len + filelen + 1) * sizeof(char));
            strcpy(file, path);
            strcat(file, DEFAULT_PALETTE);
            if ((fd = fopen(file, "r")))
            {
                if (!palfile)
                    palfile = strdup(file);
                if (pp < 12)
                    pal_paths[pp] = strdup(path);
                pp++;
                fclose(fd);
            }
            free(path);
            free(file);
        }
    }
    if (palfile)
    {
        if (palette_load(palfile))
            done = 1;
        free(palfile);
    }
    return done;
}


char ** palette_get_paths()
{
    return pal_paths;
}


int palette_load(char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return 0;

    if (!palette_read(fp))
    {
        fprintf(stderr, "Failed to load palette: %s", filename);
        fclose(fp);
        return 0;
    }

    last_used_set_file(LU_MAP, filename);

    fclose(fp);
    return 1;
}


int palette_read(FILE* fd)
{
    int i,r,g,b;
    char buf[BUF_SIZE];

    i = 0;
    while (fgets(buf, BUF_SIZE, fd) != NULL) {
        if (i >= 256)
            break;
        if (sscanf(buf, " %d %d %d", &r, &g,&b) != 3)
            break;
        palette[i] = RGB(r,g,b);
        i++;
    }

    if(i == 0)
        return 0;

    pal_indexes = i;

    return 1;
}


int palette_save(char* filename)
{
    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
        return 0;

    if (!palette_write(fp))
    {
        fprintf(stderr, "Failed to save palette: %s", filename);
        fclose(fp);
        return 0;
    }

    last_used_set_file(LU_MAP, filename);

    fclose(fp);
    return 1;
}


int palette_write(FILE* fd)
{
    int i = 0;

    for (i = 0; i < pal_indexes; ++i)
    {
        fprintf(fd, " %d %d %d\n",
                    RED(    palette[i] ),
                    GREEN(  palette[i] ),
                    BLUE(   palette[i] ));
    }

    return 1;
}



void palette_randomize(random_palette* rnd_pal)
{
    int i;
    int r_bnd, g_bnd, b_bnd;
    int r_bandcount, g_bandcount, b_bandcount;
    int ar, ab, ag, rr, rg, rb;
    guint32 r, g, b;
    gdouble r_hs, g_hs, b_hs;
    gdouble r_hsm, g_hsm, b_hsm;
    int* rnd_r;
    int* rnd_g;
    int* rnd_b;
    double r_bcsize, g_bcsize, b_bcsize;
    double r_band, g_band, b_band;
    double r_bdif, g_bdif, b_bdif;
    double r_difb, g_difb, b_difb;

    if (rnd_pal == 0)
        return;

    last_used_reset_filename(LU_MAP);

    if (rnd_pal->r_strength == 0)
        rnd_pal->r_strength = 0.01;
    r_hs = 1 - rnd_pal->r_strength;
    r_hsm = 128 * rnd_pal->r_strength;
    if (rnd_pal->g_strength == 0)
        rnd_pal->g_strength = 0.01;
    g_hs = 1 - rnd_pal->g_strength;
    g_hsm = 128 * rnd_pal->g_strength;
    if (rnd_pal->b_strength == 0)
        rnd_pal->b_strength = 0.01;
    b_hs = 1 - rnd_pal->b_strength;
    b_hsm = 128 * rnd_pal->b_strength;

    r_bandcount = pal_indexes * rnd_pal->r_bands;
    if (r_bandcount < 1)
        r_bandcount = 1;
    g_bandcount = pal_indexes * rnd_pal->g_bands;
    if (g_bandcount < 1)
        g_bandcount = 1;
    b_bandcount = pal_indexes * rnd_pal->b_bands;
    if (b_bandcount < 1)
        b_bandcount = 1;

    rnd_r = malloc(sizeof(*rnd_r) * (r_bandcount + 1));
    rnd_g = malloc(sizeof(*rnd_g) * (g_bandcount + 1));
    rnd_b = malloc(sizeof(*rnd_b) * (b_bandcount + 1));

    for (i = 0; i < r_bandcount; i++)
        rnd_r[i] = r_hsm - 255 *
            ((double)rand() / RAND_MAX) * rnd_pal->r_strength;

    for (i = 0; i < g_bandcount; i++)
        rnd_g[i] = g_hsm - 255 *
            ((double)rand() / RAND_MAX) * rnd_pal->g_strength;

    for (i = 0; i < b_bandcount; i++)
        rnd_b[i] = b_hsm - 255 *
            ((double)rand() / RAND_MAX) * rnd_pal->b_strength;

    rnd_r[r_bandcount] = rnd_r[0];
    rnd_g[g_bandcount] = rnd_g[0];
    rnd_b[b_bandcount] = rnd_b[0];

    r_bcsize = (double)r_bandcount / pal_indexes;
    g_bcsize = (double)g_bandcount / pal_indexes;
    b_bcsize = (double)b_bandcount / pal_indexes;
    r_band = g_band = b_band = 0;
    r_bnd = g_bnd = b_bnd = 0;

    for (i=0; i < pal_indexes; i++)
    {
        r_bnd = r_band;
        g_bnd = g_band;
        b_bnd = b_band;
        r_bdif = r_band - r_bnd;
        g_bdif = g_band - g_bnd;
        b_bdif = b_band - b_bnd;
        r_difb = 1 - r_bdif;
        g_difb = 1 - g_bdif;
        b_difb = 1 - b_bdif;
        rr = rnd_r[r_bnd] * r_difb + rnd_r[r_bnd + 1] * r_bdif;
        rg = rnd_g[g_bnd] * g_difb + rnd_g[g_bnd + 1] * g_bdif;
        rb = rnd_b[b_bnd] * b_difb + rnd_b[b_bnd + 1] * b_bdif;
        r = ar = r_hsm + RED(palette[i]) * r_hs + rr;
        g = ag = g_hsm + GREEN(palette[i]) * g_hs + rg;
        b = ab = b_hsm + BLUE(palette[i]) * b_hs + rb;
        r_band += r_bcsize;
        g_band += g_bcsize;
        b_band += b_bcsize;
        if (ar < 0 || ar > 255 ||
            ag < 0 || ag > 255 ||
            ab < 0 || ab > 255)
        {
            printf("\nR:%d\tG:%d\tB:%d", ar,ag,ab);
        }
        if (i >= rnd_pal->offset &&
            (i + rnd_pal->offset) % rnd_pal->stripe < rnd_pal->spread)
        {
            palette[i] = RGB(r,g,b);
        }
    }

    free(rnd_r);
    free(rnd_g);
    free(rnd_b);
}


void palette_apply_func(function_palette* funpal)
{
    int i;
    guint32 r, g, b;

    last_used_reset_filename(LU_MAP);

    for (i = 0; i < pal_indexes; i++) {
        r = RED(palette[i]);
        g = GREEN(palette[i]);
        b = BLUE(palette[i]);
        if (i >= funpal->offset &&
            (i + funpal->offset) % funpal->stripe < funpal->spread)
        {
            switch(funpal->func) {
                case PF_EX_RG:
                    palette[i] = RGB(g, r, b);
                    break;
                case PF_EX_GB:
                    palette[i] = RGB(r, b, g);
                    break;
                case PF_EX_BR:
                    palette[i] = RGB(b, g, r);
                    break;
                case PF_ROT_RGB:
                    palette[i] = RGB(g, b, r);
                    break;
                case PF_INV_RGB:
                    palette[i] = RGB(255 - r, 255 - g, 255 - b);
                    break;
                case PF_INV_R:
                    palette[i] = RGB(255 - r, g, b);
                    break;
                case PF_INV_G:
                    palette[i] = RGB(r, 255 - g, b);
                    break;
                case PF_INV_B:
                    palette[i] = RGB(r, g, 255 - b);
                    break;
                default:
                    break;
            }
        }
    }
}


void palette_rotate_backward(void)
{
    --pal_offset;

    if (pal_offset < 0)
        pal_offset = pal_indexes - 1;
}


void palette_rotate_forward(void)
{
    ++pal_offset;

    if (pal_offset == pal_indexes)
        pal_offset = 0;
}


void palette_shift(int offset)
{
    pal_offset = (pal_indexes + offset) % pal_indexes;
}


void palette_free()
{
    int pp = 0;

    free(palette);

    while(pal_paths[pp])
        free(pal_paths[pp++]);
}


void palette_apply(image_info* img, int x0, int y0, int width, int height)
{
    int x, y;
    guint32* rgb_data;
    int* raw_data;
    double sc = img->colour_scale;

    if (img->palette_ip)
    {
        for (y = y0; y < y0 + height; y++)
        {
            rgb_data = &img->rgb_data[y * width];
            raw_data = &img->raw_data[y * width];
            for (x = x0; x < x0 + width; x++, ++rgb_data, ++raw_data)
            {
                if (!*raw_data)
                    *rgb_data = 0;
                else
                {
                    /** FIXME: optimize this */
                    double ip_val = *raw_data * sc + pal_offset;
                    int ip_cval = ceil( ip_val );
                    double ip_diff = ip_cval - ip_val;
                    double ip_rdiff = 1.0 - ip_diff;
                    int ip_ind1 = (int)floor(ip_val) % (pal_indexes - 1);
                    int ip_ind2 = ip_cval % (pal_indexes - 1);
                    guint32 ip_c1 = palette[ip_ind1];
                    guint32 ip_c2 = palette[ip_ind2];

 guint8 ip_r = (guint8)(ip_diff * RED(  ip_c1) + ip_rdiff * RED(  ip_c2));
 guint8 ip_g = (guint8)(ip_diff * GREEN(ip_c1) + ip_rdiff * GREEN(ip_c2));
 guint8 ip_b = (guint8)(ip_diff * BLUE( ip_c1) + ip_rdiff * BLUE( ip_c2));

                    *rgb_data = RGB(ip_r, ip_g, ip_b);
                }
            }
        }
    }
    else
    {
        for (y = y0; y < y0 + height; y++)
        {
            rgb_data = &img->rgb_data[y * width];
            raw_data = &img->raw_data[y * width];
            for (x = x0; x < x0 + width; x++, ++rgb_data, ++ raw_data)
            {
                if (!*raw_data)
                    *rgb_data = 0;
                else
                {
                    double val = *raw_data * sc + pal_offset;
                    *rgb_data =
                        palette[(guint32)val % (pal_indexes - 1)];
                }
            }
        }
    }
}


guint32 get_pixel_colour(double val, gboolean palette_ip)
{
    if (val)
        val += pal_offset;

    if (palette_ip)
    {
        double cval,diff,rdiff;
        int ind1,ind2;
        guint32 c1,c2;
        guint8 r,g,b;


        cval = ceil(val);
        diff = cval - val;
        rdiff = 1.0 - diff;

        ind1 = ((guint32)floor(val)) % (pal_indexes-1);
        ind2 = (guint32)cval % (pal_indexes-1);

        c1 = palette[ind1];
        c2 = palette[ind2];

        r = (guint8)(diff * RED(c1) + rdiff * RED(c2));
        g = (guint8)(diff * GREEN(c1) + rdiff * GREEN(c2));
        b = (guint8)(diff * BLUE(c1) + rdiff * BLUE(c2));

        return RGB(r,g,b);
    }
    else
    {
        int index;
        if (val == 0)
            return 0;
        index = (guint32)val % (pal_indexes - 1);
        return palette[index];
    }
}

