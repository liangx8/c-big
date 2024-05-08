#ifndef WNID_META_H_ELQEKRD9023K4
#define WNID_META_H_ELQEKRD9023K4
#include <stdio.h>
#include <stdint.h>
struct WNAMEID{
    FILE *dbh;
    long cur;
    long end;
    uint64_t total;
    void *subtitles;
    /* 每个记录有１６个字节开头，定义如下
        byte 0 - 7
        bit[0:19] 20 当前记录的长度（不包含16字节）
        bit[20:54] 35手机
        bit[55:61] 7其他字段的内容的标题索引号
        bit[62:63] 2性别
        byte 8 - 15 身份证号 */
    //uint64_t lead[2];
    void *cur_raw;
    size_t raw_max;
};

#define MASK_SIZE   0xfffff
#define MASK_MOBILE 0x7ffffffff
#define MASK_TITLES 0x7f
#define MASK_GENDER 0x3


#define GENDER_UNKNOWN 0
#define GENDER_MALE    (((uint64_t)1) << 62)
#define GENDER_FEMALE  (((uint64_t)2) << 62)
#define SHMASK_GENDER  (((uint64_t)3) << 62)


#define NAMEID_EOF     -2
#endif