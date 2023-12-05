#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "timestamp.h"
#include "error_stack.h"
#define OBJSIZE 12

typedef int (*itcmp)(const void *,const void *);

int qq_cmp(const void *,const void *);

void buble_sort(void *,size_t ,size_t ,itcmp);
void quick_sort(void *,size_t ,size_t ,itcmp);
long random(void);
struct sort_para{
    const char *desc;
    void *data;
    size_t total;
    size_t size;
    itcmp cmp;
};
uint8_t *randomdata(long total)
{
    uint8_t *ary=malloc(OBJSIZE*total);
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
    void (*sort)(void *,size_t,size_t,itcmp))
{
    char tmstr[256];
    long tm1,tm2;
    printf("正在\033[0;34m%s\033[0m, %ld个对象",para->desc,para->total);
    fflush(stdout);    
    now(&tm1);
    sort(para->data,para->total,para->size,para->cmp);
    now(&tm2);
    for(size_t ix=0;ix<para->total -1;ix++){
        if(para->cmp(para->data+ix*para->size,para->data + (ix+1) * para->size)>0){
            printf("数组没有被正确排序\n");
            return -1;
        }
    }
    timestamp_str(&tmstr[0],tm2-tm1);
    printf("正确排序，用时%s\n",tmstr);
    return 0;
}
int mem_sort_test(long total){
    struct sort_para pr;
    uint8_t *org=randomdata(total);
    if(org==NULL){
        return -1;
    }
    uint8_t *ary=malloc(total * OBJSIZE);
    if(ary==NULL){
        free(org);
        return -1;
    }
    memcpy(ary,org,total*OBJSIZE);
    pr.cmp=qq_cmp;
    pr.data=ary;
    pr.total=total;
    pr.size=OBJSIZE;
    pr.desc="系统快速";
    if(run_sort(&pr,qsort)){
        return -1;
    }
    memcpy(ary,org,total*OBJSIZE);
    pr.desc="快速排序";
    if(run_sort(&pr,quick_sort)){
        return -1;
    }
    memcpy(ary,org,total*OBJSIZE);
    pr.desc="冒泡排序";
    if(run_sort(&pr,buble_sort)){
        return -1;
    }
    free(org);
    free(ary);
   return 0;
}
