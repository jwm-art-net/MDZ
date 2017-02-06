#include "last_used.h"

#include <stdlib.h>
#include <string.h>
#include <libgen.h>



typedef struct _last_used
{
    char* dir;
    char* filename;
    char* name;
    char* ext;
    char* filename_path;
} last_used;


static last_used* mdz_lu = 0;
static last_used* png_lu = 0;
static last_used* map_lu = 0;

static last_used*  _get_lu(lu_type lt)
{
    switch(lt) {
      case LU_MDZ:      return mdz_lu;
      case LU_PNG:      return png_lu;
      case LU_MAP:      return map_lu;
      default:          return 0;
    }
}


static last_used*  _last_used_create(const char* ext)
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


static void _last_used_free(last_used* lu)
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


void last_used_init(void)
{
    mdz_lu = _last_used_create(".mdz");
    png_lu = _last_used_create(".png");
    map_lu = _last_used_create(".map");
}

void last_used_cleanup(void)
{
    _last_used_free(mdz_lu);
    _last_used_free(png_lu);
    _last_used_free(map_lu);
}


void last_used_set(lu_type lt, const char* path)
{
    if (!path)
        return;

    last_used* lu = _get_lu(lt);

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


char* last_used_suggest(lu_type lt)
{
    last_used* lu = _get_lu(lt);

    const char* lun = lu->name;

    if (!lun) {
        lun = last_used_get_name(lt);
        if (!lun)
            lun = "untitled";
    }

    size_t ll = strlen(lun);
    char* ret = malloc(ll + strlen(lu->ext) + 1);
    strcpy(ret, lun);
    strcpy(ret + ll, lu->ext);
}


const char* last_used_get_name(lu_type lt)
{
    return _get_lu(lt)->name;
}

const char* last_used_get_ext(lu_type lt)
{
    return _get_lu(lt)->ext;
}

const char* last_used_get_dir(lu_type lt)
{
    return _get_lu(lt)->dir;
}

const char* last_used_get_filename(lu_type lt)
{
    return _get_lu(lt)->filename;
}

void last_used_reset_filename(lu_type lt)
{
    last_used* lu = _get_lu(lt);
    if (lu->filename_path) {
        free(lu->filename_path);
        lu->filename_path = 0;
        lu->filename = 0;
        lu->name = 0;
    }
}


