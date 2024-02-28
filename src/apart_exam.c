#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include "entity.h"
#include "status.h"
#include "error_stack.h"

#define BUF_CNT 100000


#define LOG(m) printf("%s(%d):" m,__FILE__,__LINE__)
/**
 * @brief 检测分区
 * @param fh    被检查的文件句柄
 * @param scope 4个数的数组，[0]左边界[1]分界点[3]右边界
 * @param ent   ...
 **/
int apart_exam(FILE *fh,long *scope,const struct ENTITY *ent,const void *pl)
{
    const int us=ent->unitsize;
    char pivot[us];
    char current[us];
    long pos1=scope[0];
    long pos2=scope[3];
    long mid=scope[1];
    fseek(fh,mid * us,SEEK_SET);
    if(fread(&pivot[0],us,1,fh)<1){
        ERROR_BY_ERRNO();
        return -1;
    }
    printf("value: \033[0;33m%s\033[0m\n",ent->str(pivot,pl));
    fseek(fh,pos1 * us,SEEK_SET);
    long cnt=0;
    while(1){
        if(fread(&current[0],us,1,fh)<1){
            ERROR_BY_ERRNO();
            return -1;
        }
        if(ent->cmp(current,pivot)>=0){
            ERROR("数据不正确1");
            return -1;
        }
        cnt ++;
        if(cnt == mid){
            break;
        }
    }
    cnt++;
    if(fread(&current[0],us,1,fh)<1){
        ERROR_BY_ERRNO();
        return -1;
    }
    while(1){
        if(fread(&current[0],us,1,fh)<1){
            ERROR_BY_ERRNO();
            return -1;
        }
        if(ent->cmp(current,pivot)<0){
            ERROR("数据不正确2");
            return -1;
        }
        cnt ++;
        if(cnt == pos2){
            break;
        }
    }
    return 0;
}
void s32_apart_exam(struct STATUS *sta)
{
    FILE *fh;
    const struct ENTITY *ent=sta->payload;
    fh=fopen(sta->preform_dst,"r");
    if(fh==NULL){
        ERROR(sta->preform_dst);
        ERROR_BY_ERRNO();
        return;
    }
    if(apart_exam(fh,sta->scope,ent,NULL)){
        print_error_stack(stderr);
        return;
    }
    printf("数据准确分区\n");
}