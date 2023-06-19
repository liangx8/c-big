#include <stdint.h>
#include <stdio.h>
#include <malloc.h>

#include "error_stack.h"

#define BUF_SZ 840 * 1024
int gentestdata(const char *src,const char *dst,int64_t size)
{
    // try exam two file by cmd "cmp -b src dst"
    char *buf;
    FILE *fsrc,*fdst;
    fsrc=fopen(src,"r");
    if(fsrc==NULL){
        ERROR_BY_ERRNO();
        return -1;
    }
    fdst=fopen(dst,"w+");
    if(fdst==NULL){
        ERROR_BY_ERRNO();
        fclose(fsrc);
        return -1;
    }

    buf=malloc(BUF_SZ);
    if(buf == NULL){
        ERROR_BY_ERRNO();
        fclose(fsrc);
        fclose(fdst);
        return -1;
    }
    printf("生成开发用数据:\n%s => %s (%ld)\n",src,dst,size);
    int64_t sum=0;
    while(1){
        size_t sz;
        if(sum < size){
            sz=BUF_SZ /12;
            if(sz + sum > size){
                sz=size-sum;
            }
        }
        sz=fread(buf,12,sz,fsrc);
        sum+=sz;
        if(ferror(fsrc)){
            ERROR_BY_ERRNO();
            free(buf);
            fclose(fsrc);
            fclose(fdst);
            return -1;
        }
        fwrite(buf,12,sz,fdst);
        if(feof(fsrc)){
            break;
        }
        if(sum>=size)break;

    }
    free(buf);
    fclose(fsrc);
    fclose(fdst);
    return 0;
}
int rand(void); // stdlib.h
int makedata(const char *dst,int64_t size)
{
    FILE *fdst=fopen(dst,"w+");
    if(fdst==NULL){
        ERROR_BY_ERRNO();
        return -1;
    }

    fclose(fdst);
    return 0;
}