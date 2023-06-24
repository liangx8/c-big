/*直接在内存中排序*/
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include "error_stack.h"
#include "bag.h"

/*冒泡排序*/
int buble_sort(uint8_t *buf,int bufsize,int increment,int (*cmp)(void *,void*))
{
    uint8_t save[increment];
    int cnt=bufsize/increment;
    for(int ix=0;ix<cnt;ix++){
        int touch=0;
        int update=0;
        uint8_t *ptr=buf+ix*increment;
        memcpy(save,ptr,increment);

        for(int iy=ix+1;iy<cnt;iy++){
            uint8_t *src=buf+iy*increment;
            if(cmp(ptr,src)>0){
                memcpy(ptr,src,increment);
                touch=1;
                update=iy;
            }
        }
        if(touch){
            memcpy(buf+update*increment,save,increment);
        }
    }
    return 0;
}
/* 快速排序 
*/
int64_t qpart(uint8_t *mtr,int increment,int64_t pos1,int64_t pos2,int (*lt)(void *,void*))
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
        if(lt(mtr+store_idx*increment,pivot)){
            store_idx++;
        } else {
            break;
        }
    }
    pstore=mtr+store_idx*increment;
    memcpy(store,pstore,increment);
    for(int64_t ix=store_idx+1;ix<pos2-1;ix++){
        uint8_t *pix=mtr+ix*increment;
        if(lt(pix,pivot)){

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

int quick_sort(uint8_t *mtr,int bufsize,int increment,int (*lt)(void *,void*))
{
    struct Bag *cot=bag_create();
    long total=bufsize/increment;
    int64_t stack=1;
#ifndef NDEBUG
    int64_t max=0;
#endif
    if(bag_put(cot,0,total)){
        ERROR("内部错误:struct Bag空间不够");
        return -1;
    }
    int64_t scope[2];
    while(bag_get(cot,&scope[0])==0){
        stack--;
        int64_t mid=qpart(mtr,increment,scope[0],scope[1],lt);
        int64_t sc0=scope[0];
        int64_t sc1=scope[1];
        if(mid-sc0>1){
            if(bag_put(cot,sc0,mid)){
                ERROR("内部错误:struct Bag空间不够");
                return -1;
            }
            stack++;
#ifndef NDEBUG
            if(stack>max)max=stack;
#endif
        }
        if(sc1-mid>2){
            
            if(bag_put(cot,mid+1,sc1)){
                ERROR("内部错误:struct Bag空间不够");
                return -1;
            }
            stack++;
#ifndef NDEBUG
            if(stack>max)max=stack;
#endif
        }
    }
    bag_free(cot);
#ifndef NDEBUG
    return max;
#else
    return 0;
#endif

}

/**
 * @brief sort data in memory
 * @param buf the pointer of memory that is going to sorting
 * @param bufsize size of buffer
 * @param increment size of object
 * @param cmp object compare 
*/
void mem_sort(uint8_t *buf,int bufsize,int increment,int (*cmp)(void *,void*)){

}


