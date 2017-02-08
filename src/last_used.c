#include "last_used.h"

#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include "debug.h"

typedef struct _last_used
{
    char* basedir;
    char* filename;
    char* name;
    char* ext;
    char* filepath;
    char* suggested;
} last_used;


static char* workspace = 0;
static last_used* mdz_lu = 0;
static last_used* png_lu = 0;
static last_used* map_lu = 0;
static const char* untitled="untitled";

static last_used*  _get_lu(lu_type lt)
{
    switch(lt) {
      case LU_MDZ:      return mdz_lu;
      case LU_PNG:      return png_lu;
      case LU_MAP:      return map_lu;
      default:          return 0;
    }
}


lu_type prece[LU_XXX_LAST][LU_XXX_LAST - 1] = {
                            { 0, 0, 0 },
                            { LU_PNG, LU_XXX_LAST },
                            { LU_MDZ, LU_XXX_LAST },
                            { LU_MDZ, LU_PNG, LU_XXX_LAST }  };


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
    if (lu->basedir)
        free(lu->basedir);
    /* filename is basename() derived */
    if (lu->name)
        free(lu->name);
    if (lu->ext)
        free(lu->ext);
    if (lu->filepath)
        free(lu->filepath);
    if (lu->suggested)
        free(lu->suggested);
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


void last_used_set_file(lu_type lt, const char* path)
{
    DMSG("type: %s, path:'%s'\n", _get_lu(lt)->ext, path);
    if (!path)
        return;

    last_used* lu = _get_lu(lt);

    if (lu->basedir && strcmp(lu->basedir, path) != 0) {
        free(lu->basedir);
        lu->basedir = 0;
    }
    if (!lu->basedir) {
        lu->basedir = strdup(path);
        lu->basedir = dirname(lu->basedir);
    }
    if (!lu->filename) {
        lu->filepath = strdup(path);
        lu->filename = basename(lu->filepath);
        const char* dp = strrchr(lu->filename, '.');
        if (!dp)
            lu->name = strdup(lu->filename);
        else {
            size_t nl = dp - lu->filename;
            lu->name = malloc(nl + 1);
            strncpy(lu->name, lu->filename, nl);
            *(lu->name + nl) = '\0';
        }
    }
    DMSG("lu->basedir: %s\n",lu->basedir);
    DMSG("lu->filename %s\n",lu->filename);
    DMSG("lu->name %s\n",lu->name);
    DMSG("lu->ext %s\n",lu->ext);
    DMSG("lu->filepath %s\n",lu->filepath);
    DMSG("lu->suggested %s\n",lu->suggested);

}


const char* last_used_suggest_dir(lu_type lt)
{
    last_used* lu = _get_lu(lt);

    if (lu->basedir)
        return lu->basedir;

    const char* dir = 0;

    for (int l = 0; prece[lt][l] != LU_XXX_LAST; ++l) {
        DMSG("checking %s name for %s\n", _get_lu(lt)->ext, _get_lu(prece[lt][l])->ext);
        if ((dir = last_used_get_basedir(prece[lt][l])))
            return dir;
    }

    return 0;
}


const char* last_used_suggest_filename(lu_type lt, const char* append)
{
    last_used* lu = _get_lu(lt);

    if (lu->filename)
        return lu->filename;

    const char* lun = 0;

    for (int l = 0; prece[lt][l] != LU_XXX_LAST; ++l) {
        DMSG("checking %s name for %s\n", _get_lu(lt)->ext, _get_lu(prece[lt][l])->ext);
        if ((lun = last_used_get_name(prece[lt][l])))
            break;
    }


    DMSG("lun '%s'\n", lun);

    if (!lun)
        lun = untitled;

    if (lu->suggested) {
        free(lu->suggested);
        lu->suggested = 0;
    }

    size_t ll = strlen(lun);
    size_t al = (append ? (strlen(append) + 1) : 0);
    size_t tl = ll + al + strlen(lu->ext) + 1;

    lu->suggested = malloc(tl);
    strcpy(lu->suggested, lun);

    if (lun == untitled && append) {
        *(lu->suggested + ll) = '-';
        strcpy(lu->suggested + ll + 1, append);
        strcpy(lu->suggested + ll + al, lu->ext);
    }
    else
        strcpy(lu->suggested + ll, lu->ext);

    return lu->suggested;
}


const char* last_used_get_name(lu_type lt)
{
    return _get_lu(lt)->name;
}

const char* last_used_get_ext(lu_type lt)
{
    return _get_lu(lt)->ext;
}

const char* last_used_get_basedir(lu_type lt)
{
    return _get_lu(lt)->basedir;
}

const char* last_used_get_filename(lu_type lt)
{
    return _get_lu(lt)->filename;
}

const char* last_used_get_filepath(lu_type lt)
{
    return _get_lu(lt)->filepath;
}

void last_used_reset_filename(lu_type lt)
{
    DMSG("resetting filename...\n");
    last_used* lu = _get_lu(lt);
    if (lu->filepath) {
        free(lu->filepath);
        lu->filepath = 0;
        lu->filename = 0;
        lu->name = 0;
    }
}


