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
int qq_cmp(const uint32_t *qq1,const uint32_t *qq2)
{
    return *qq1-*qq2;
}
void qq_fprint(FILE *hdl,const void *obj,int64_t seq, const char *color);
/**
 * @param seq 当前记录在号
 * @param obj 当前记录的指针
*/
void qq_print(const void *obj,int64_t seq,const char *color){
    qq_fprint(stdout,obj,seq,color);
}
void qq_fprint(FILE *hdl,const void *obj,int64_t seq, const char *color)
{
    const struct qq_entity *qq=obj;
    if(color){
        fprintf(hdl,"\033[0;%sm",color);
    }
    fprintf(hdl,"%10ld:%10u %11lu",seq, qq->qq,qq->ph);
    if(color){
        fprintf(hdl,"\033[0m\n");
    } else {
        fprintf(hdl,"\n");
    }
}
