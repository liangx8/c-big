#pragma once
typedef long (*CMP) (const void *,const void *);
#ifndef uint64_t
typedef unsigned long uint64_t;
#endif
struct ENTITY{
    CMP                                          lt;
    CMP                                          cmp;
    typeof(int (*)(const void*,uint64_t,int))    print;
    typeof(int (*)(void *))                      close;
    typeof(uint64_t (*)(const void *,uint64_t))  id;
    const int                                    unitsize;
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
// void log_info(const wchar_t *fmt,...);
// #define log_err(fmt,args...) log_info(fmt,args)
//abstract
