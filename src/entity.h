#ifndef ENTITY_H_KKDLAK3K4KL3LK
#define ENTITY_H_KKDLAK3K4KL3LK
struct ENTITY{
    int (*cmp)(const char *l,const char *r);
    int (*lt)(const char *l,const char *r);
    int (*print)(const char *val,const void *pl);
    int (*next)(const void *pl,char *data);
    const int unitsize;
};
#define DONE    -2
#endif