#ifndef ENTITY_H_KKDLAK3K4KL3LK
#define ENTITY_H_KKDLAK3K4KL3LK
struct ENTITY{
    int (*cmp)(const void *,const void *);
    int (*lt)(const void *,const void *);
    int (*valcmp)(unsigned long,const void *);
    const char *(*str)(const void *);
    const int unitsize;
};
#endif