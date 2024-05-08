#ifndef BUFFER_H_KKE3K3KDK
#define BUFFER_H_KKE3K3KDK
struct BUFFER{
    long size;
    char data[];
};
struct BUFFER *new_buffer(long);

#endif