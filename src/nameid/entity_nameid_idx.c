#include <stdio.h>
#include <stdint.h>


// 身份证号的索引结构
struct nameid_idx_ent {
    // 身份证号
    uint64_t id;
    // 在正文中的偏移
    long pos;
};

