#include <stdio.h>
#include <wchar.h>
#include <malloc.h>
#include <stdlib.h>
#include "abstract_db.h"
#include "sort_range.h"

long ulong_lt(const void*,const void*);
long long_cmp(const void*,const void*);
/**
 * @brief 生产64位的带符号整数
 */
long signed_rand(void)
{
    unsigned long x1=random();
    unsigned long x2=random();
    return (x1 << 31)+x2  + ((x2 << 40) & 0xc000000000000000);
}

int rd_print(const struct ABSTRACT_DB *db,uint64_t seq,int)
{
    long *ptr=(long*)(db->raw+seq * sizeof(long));
    wprintf(L"%8ld:0x%016lx\n",seq,*ptr);
    return 0;
}
int rd_close(struct ABSTRACT_DB *db)
{
    rng_release(db->scops);
    free(db);
    
    return 0;
}
uint64_t rd_id(const struct ABSTRACT_DB *db,uint64_t seq)
{
    uint64_t *ptr= (uint64_t*)(db->raw+seq * sizeof(long));
    return *ptr;
}
const struct ENTITY rd_entity={
    (CMP)                                       ulong_lt,
    (CMP)                                       long_cmp,
    (int (*)(const void *,uint64_t,int))            rd_print,
    (int (*)(void *))                           rd_close,
    (uint64_t (*)(const void*db,uint64_t seq)) rd_id,
    8
};
struct ABSTRACT_DB* random_db(long size)
{
    void *base=malloc(sizeof(struct ABSTRACT_DB)+size*sizeof(long));
    struct ABSTRACT_DB *db=base;
    struct RANGES *scops=rng_new();
    rng_push(scops,0L,size);
    db->scops=scops;
    db->entity=&rd_entity;
    db->raw=base+sizeof(struct ABSTRACT_DB);
    
    uint64_t *ptr=(uint64_t *)db->raw;
    for(int idx=0;idx<size;idx++){
        *ptr=signed_rand();
        ptr++;
    }
    return base;
}
