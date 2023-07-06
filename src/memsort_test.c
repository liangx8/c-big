#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include "timestamp.h"
#include "error_stack.h"
// 27秒
#define TOTAL 3000000
//#define TOTAL 20000

int qq_gt(void *,void *);
int qq_lt(void *,void *);
int qq_same(void *,void *,int64_t);

void buble_sort(uint8_t *,int ,int ,int (*)(void *,void*));
int quick_sort(uint8_t *,int ,int ,int (*)(void *,void*));

uint8_t *create_data(const char *fname,int amount){
    uint8_t *ary=malloc(amount*12);
    FILE *fh=fopen(fname,"r");
    if(fh==NULL){
        print_current_error(fname);
        free(ary);
        return NULL;
    }
    fread(ary,amount,12,fh);
    fclose(fh);
    return ary;
}

void mem_sort_test(const char *fname){
    char tmstr[256];
    long tm1,tm2;
    printf("data from file %s\n",fname);
    uint8_t *ary=create_data(fname,TOTAL);
    if(ary==NULL){
        return;
    }
    printf("正在快速排序%d个对象",TOTAL);
    fflush(stdout);
    now(&tm1);
    int res=quick_sort(ary,TOTAL*12,12,qq_lt);
    now(&tm2);
    if(res<0){
        print_error_stack(stdout);
        return;
    }
    for(int64_t ix=0;ix<TOTAL-1;ix++){
        if(qq_lt(ary+(ix+1)*12,ary + ix *12)){
            printf("数组没有被正确排序\n");
            free(ary);
            return;
        }
    }

    timestamp_str(tmstr,tm2-tm1);
#ifndef NDEBUG
    printf("用时 %s\n 最大内存使用：%d\n　正确排序\n",tmstr,res);
#else
    printf("用时 %s\n　正确排序\n",tmstr);
#endif

   free(ary);
}
