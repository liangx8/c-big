/*直接在内存中排序*/
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include "error_stack.h"
#include "bag.h"
extern int int_act;
/*冒泡排序*/
void buble_sort(uint8_t *buf,size_t cnt,size_t increment,int (*cmp)(const void *,const void*))
{
    uint8_t save[increment];
    for(size_t ix=0;ix<cnt;ix++){
        int touch=0;
        size_t update=0;
        uint8_t *ptr=buf+ix*increment;
        memcpy(save,ptr,increment);

        for(size_t iy=ix+1;iy<cnt;iy++){
            uint8_t *src=buf+iy*increment;
            if(cmp(ptr,src)>0){
                memcpy(ptr,src,increment);
                touch=1;
                update=iy;
            }
            if(int_act==99){
                int_act=0;
                printf("total:%10ld,ix: %10ld,iy: %10ld\n",cnt,ix,iy);
            }
        }
        if(touch){
            memcpy(buf+update*increment,save,increment);
        }
    }
    return;
}
/* 快速排序 
*/
int64_t qpart(uint8_t *mtr,int increment,int64_t pos1,int64_t pos2,int (*cmp)(const void *,const void*))
{
    uint8_t pivot[increment];
    uint8_t store[increment];
    uint8_t *pstore;
    memcpy(pivot,mtr + (pos2-1)*increment,increment);
    int64_t store_idx=pos1;
    while(1){
        if(store_idx == pos2-1){
            return store_idx;
        }
        if(cmp(mtr+store_idx*increment,pivot)<0){
            store_idx++;
        } else {
            break;
        }
    }
    pstore=mtr+store_idx*increment;
    memcpy(store,pstore,increment);
    for(int64_t ix=store_idx+1;ix<pos2-1;ix++){
        uint8_t *pix=mtr+ix*increment;
        if(cmp(pix,pivot)<0){

            memcpy(pstore,pix,increment);
            memcpy(pix,store,increment);
            store_idx++;
            pstore +=increment;
            memcpy(store,pstore,increment);
        }
    }
    memcpy(mtr+(pos2-1)*increment,store,increment);
    memcpy(pstore,pivot,increment);
    return store_idx;
}

void quick_sort(uint8_t *mtr,size_t total,size_t increment,int (*lt)(const void *,const void*))
{
    struct Bag *cot=bag_create();
    if(bag_put2(cot,0,total)){
        ERROR("内部错误:struct Bag空间不够");
        return;
    }
    int64_t scope[2];
    while(bag_get(cot,&scope[0])==0){
        int64_t mid=qpart(mtr,increment,scope[0],scope[1],lt);
        int64_t sc0=scope[0];
        int64_t sc1=scope[1];
        if(mid-sc0>1){
            if(bag_put2(cot,sc0,mid)){
                ERROR("内部错误:struct Bag空间不够");
                return;
            }
        }
        if(sc1-mid>2){
            
            if(bag_put2(cot,mid+1,sc1)){
                ERROR("内部错误:struct Bag空间不够");
                return;
            }
        }
    }
    bag_free(cot);
    return;

}




