#ifndef BUFFER_H_KKE3K3KDK
#define BUFFER_H_KKE3K3KDK
struct BUFFER{
    long size;
    void *data;
};
struct BUFFER *new_buffer(long);
void buffer_free(struct BUFFER*);
#endif