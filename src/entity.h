#ifndef ENTITY_H_KKDLAK3K4KL3LK
#define ENTITY_H_KKDLAK3K4KL3LK
#include <stdint.h>
struct ENTITY{
    int (*cmp)  (const void *l,const void *r);
    int (*lt)   (const void *l,const void *r);
    int (*print)(long seq,const void *val,void *my);
    /* 从数据文件中获取下一个记录的地址，保存到*ptr中，大小必须是unitsize */
    const void* (*next) (void *my);
    int (*status)(void *my);
    /* 索引的识别，位于索引文件的开始，长度为unitsize,如果这个值为NULL无意义*/
    const uint8_t *index_meta;
    const int unitsize;
};
#define STATUS_OK
#define STATUS_ERROR   -1
#define STATUS_EOF     -2
#endif