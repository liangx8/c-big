/*
4字节qq号，接着8字节手机号，每个记录12字节
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "error_stack.h"
#include "entity.h"
#define PRINT_WIDTH 30
#define RECORD_SIZE 12
static char print_str[PRINT_WIDTH];
/* 令结构以4字节对齐 */
#pragma pack(push, 4)
struct qq_ent
{
    uint32_t qq;
    uint64_t ph;
};
#pragma pack(pop)
int qq_cmp(const uint32_t *qq1, const uint32_t *qq2)
{
    if(*qq1 == *qq2){
        return 0;
    }
    if(*qq1 > *qq2) {
        return 1;
    } 
    return -1;
}
int qq_lt(const uint32_t *qq1,const uint32_t *qq2)
{
    return *qq1 < *qq2;
}
/**
 * 
*/
const char *qq_keystr(const void *obj)
{
    const struct qq_ent *qq = obj;
    snprintf(print_str,PRINT_WIDTH,"%11u:%11lu",qq->qq,qq->ph);
    return print_str;
}
uint32_t qq_value(const struct qq_ent *ent)
{   
    return ent->qq;
}
int qq_vcmp(uint32_t val,const uint32_t *qq)
{
    return val <= *qq;
}

int qq_list(FILE *fh,long offset,long hl,int limit){
    char *buf;
    int bsize = limit * RECORD_SIZE;
    buf = malloc(bsize);
    if (fseek(fh, offset * RECORD_SIZE, SEEK_SET))
    {
        ERROR_BY_ERRNO();
        return -1;
    }
    int num = fread(buf, RECORD_SIZE, limit, fh);
    if (ferror(fh))
    {
        ERROR_BY_ERRNO();
        return -1;
    }
    char *ptr=buf;
    for (int i = 0; i < num; i++)
    {
        if(hl == offset + i){
            printf("\033[0;35m");
        }
        struct qq_ent *ent=(struct qq_ent*)ptr;
        printf("%12ld,%11u:%11lu",offset + i,ent->qq,ent->ph);
        if(hl == offset + i){
            printf("\033[0m");
        }
        ptr +=RECORD_SIZE;
        printf("\n");
    }

    free(buf);
    return 0;



}
const struct ENTITY qq_entity={
    (int (*)(const void *,const void *))qq_cmp,
    (int (*)(const void *,const void *))qq_lt,
    (int (*)(uint64_t,const void *))qq_vcmp,
    qq_keystr,
//    (int (*)(const void *,const void *,int64_t))qq_print,
    (int (*)(const void *,long,long,int))qq_list,
    RECORD_SIZE
};


int test_qq_entity(void *pl)
{
    struct qq_ent q1,q2;
    q1.qq=259213601;
    q2.qq=3523406341;
    printf("%08x-%08x=%d\n",q1.qq,q2.qq,qq_entity.cmp(&q1,&q2));
    return 0;
}