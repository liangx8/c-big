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
        size_t sz=BUF_SZ /12;
        if(sum < size){
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
#define BUF_SIZE 2048
int makedata(const char *dst,int64_t size)
{
    uint8_t *buf=malloc(BUF_SIZE*12);
    uint8_t *ptr=buf;
    FILE *fdst=fopen(dst,"w");
    if(fdst==NULL){
        ERROR_BY_ERRNO();
        return -1;
    }
    int64_t cnt=0;
    int idx=0;
    while(cnt < size){
        uint32_t *p32=(uint32_t*)ptr;
        int64_t  *p64;
        ptr +=4;
        p64=(long *)ptr;
        *p32=rand();
        *p64=cnt;
        cnt ++;
        idx ++;
        ptr +=8;
        if(idx == BUF_SIZE){
            fwrite(buf,12,BUF_SIZE,fdst);
            idx=0;
            ptr=buf;
        }
    }
    fwrite(buf,12,idx,fdst);
    fclose(fdst);
    free(buf);
    return 0;
}