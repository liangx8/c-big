#include <stdio.h>
#include <wchar.h>
#include <malloc.h>
#include "error_stack.h"
#include "entity.h"



extern const struct Entity chinaid_entity;


const struct Entity *entities[]={&chinaid_entity};


int loadTag(FILE *dbf,char tag[])
{
    fseek(dbf,-16,SEEK_END);
    int res=fread(tag,16,1,dbf);
    if (res != 1){
        ERROR(L"１６个字节都读不出来");
        return -1;
    }
    return 0;
}


/**
 * @brief sort hurge amount of data in memory
 * @param cfgname 包含有多少单元要排序，目标文件名
 * @param cmp compare function
 */
void bigsort(const char *dbname)
{
    char tag[16];
    FILE *dbf=fopen(dbname,"r");
    if(dbf==NULL){
        ERROR_BY_ERRNO();
        return;
    }
    if(loadTag(dbf,tag)){
        fclose(dbf);
        return;
    }
    for(int ix=0;ix<1;ix++){
        int same=1;
        for(int iy=0;iy<16;iy++){
            if(entities[ix]->tag[iy]!=tag[iy]){
                same=0;
                break;
            }
        }
        if(same){
            char *buf;
            wprintf(entities[ix]->remark);
            putwchar(L'\n');
            struct db *ndb=entities[ix]->index(dbf,&buf);
            entities[ix]->print(ndb,NULL);
            wprintf(L"释放内存\n");
            free(buf);
        }
    }
    
    fclose(dbf);
}