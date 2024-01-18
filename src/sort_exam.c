#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include "status.h"
#include "entity.h"
#include "error_stack.h"

#define PRE(fmt,args...) printf("%s(%3d):" fmt,__FILE__,__LINE__,args)
#define BUF_SIZE 10000
uint32_t qq_value(const void *ent);
int is_sorted(FILE *fh,long offset,long total,const struct ENTITY *ent)
{
    const int us=ent->unitsize;
    int bs;
    bs=total>BUF_SIZE?BUF_SIZE:total;
    char *buf=malloc(bs*us);
    fseek(fh,offset * us,SEEK_SET);
    long cnt=0;
    while(1){
        long ss=fread(buf,us,bs,fh);
        for(long ix=0;ix<ss-1;ix++){
            if(ent->cmp(buf + ix * us,buf + (ix+1) * us)>0){
                ERROR("没排序");
                free(buf);
                return -1;
            }
        }
        cnt +=ss;
        if(cnt < total){
            fseek(fh,-us,SEEK_CUR);
        } else {
            break;
        }
    }
    
    free(buf);
    return 0;
}

/**
 * @brief 比较２过块，不论顺序，内容是否一样
 * @param f1 从当前位置开始取数
 * @param f2 从最开始取
*/
int same_block(FILE *f1,FILE *f2,int size,const struct ENTITY *ent)
{
    const int us=ent->unitsize;
    uint8_t *bsrc=malloc((size +1)*us + 4*size);
    uint8_t *bdst=bsrc+us;
    uint32_t *offsets=(uint32_t *)(bsrc + (size+1)*us);
    for(int ix=0;ix<size;ix++){
        offsets[ix]=ix;
    }
    rewind(f2);
    if(fread(bdst,us,size,f2)!=size){
        ERROR("internal error");
        free(bsrc);
        return -1;
    }
    int cnt=0;
    int dnum=size;
    while(cnt < size){
        if(fread(bsrc,us,1,f1)<1){
            ERROR("INTERNAL ERROR");
            free(bsrc);
            return -1;
        }
        int notfound=1;
        for(int ix=0;ix<dnum;ix++){
            uint8_t *ptr=bdst + offsets[ix]*us;
            if(ent->cmp(bsrc,ptr)==0){
                uint32_t tmp=offsets[dnum-1];
                offsets[dnum-1]=offsets[ix];
                offsets[ix]=tmp;
                dnum--;
                notfound=0;
                break;
            }
        }
        if(notfound){
            ERROR("不一样");
            free(bsrc);
            return -1;
        }
        cnt ++;
    }
    /*
    for(int ix=0;ix<size;ix++){
        if((ix !=0) &&(ix %16==0)){
            printf("\n");
        }
        printf("%4u ",offsets[ix]);
    }
    printf("\n");
    */
    free(bsrc);
    return 0;
}

#define SIZE 300


int64_t qsort_partition(FILE *fh,int64_t pos1,int64_t pos2,const struct ENTITY *ent); // sort32.c
int mem_sort(FILE *fh,int64_t pos,int64_t amount,char *buf,const struct ENTITY *ent); // sort32.c

size_t copy(const char *,const char *,size_t); // pathutil.c
int apart_exam(FILE *fh,long *scope,const struct ENTITY *ent); // apart_examp.c



const char *dst="/tmp/qq-sorted.bin";
const char *src="/home/tec/big/qq-test.bin";




int test_qsort_partition(const struct ENTITY *ent)
{
    const int us=ent->unitsize;
    long size=copy(dst,src,SIZE*us);
    FILE *fsrc=fopen(src,"r");
    if(fsrc == NULL){
        ERROR("internal error");
        return -1;
    }
    if(size){
        if(size % us){
            ERROR("不完整的数据");
            goto return_with_error;
        }
        long total=size/us;
        FILE *fdst=fopen(dst,"r+");
        if(fdst==NULL){
            ERROR_BY_ERRNO();
            goto return_with_error;
        }
        long mid=qsort_partition(fdst,0,total,ent);
        if(mid<0){
            goto return_with_error;
        } 
        long data[4];
        data[0]=0;
        data[3]=total;
        data[1]=mid;
        if(apart_exam(fdst,&data[0],ent)){
            goto return_with_error;
        }
        if(same_block(fsrc,fdst,total,ent)){
            goto return_with_error;
        }

            
        char *buf=malloc(total * us);
        if(mem_sort(fdst,0,total,buf,ent)){
            free(buf);
            goto return_with_error;
        }
        free(buf);
        if(is_sorted(fdst,0,total,ent)){
            fclose(fdst);
            goto return_with_error;
        }
        rewind(fsrc);
        if(same_block(fsrc,fdst,total,ent)){
            fclose(fdst);
            goto return_with_error;
        }
        fclose(fsrc);
        return 0;
    }
return_with_error:
    fclose(fsrc);
    return -1;
}