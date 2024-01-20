#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "error_stack.h"
#include "entity.h"
#define PRINT_WIDTH 30

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
const char * qq_str(const void *obj)
{
    const struct qq_ent *qq = obj;
    snprintf(print_str,PRINT_WIDTH,"%11u:%11lu",qq->qq,qq->ph);
    return print_str;
}
uint32_t qq_value(const struct qq_ent *ent)
{
    return ent->qq;
}
int qq_vcmp(uint32_t val,const struct qq_ent *ent)
{
    if(val == ent->qq){
        return 0;
    }
    if(val < ent->qq){
        return -1;
    }
    return 1;
}
const struct ENTITY qq_entity={
    (int (*)(const void *,const void *))qq_cmp,
    (int (*)(const void *,const void *))qq_lt,
    (int (*)(uint64_t,const void *))qq_vcmp,
    qq_str,
    12};


int test_qq_entity(void *pl)
{
    struct qq_ent q1,q2;
    q1.qq=259213601;
    q2.qq=3523406341;
    printf("%08x-%08x=%d\n",q1.qq,q2.qq,qq_entity.cmp(&q1,&q2));
    return 0;
}