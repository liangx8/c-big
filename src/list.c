#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include "error_stack.h"
#define ENTITY_SIZE 12

int list(const char *fname, int64_t offset, int limit)
{
    FILE *fh = fopen(fname, "rb");
    char *buf;
    int bsize = limit * ENTITY_SIZE;
    if (fh == NULL)
    {
        error_stack_by_errno(__FILE__,__LINE__);
        return -1;
    }
    buf = malloc(bsize);
    if (fseek(fh, offset * ENTITY_SIZE, SEEK_SET))
    {
        error_stack_by_errno(__FILE__,__LINE__);
        goto err_ret;
    }
    
    if (buf == NULL)
    {
        error_stack_by_errno(__FILE__,__LINE__);
        goto err_ret;
    }
    int num = fread(buf, bsize, 1, fh);
    if (num < 1)
    {
        error_stack_by_errno(__FILE__,__LINE__);
        goto err_ret;
    }
    fclose(fh);
    uint32_t *ptr=(uint32_t*)buf;
    for (int i = 0; i < limit; i++)
    {
        uint32_t qq=*ptr;
        ptr++;
        uint64_t *phone=(uint64_t*)ptr;
        printf("%10ld:%10u %11lu\n",offset+i, qq,*phone);
        ptr +=2;
    }

    free(buf);
    return 0;
err_ret:
    fclose(fh);
    return -1;
}