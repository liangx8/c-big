#include <stdio.h>
#include <wchar.h>
#include <malloc.h>
#include "entity.h"
#include "timestamp.h"
#include "werror.h"
#include "app_path.h"
#include "breakinfo.h"
int apart(const struct ENTITY *,void *,FILE *);
//int sort_part(const struct ENTITY *ent,long povit,struct break_info *);
int sort_big(const struct ENTITY *ent,void *db,const struct APP_PATH *ap)
{

    long start=now();
    if(start==0){
        return -1;
    }
    FILE *fdst=fopen(ap->preform_dst,"w");
    if(fdst == NULL){
        WERR_NO_MSG(errno,L"打开文件错误%s\n",ap->preform_dst);
        return -1;
    }
    long pivot=apart(ent,db,fdst);
    if( pivot<0){
        fclose(fdst);
        return -1;
    }
    long end=now();
    if(end==0){
        fclose(fdst);
        return -1;
    }
    char tmstr[64];
    timestamp_str(&tmstr[0],end-start);
    fclose(fdst);

    struct break_info *bi=break_info_load(NULL);
    bi->indexname=ap->dst;
    bi->time1=end-start;
    bi->time2=0;
    bi->scope_cnt=4;
    bi->scope=malloc(sizeof(long)*4);
    bi->scope[0]=1;
    bi->scope[1]=pivot;
    bi->scope[2]=pivot+1;
    bi->scope[3]= *((long *)db); // 数据库结果第一个字段保存了记录总数
    if(break_info_save(bi,ap->preform_break_info)){
        return -1;
    }
    //sort_part(ent,pivot,fdst);
    wprintf(L"用时:%s\n",tmstr);
    free(bi->scope);
    return 0;
}
