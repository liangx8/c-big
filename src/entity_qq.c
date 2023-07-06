#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "error_stack.h"
#define UNITSIZE 12

/* 令结构以4字节对齐 */
#pragma pack(push,4)
struct qq_entity{
    uint32_t qq;
    uint64_t ph;
};
#pragma pack(pop)
int qq_gt(void *left,void *right)
{
    uint32_t *qq1=left;
    uint32_t *qq2=right;
    return *qq1 > *qq2;
}
int qq_lt(void *left,void *right)
{
    uint32_t *qq1=left;
    uint32_t *qq2=right;
    return *qq1 < *qq2;
}
int qq_same(void *obj1,void *obj2,int64_t total)
{
    for(int64_t ix=0;ix<total;ix++){
        uint32_t *qq1=obj1 + ix * 12;
        uint32_t *qq2=obj2 + ix * 12;
        if (*qq1 != *qq2){
            return 0;
        }
    }
    return -1;
}
/**
 * @param seq 当前记录在号
 * @param obj 当前记录的指针
*/
int qq_print(void *obj,int64_t seq,const char *color){
    struct qq_entity *qq=obj;
    if(color){
        printf("\033[0;%sm",color);
    }
    printf("%10ld:%10u %11lu",seq, qq->qq,qq->ph);
    if(color){
        printf("\033[0m\n");
    } else {
        printf("\n");
    }
    return 0;
}
