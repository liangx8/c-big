#include <stdio.h>
#include <wchar.h>
#include <malloc.h>
#include <stdlib.h>
#include "error_stack.h"
#include "memdb.h"
#include "sort_range.h"

int id_lt(const unsigned long*,const unsigned long*);
int id_cmp(const unsigned long*,const unsigned long*);
int rd_print(void * _,const unsigned long* val,long seq)
{
    wprintf(L"%8ld:%10ld\n",seq,*val);
    return 0;
}
int rd_close(struct MEMDB *db)
{
    rng_release(db->scops);
    free(db);
    
    return 0;
}
const struct ENTITY rd_entity={
    (CMP)                               id_lt,
    (CMP)                               id_cmp,
    (int (*)(void *,const void *,long)) rd_print,
    (int (*)(void *))                   rd_close,
    //L"居民身份证查询数据库",
    8

};
const struct MEMDB* random_db(long size)
{
    void *base=malloc(sizeof(struct MEMDB)+size*sizeof(long));
    struct MEMDB *db=base;
    struct RANGES *scops=rng_new();
    rng_push(scops,0L,size);
    db->payload=base;
    db->scops=scops;
    db->entity=&rd_entity;
    db->raw=base+sizeof(struct MEMDB);
    
    long *ptr=(long*)db->raw;
    for(int idx=0;idx<size;idx++){
        *ptr=random();
        ptr++;
    }
    return base;
}