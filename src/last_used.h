#ifndef LAST_USED_H
#define LAST_USED_H



typedef enum LU_TYPE
{
  LU_MDZ,
  LU_PNG,
  LU_MAP
} lu_type;



void  last_used_init(void);
void  last_used_cleanup(void);

void        last_used_set(lu_type, const char* filepath);
char*       last_used_suggest(lu_type);
const char* last_used_get_name(lu_type);
const char* last_used_get_dir(lu_type);
const char* last_used_get_filename(lu_type);
void        last_used_reset_filename(lu_type);

/*
 * typedef struct _last_used last_used;


last_used*  last_used_create(const char* ext);
void        last_used_free(last_used*);

void        last_used_set(last_used*, const char* filepath);

void        last_used_see_also(last_used*, const last_used*);

char*       last_used_suggest(last_used*);


const char* last_used_get_name(const last_used*);
const char* last_used_get_ext(const last_used*);
const char* last_used_get_dir(const last_used*);
const char* last_used_get_filename(const last_used*);

void        last_used_reset_filename(last_used*);
*/

#endif /* LAST_USED_H */

