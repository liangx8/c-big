#include <malloc.h>

struct BUFFER{
    long size;
    void *data;
};


void* new_buffer(long size){
    char *buf=malloc(size + 16);
    if(buf == NULL){
        return NULL;
    }
    struct BUFFER *ptr=(struct BUFFER *)buf;
    ptr->size=size;
    ptr->data=buf + 16;
    return buf;
}
void buffer_free(void *ptr){
    free(ptr);
}