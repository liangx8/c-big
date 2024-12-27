#pragma once


struct ENTITY{
    int (*lt)    (const void *p1,const void *p2);
    int (*cmp)   (const void *p1,const void *p2);
    int (*print) (void *pl,const void *p,long seq);
    int (*close) (void *pl);
    const int    unitsize;
};
struct RANGES;
struct MEMDB{
    const struct ENTITY *entity;
    struct RANGES       *scops;
    void                *payload;
    char                *raw;
};

struct ENV{
    int cpunum;
    int pid;
};