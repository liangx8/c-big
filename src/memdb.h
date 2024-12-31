#pragma once

typedef int (*CMP) (const void *,const void *);

struct ENTITY{
    CMP lt;
    CMP cmp;
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