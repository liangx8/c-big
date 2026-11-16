#include <stddef.h>
#include <stdio.h>

typedef struct _strlist strlist;
strlist    *strlist_create(void);

strlist    *all_line(FILE *file);
int         strlist_add(strlist *list, const char *value);
void        strlist_clear(strlist *list);
void        strlist_destory(strlist *list);
const char *strlist_get(const strlist *list, int index);
size_t      strlist_size(const strlist *list);
int         strlist_shuffle(strlist *list);

#define INIT_CAPACITY 10 * 1024