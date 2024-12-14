#include <stdio.h>
#include <wchar.h>
#include "error_stack.h"
#include "entity.h"

#ifndef NULL
#define NULL (void*)0
#endif

extern const struct Entity chinaid_entity;


const struct Entity *entities[]={&chinaid_entity};


int loadTag(const char *db,char tag[])
{
    FILE *dbf=fopen(db,"r");
    if(dbf==NULL){
        ERROR_BY_ERRNO();
        return -1;
    }
    fseek(dbf,-16,SEEK_END);
    int res=fread(tag,16,1,dbf);
    if (res != 1){
        ERROR(L"１６个字节都读不出来");
        fclose(dbf);
        return -1;
    }
    fclose(dbf);
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
    loadTag(dbname,tag);
    for(int ix=0;ix<16;ix++){
        putwchar(tag[ix]);
    }
    putwchar(L'\n');
    for(int ix=0;ix<16;ix++){
        putwchar(entities[0]->tag[ix]);
    }
    putwchar(L'\n');
    
}