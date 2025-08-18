#pragma once
#include <wchar.h>
typedef long (*CMP) (const void *,const void *);

struct ENTITY{
    CMP                                          lt;
    CMP                                          cmp;
    typeof(int (*)(const void*,long))            print;
    typeof(int (*)(void *))                      close;
    typeof(unsigned long (*)(const void *,long)) id;
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
void log_info(const wchar_t *fmt,...);

//abstract