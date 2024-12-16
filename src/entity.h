#pragma once
#include <wchar.h>
struct db{
    long total;

};
struct Entity{
    int   (*lt)    (const void *,const void *);
    int   (*cmp)   (const void *,const void *);
    // 从参数的文件名中建立一个索引对象,内容必须是以unitsize为单位对齐的内存
    // 并返回单位到total
    struct db*  (*index) (void *,char **);
    int   (*print) (struct db*,const void*);
    const char     tag[16];
    const wchar_t *remark;
    const int      unitsize;
};
