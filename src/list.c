#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include "status.h"
#include "entity.h"
#include "error_stack.h"
//#define ENTITY_SIZE 12
int list(const struct STATUS *stu,int64_t hl, int64_t offset, int limit)
{
    FILE *fh = fopen(stu->preform_dst, "rb");
    if (fh == NULL)
    {
        ERROR(stu->preform_dst);
        ERROR_BY_ERRNO();
        return -1;
    }
    const struct ENTITY *ent=stu->payload;
    if (ent->list(fh,offset,hl,limit)){
        fclose(fh);
        return -1;
    }
    fclose(fh);
    return 0;
}