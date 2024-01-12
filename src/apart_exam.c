#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include "entity.h"
#include "status.h"
#include "error_stack.h"

#define BUF_CNT 100000


#define LOG(m) printf("%s(%d):" m,__FILE__,__LINE__)

int full_path(char *,const char *);
void s32_apart_exam(struct STATUS *sta)
{
    FILE *fh;
    const struct ENTITY *ent=sta->payload;
    const int us=ent->unitsize;
    char pivot[ent->unitsize];
    fh=fopen(sta->preform_dst,"r");
    if(fh==NULL){
        ERROR(sta->preform_dst);
        ERROR_BY_ERRNO();
        return;
    }
    fseek(fh,sta->scope[1]*us,SEEK_SET);
    if(fread(&pivot[0],us,1,fh)<1){
        ERROR_BY_ERRNO();
        fclose(fh);
        goto exit_with_error;
    }
    printf("value: \033[0;33m%s\033[0m\n",ent->str(pivot));
    char *buf=malloc(BUF_CNT * us);
    rewind(fh);
    long num=fread(buf,us,BUF_CNT,fh);
    long cnt=0;
    if(num){
        long idx=0;
        while(1){
            if(cnt >= sta->scope[1]){
                break;
            }
            if(ent->cmp(buf+(idx * us),&pivot[0])>=0){
                goto nopass;
            }
            idx ++;
            cnt++;
            if(idx == num){
                if(feof(fh)){
                    break;
                }
                num=fread(buf,us,BUF_CNT,fh);
                if(num==0){
                    break;
                }
                idx=0;
            }
        }
    }
    fseek(fh,sta->scope[2] * us,SEEK_SET);
    num=fread(buf,us,BUF_CNT,fh);
    cnt ++;
    if(num){
        long idx=0;
        while(1)
        {
            if(cnt >= sta->scope[3]){
                break;
            }
            if(ent->cmp(buf+(idx * us),&pivot[0]) < 0){
                goto nopass;
            }
            idx++;
            cnt++;
            if(idx == num){
                if(feof(fh)){
                    break;
                }
                num=fread(buf,us,BUF_CNT,fh);
                if(num==0){
                    break;
                }
                idx=0;
            }

        }
    }
    free(buf);
    printf("数据分区\033[0;33m完成\033[0m\n");
    return;
    nopass:
    printf("数据\033[1;36m没有\033[0m排序(%ld)\n",cnt);
    return;
    exit_with_error:
    print_error_stack(stderr);
}