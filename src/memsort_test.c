#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "entity.h"
#include "timestamp.h"
#include "error_stack.h"



void buble_sort(void *,size_t ,size_t ,const struct ENTITY *);
void quick_sort(void *,size_t ,size_t ,const struct ENTITY *);
void libqsort(void *mtr,size_t left,size_t right,const struct ENTITY *ent)
{
    char *ptr=mtr+left;
    size_t cnt=right-left;
    qsort(ptr,cnt,ent->unitsize,ent->cmp);
}
long random(void);
struct sort_para{
    const char *desc;
    void *data;
    size_t total;
    const struct ENTITY *ent;
};
uint8_t *randomdata(long total)
{
    uint8_t *ary=malloc(12*total);
    if(ary==NULL){
        return NULL;
    }
    for(long ix=0;ix<total;ix++){
        int32_t *iptr=(int32_t*)(ary + ix*12);
        int64_t *dptr=(int64_t*)(ary+ix*12 + 4);
        *iptr=(int32_t )random();
        *dptr= ix;
    }
    return ary;
}
int run_sort(
    struct sort_para *para,
    void (*sort)(void *,size_t ,size_t,const struct ENTITY *))
{
    char tmstr[256];
    long tm1,tm2;
    const int us=para->ent->unitsize;
    printf("正在\033[0;34m%s\033[0m, %ld个对象",para->desc,para->total);
    fflush(stdout);    
    now(&tm1);
    sort(para->data,0,para->total,para->ent);
    now(&tm2);
    for(size_t ix=0;ix<para->total -1;ix++){
        if(para->ent->cmp(para->data+ix*us,para->data + (ix+1) * us)>0){
            printf("数组没有被正确排序(%ld)\n",ix);
            return -1;
        }
    }
    timestamp_str(&tmstr[0],tm2-tm1);
    printf("正确排序，用时%s\n",tmstr);
    return 0;
}
extern const struct ENTITY qq_entity;
int mem_sort_test(long total){
    struct sort_para pr;
    const int us=qq_entity.unitsize;
    uint8_t *org=randomdata(total);
    if(org==NULL){
        return -1;
    }
    uint8_t *ary=malloc(total * us);
    if(ary==NULL){
        free(org);
        return -1;
    }
    memcpy(ary,org,total*us);
    pr.ent=&qq_entity;
    pr.data=ary;
    pr.total=total;
    pr.desc="系统快速";
    if(run_sort(&pr,libqsort)){
        return -1;
    }
    memcpy(ary,org,total*us);
    pr.desc="快速排序";
    if(run_sort(&pr,quick_sort)){
        return -1;
    }
    memcpy(ary,org,total*us);
    pr.desc="冒泡排序";
    if(run_sort(&pr,buble_sort)){
        return -1;
    }
    free(org);
    free(ary);
   return 0;
}
