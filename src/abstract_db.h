#pragma once

typedef long (*CMP) (const void *,const void *);

struct ENTITY{
    CMP             lt;
    CMP             cmp;
    int           (*print)     (const void *pl,long seq);
    int           (*close)     (void *pl);
    unsigned long (*id)        (const void *pl,long seq);
    const int       unitsize;
};
struct RANGES;
struct ABSTRACT_DB{
    const struct ENTITY *entity;
    struct RANGES       *scops;
    char                *raw;
};

struct ENV{
    int cpunum;
    int pid;
};
//abstract