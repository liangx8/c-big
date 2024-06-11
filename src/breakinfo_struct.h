#ifndef BREAKINFO_STRUCT_H_AMES93KK
#define BREAKINFO_STRUCT_H_AMES93KK
struct break_info{
    const char *indexname;
    long time1;
    long time2;
    void *pl;
    long scope_cnt; // scope表示的是区间，这个数字必须是偶数，
    long *scope;
};

#endif