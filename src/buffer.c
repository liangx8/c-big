#include <malloc.h>
#include "error_stack.h"

struct BUFFER{
    long size;
    char data[];
};


void* new_buffer(long size){
    char *buf=malloc(size + 8);
    if(buf == NULL){
        ERROR_BY_ERRNO();
        return NULL;
    }
    struct BUFFER *ptr=(struct BUFFER *)buf;
    ptr->size=size;
    return buf;
}
