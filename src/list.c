#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include "error_stack.h"
#define ENTITY_SIZE 12
int qq_print(void *obj,int64_t seq);
int list(const char *fname, int64_t offset, int limit)
{
    FILE *fh = fopen(fname, "rb");
    char *buf;
    int bsize = limit * ENTITY_SIZE;
    if (fh == NULL)
    {
        ERROR_BY_ERRNO();
        return -1;
    }
    buf = malloc(bsize);
    if (fseek(fh, offset * ENTITY_SIZE, SEEK_SET))
    {
        ERROR_BY_ERRNO();
        goto err_ret;
    }
    
    if (buf == NULL)
    {
        ERROR_BY_ERRNO();
        goto err_ret;
    }
    int num = fread(buf, bsize, 1, fh);
    if (num < 1)
    {
        ERROR_BY_ERRNO();
        goto err_ret;
    }
    fclose(fh);
    char *ptr=buf;
    for (int i = 0; i < limit; i++)
    {
        qq_print(ptr,offset+i);
        ptr +=12;
    }

    free(buf);
    return 0;
err_ret:
    fclose(fh);
    return -1;
}