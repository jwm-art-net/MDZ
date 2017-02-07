#ifndef LAST_USED_H
#define LAST_USED_H



typedef enum LU_TYPE
{
  LU_XXX_FIRST,
  LU_MDZ,
  LU_PNG,
  LU_MAP,
  LU_XXX_LAST
} lu_type;



void  last_used_init(void);
void  last_used_cleanup(void);

void  last_used_set_file(lu_type, const char* filepath);

/* return last used xxx, otherwise NULL  */
const char* last_used_get_name(lu_type);
const char* last_used_get_dir(lu_type);
const char* last_used_get_filename(lu_type);

const char* last_used_suggest_dir(lu_type);
const char* last_used_suggest_filename(lu_type, const char* untitled_append);

/* reset name/filename, but not dir/path */
void        last_used_reset_filename(lu_type);


#endif /* LAST_USED_H */

