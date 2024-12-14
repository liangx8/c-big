#pragma once

struct Entity{
    int   (*lt)    (const void *,const void *);
    int   (*cmp)   (const void *,const void *);
    // 从参数的文件名中建立一个索引对象,内容必须是以unitsize为单位对齐的内存
    // 并返回单位到total
    long (*index) (const char *,void **);
    const char tag[16];
    const int unitsize;
};
//"chinaid         "
//99,104,105,110,97,105,100,32,32,32,32,32,32,32,32,32
