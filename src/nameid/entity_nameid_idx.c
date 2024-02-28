#include <stdio.h>
#include <stdint.h>


// 身份证号的索引结构
struct nameid_idx_ent {
    // 身份证号
    uint64_t id;
    // 在正文中的偏移
    off_t pos;
};

int id_idx_lt(const uint64_t *id1,const uint64_t *id2)
{
    return *id1 < *id2;
}

int id_idx_cmp(const uint64_t *id1,const uint64_t *id2)
{
    if(*id1 == *id2){
        return 0;
    }
    if(*id1 > *id2) {
        return 1;
    } 
    return -1;
}