#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include "entity.h"
#include "werror.h"
#define BLOCK_SIZE 102400

int concat(FILE *fdst,FILE *fsrc,char *buf,int bs)
{
    while(1){
        long amt=fread(buf,1,bs,fsrc);
        if(feof(fsrc)){
            if(amt){
                long osz=fwrite(buf,1,amt,fdst);
                if(osz != amt){
                    WERROR(L"写文件错误%ld",osz);
                    return -1;
                }
            }
            break;
        }
        long osz=fwrite(buf,1,bs,fdst);
        if(osz != bs){
            WERROR(L"写文件错误%ld",osz);
            return -1;
        }
    }
    return 0;
}


long apart(const struct ENTITY *ent,void *pl,FILE *fdst)
{
    const char *ptr;
    const int US=ent->unitsize;
    char pd[US];
    //long ppos=US; // pivot的位置
    
    ptr=ent->next(pl);
    if(ptr==NULL){
        WERROR(L"未知的错误导致异常%ls",L"\n");
        return -1;
    }
    memcpy(&pd[0],ptr,US);
    char *lside=malloc(BLOCK_SIZE * US*2);
    char *rside=lside+BLOCK_SIZE * US;
    int lcnt=0;
    int rcnt=0;

    FILE *rf=tmpfile();

    while(1){
        ptr=ent->next(pl);
        if(ptr == NULL){
            if(ent->status(pl)==STATUS_EOF){
                break;
            } else {
                return -1;
            }
        }
        if(ent->lt(ptr,&pd[0])){
            // 左边
            memcpy(lside + lcnt * US,ptr,US);
            lcnt ++;
            if(lcnt == BLOCK_SIZE){
                long amt=fwrite(lside,US,BLOCK_SIZE,fdst);
                if(amt != BLOCK_SIZE){
                    WERROR(L"写入数据不对%ld",amt);
                    goto exit_onerror;
                }
                lcnt=0;
            }
        } else {
            // 右边
            memcpy(rside + rcnt * US,ptr,US);
            rcnt ++;
            if (rcnt == BLOCK_SIZE){
                long amt=fwrite(rside,US,BLOCK_SIZE,rf);
                if(amt != BLOCK_SIZE){
                    WERROR(L"写入数据不对%ld",amt);
                    goto exit_onerror;
                }
                rcnt=0;
            }
        }

    }
    if(rcnt){
        long amt=fwrite(rside,US,rcnt,rf);
        if(amt != rcnt){
            WERR_NO_MSG(errno,L"写临时错误(size:%ld,expect %d)\n",amt,rcnt);
            goto exit_onerror;
        }
    }
    if(lcnt){
        long amt=fwrite(lside,US,lcnt,fdst);
        if(amt != lcnt){
            WERR_NO_MSG(errno,L"写临时错误(size:%ld,expect %d)\n",amt,lcnt);
            goto exit_onerror;
        }

    }
    long ppos=ftell(fdst);
    CP_MSG(L"=========1>%ld\n",ppos);
    fseek(fdst,0,SEEK_END);
    CP_MSG(L"=========2>%ld\n",ftell(fdst));
    fwrite(&pd[0],US,1,fdst);
    rewind(rf);
    if(concat(fdst,rf,rside,BLOCK_SIZE * US)){
        goto exit_onerror;
    }
    long total=ftell(fdst)/US;
    ent->print(0,&pd[0],pl);
    fseek(fdst,4,SEEK_SET);
    wprintf(L"对\033[1m%d\033[0m条记录排序,中间位置%ld\n",total,ppos/US);
    free(lside);
    fclose(rf);
    return ppos/US;
exit_onerror:

    free(lside);
    fclose(rf);
    return -1;
}
