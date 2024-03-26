#ifndef ENTITY_H_KKDLAK3K4KL3LK
#define ENTITY_H_KKDLAK3K4KL3LK
struct ENTITY{
    int (*cmp)(const void *,const void *);
    int (*lt)(const void *,const void *);
    int (*vallt)(unsigned long,const void *);
    const char *(*keystr)(const void*);
    //int (*print)(const void *,const void *,long offset);
    int (*list)(const void *,long,long,int);
    const int unitsize;
};
#endif