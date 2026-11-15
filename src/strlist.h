typedef struct _strlist strlist;
typedef unsigned long size_t;
strlist *   strlist_create(void);
int         strlist_add(strlist *list, const char *value);
void        strlist_clear(strlist *list);
void        strlist_destory(strlist *list);
const char *strlist_get(const strlist *list, int index);
size_t      strlist_size(const strlist *list);


#define INIT_CAPACITY 10 * 1024