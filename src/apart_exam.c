#include <stdint.h>
#include <stdio.h>
#include "status.h"
#include "error_stack.h"

#define UNIT_SIZE 12



#define LOG(m) printf("%s(%d):" m,__FILE__,__LINE__)

int full_path(char *,const char *);
void s32_apart_exam(struct STATUS *sta)
{
    FILE *fh;
    char val12[UNIT_SIZE];
    fh=fopen(sta->preform_dst,"r");
    if(fh==NULL){
        ERROR(sta->preform_dst);
        ERROR_BY_ERRNO();
        return;
    }
    fseek(fh,sta->scope[1]*UNIT_SIZE,SEEK_SET);
    if(fread(&val12[0],UNIT_SIZE,1,fh)<1){
        ERROR_BY_ERRNO();
        fclose(fh);
        return;
    }
    uint32_t pv=*((uint32_t *)val12);
    printf("value:%u\n",pv);
    rewind(fh);
    int64_t cnt;
    for(cnt=0;cnt < sta->scope[1];cnt++){
        uint32_t *ppv;
        if(feof(fh)){
            LOG("内部错误！");
            goto nopass;
        }
        if(fread(&val12[0],UNIT_SIZE,1,fh)<1){
            LOG("内部错误！");
            goto nopass;
        }
        ppv = (uint32_t *) val12;
        if(*ppv >= pv){
            goto nopass;
        }
        
    }

    cnt++;
    for(;cnt<sta->scope[3];cnt++){
        uint32_t *ppv;
        if(feof(fh)){
            LOG("内部错误！");
            goto nopass;
        }
        if(fread(&val12[0],UNIT_SIZE,1,fh)<1){
            LOG("内部错误！");
            goto nopass;
        }
        ppv = (uint32_t *) val12;
        if(*ppv < pv){
            goto nopass;
        }
    }
    printf("数据分区正确\n");
    fclose(fh);
    return;
nopass:
    printf("数据没有被排序%ld\n",cnt);
    fclose(fh);
}