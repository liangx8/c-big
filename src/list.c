#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include "status.h"
#include "entity.h"
#include "error_stack.h"
//#define ENTITY_SIZE 12
int list(const struct STATUS *stu, int64_t offset, int limit,int highlight)
{
    FILE *fh = fopen(stu->preform_dst, "rb");
    const struct ENTITY *ent=stu->payload;
    
    char *buf;
    int bsize = limit * ent->unitsize;

    if (fh == NULL)
    {
        ERROR(stu->preform_dst);
        ERROR_BY_ERRNO();
        return -1;
    }
    buf = malloc(bsize);
    if (fseek(fh, offset * ent->unitsize, SEEK_SET))
    {
        ERROR_BY_ERRNO();
        goto err_ret;
    }
    
    if (buf == NULL)
    {
        ERROR_BY_ERRNO();
        goto err_ret;
    }
    int num = fread(buf, ent->unitsize, limit, fh);
    if (ferror(fh))
    {
        ERROR_BY_ERRNO();
        goto err_ret;
    }
    fclose(fh);
    char *ptr=buf;
    for (int i = 0; i < num; i++)
    {
        if(highlight == offset + i){
            printf("\033[0;35m");
        }
        printf("%12ld,%s",offset + i,ent->str(ptr));
        if(highlight== offset +i){
            printf("\033[0m");
        }
        printf("\n");
        ptr +=ent->unitsize;
    }

    free(buf);
    return 0;
err_ret:
    fclose(fh);
    return -1;
}