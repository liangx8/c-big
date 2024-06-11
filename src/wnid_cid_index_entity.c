#include <stdio.h>
#include <errno.h>
#include "wnid_meta.h"
#include "entity.h"
#include "werror.h"
struct NAMEID_ID_INDEX {
    struct WNAMEID nid;
    const char *index_name;
};
int chnid_cmp(const uint64_t *p1,const uint64_t *p2)
{
    if(*p1 > *p2){
        return 1;
    }
    if(*p1 == *p2){
        return 0;
    }
    return -1;
}
int chnid_lt(const uint64_t *p1,const uint64_t *p2)
{
    return *p1 < *p2;
}
const uint8_t id_idx_meta[]={INDEX_BY_ID,0,0,0,1,2,3,4,5,6,7,8};
int wnid_print(long,const void *,const void *);
const void* wnid_next_chnid(void *);
int wnid_status(void *my);
const struct ENTITY chnid_index_entity={
    (int             (*)(const void *,const void *))chnid_cmp,
    (int             (*)(const void *,const void *))chnid_lt,
    (int              (*)(long,const void *,void *))wnid_print,
    wnid_next_chnid,
    wnid_status,
    id_idx_meta,
    12
};
