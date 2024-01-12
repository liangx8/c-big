#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include "status.h"

#define PRE(fmt,args...) printf("%s(%3d):" fmt,__FILE__,__LINE__,args)
/**
 * @brief 比较２过块，不论顺序，每１２个字节是否是相同的uint32_t
*/
int same_block(const char *src,const char *dst,int len)
{
    const char *ps;
    int *idxes=malloc(len*sizeof(int));
    int remain_cnt=len;
    int retval=0;
    for(int ix=0;ix<len;ix++){
        idxes[ix]=ix*12;
    }
    ps = src;
    for(int ix=0;ix<len;ix++){
        uint32_t val= *((uint32_t *)ps);
        int notfound=1;
        ps += 12;
        for(int iy=0;iy<remain_cnt;iy++){
            uint32_t idx=idxes[iy];
            uint32_t *pd=(uint32_t *)(dst + idx);
            if(*pd == val){
                remain_cnt --;
                if(remain_cnt < 0 ){
                    PRE("not possible %d\n",remain_cnt);
                    retval=-1;
                    goto quit;
                }
                // 数字已经比较过，保存到数组最后
                idxes[iy]=idxes[remain_cnt];
                idxes[remain_cnt]=idx;
                notfound=0;
                break;
            }
        }
        if(notfound){
            retval=-1;
            goto quit;
        }
    }
quit:
    free(idxes);
    return retval;
}

#define SIZE 720
void check_dup(const struct STATUS *st)
{
    char *src=malloc(SIZE * 2);
    char *dst=src + SIZE;
    FILE *hdl=fopen(st->preform_src,"r");
    if(hdl==NULL){
        PRE("open file '%s' error:%s\n",st->preform_src,strerror(errno));
        return;
    }
    fread(src,12,60,hdl);
    if(ferror(hdl)){
        PRE("read file '%s' error:%s\n",st->preform_src,strerror(errno));
    }
    fclose(hdl);
    hdl=fopen(st->preform_dst,"r");
    if(hdl==NULL){
        PRE("open file '%s' error:%s\n",st->preform_dst,strerror(errno));
        return;
    }
    fread(dst,12,60,hdl);
    if(ferror(hdl)){
        PRE("read file '%s' error:%s\n",st->preform_src,strerror(errno));
    }
    fclose(hdl);
    if(same_block(src,dst,60)){
        printf("%s is not same as %s!\n",st->preform_src,st->preform_dst);
    }
    free(src);
}