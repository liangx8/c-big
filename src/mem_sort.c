
#include <stdint.h>
#include <string.h> // memcpy()
#include "abstract_db.h"
#include "sort_range.h"

#define DBG1 1

uint64_t _mem_partion(void *base,const struct ENTITY *ent,uint64_t p1,uint64_t p2)
{
    //p1,p2之间最小差２，这个由调用者检查
    const int incrment=ent->unitsize;
    char store[incrment];
    if(p1 + 2 == p2){
        void *pp1=base + p1 * incrment;
        void *pp2=base + (p2-1) * incrment;
        if(ent->lt(pp1,pp2)==0){
            memcpy(store,pp1,incrment);
            memcpy(pp1,pp2,incrment);
            memcpy(pp2,store,incrment);
        }
        return p1+1;
    }
    uint64_t store_idx=p1;
    void *povit=base + (p2 -1) * incrment;
    void *pstore = base + (store_idx * incrment);
    void *pix;
    while(1){
        if(ent->lt(pstore,povit)){
            store_idx++;
            pstore += incrment;
        } else {
            break;
        }
    }
    pix=base + (store_idx * incrment);
    //将pstore的位置腾空
    memcpy(store,pstore,incrment);

    for(uint64_t ix=store_idx+1;ix<p2-1;ix++){
        pix += incrment;
        if(ent->lt(pix,povit)){
            memcpy(pstore,pix,incrment);
            store_idx++;
            pstore += incrment;
            //将pstore的位置腾空
            memcpy(pix,pstore,incrment);
        }
    }
    memcpy(pstore,povit,incrment);
    memcpy(povit,store,incrment);
    return store_idx;
}
void mem_quick_sort(struct ABSTRACT_DB *db)
{
    uint64_t l1,l2;
    while(rng_pop(db->scops,&l1,&l2)==0){
repeat:
        uint64_t povit_idx=_mem_partion(db->raw,db->entity,l1,l2);
        int flag=0;
        if(povit_idx - l1 > 1){
            flag=1;
//            rng_push(db->scops,l1,povit_idx);
        }
        if(l2-povit_idx > 1){
            flag = flag | 0b10;
//            rng_push(db->scops,povit_idx,l2);
        }
        switch(flag){
            case 1:
                l2=povit_idx;
                goto repeat;
            case 3:
                rng_push(db->scops,l1,povit_idx);
            case 2:
                l1=povit_idx;
                goto repeat;
        }
    }
// #if DBG1
//     if(rng_pop(db->scops,&l1,&l2)){
//         FATAL(L"构建ABSTRACT_DB结构时，必须定义一个范围");
//     }
// #else
//     rng_pop(db->scops,&l1,&l2)
// #endif
    
}