#include <malloc.h>

struct BUFFER{
    long size;
    char data[];
};


void* new_buffer(long size){
    char *buf=malloc(size + 8);
    if(buf == NULL){
        return NULL;
    }
    struct BUFFER *ptr=(struct BUFFER *)buf;
    ptr->size=size;
    return buf;
}
void buffer_free(void *ptr){
    free(ptr);
}