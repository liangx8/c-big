#ifndef STRLIST_H
#define STRLIST_H

#include <stddef.h>

typedef struct strlist strlist;

strlist *strlist_create(void);
int strlist_add(strlist *list, const char *value);
void strlist_destroy(strlist *list);
size_t strlist_size(const strlist *list);
const char *strlist_get(const strlist *list, size_t index);
int strlist_shuffle(strlist *list);
int strlist_order(strlist *list);
int strlist_sort(strlist *list); /* Alias for strlist_order. */

#endif
