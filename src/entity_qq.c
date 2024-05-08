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
/* 令结构以4字节对齐 */
#pragma pack(push, 4)
struct qq_ent
{
    uint32_t qq;
    uint64_t ph;
};
#pragma pack(pop)
