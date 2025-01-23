#include <stdio.h>
#include <wchar.h>
#include "memdb.h"
#include "sort_range.h"

long partion(void *base,const struct ENTITY *ent,long l1,long l2);
void sort_onethread(struct MEMDB *db)
{
    long l1,l2;
    for(int ix=0;ix<15;ix++){
        db->entity->print(db->payload,ix);
    }
    while(rng_pop(db->scops,&l1,&l2)>=0){
        long pvt=partion(db->raw,db->entity,l1,l2);
        if(pvt-l1>1){
            rng_push(db->scops,l1,pvt);
        }
        if(l2-pvt>2){
            rng_push(db->scops,pvt+1,l2);
        }
        wprintf(L"%ld,%ld,%ld\n",l1,pvt,l2);
    }
    for(int ix=0;ix<15;ix++){
        db->entity->print(db->payload,ix);
    }
    wprintf(L"结束\n");
}