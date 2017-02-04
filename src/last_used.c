#include "last_used.h"

#include <stdlib.h>
#include <string.h>
#include <libgen.h>



struct _last_used
{
    char* dir;
    char* filename;
    char* name;
    char* ext;
    char* filename_path;
    const last_used* cf;
};


last_used*  last_used_create(const char* ext)
{
    last_used* lu = calloc(1, sizeof(*lu));

    if (!lu)
        return 0;

    if (ext) {
        lu->ext = malloc(strlen(ext) + 1);
        if (lu->ext)
            strcpy(lu->ext, ext);
    }

    return lu;
}


void last_used_free(last_used* lu)
{
    if (lu->dir)
        free(lu->dir);
    if (lu->filename)
        free(lu->filename);
    if (lu->name)
        free(lu->name);
    if (lu->ext)
        free(lu->ext);
    if (lu->filename_path)
        free(lu->filename_path);
    free(lu);
}


void last_used_set(last_used* lu, const char* path)
{
    if (!path)
        return;

    if (lu->dir && strcmp(lu->dir, path) != 0) {
        free(lu->dir);
        lu->dir = 0;
    }
    if (!lu->dir) {
        lu->dir = strdup(path);
        lu->dir = dirname(lu->dir);
    }
    if (lu->filename && strcmp(lu->filename, path) != 0) {
        free(lu->filename);
        lu->filename = 0;
    }
    if (!lu->filename) {
        lu->filename_path = strdup(path);
        lu->filename = basename(lu->filename_path);
    }
}


void last_used_see_also(last_used* lu, const last_used* cf)
{
    lu->cf = cf;
}


char* last_used_suggest(last_used* lu)
{
    const char* lun = lu->name;

    if (!lun) {
        lun = last_used_get_name(lu->cf);
        if (!lun)
            lun = "untitled";
    }

    size_t ll = strlen(lun);
    char* ret = malloc(ll + strlen(lu->ext) + 1);
    strcpy(ret, lun);
    strcpy(ret + ll, lu->ext);

}


const char* last_used_get_name(const last_used* lu)
{
    return (lu ? lu->name : 0);
}

const char* last_used_get_ext(const last_used* lu)
{
    return (lu ? lu->ext : 0);
}

const char* last_used_get_dir(const last_used* lu)
{
    return (lu ? lu->dir : 0);
}

const char* last_used_get_filename(const last_used* lu)
{
    return (lu ? lu->filename : 0);
}

void last_used_reset_filename(last_used* lu)
{
    if (lu->filename_path) {
        free(lu->filename_path);
        lu->filename_path = 0;
        lu->filename = 0;
        lu->name = 0;
    }
}


