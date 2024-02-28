#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <wchar.h>
#include <stdlib.h>
#include "../error_stack.h"
#include "strs.h"



struct st_nameid{
    FILE *dbh;
    long cur;
    const struct STRS **titles;
    int title_cnt;
};
int load_titles(struct st_nameid *nid,const char *buf,int size)
{
    int strscnt= calc_strss(buf,size);
    nid->titles=malloc(sizeof(struct st_nameid *)*strscnt);
    uint16_t offset=0;
    for(int ix=0;ix<strscnt;ix++){
        uint16_t len;
        const struct STRS *strs=strs_load(buf+offset,&len);
        if (strs==NULL){
            return -1;
        }
        nid->titles[ix]=strs;
        offset +=len;
    }
    if(offset != size){
        printf("%d=====%d\n",offset,size);
        ERROR("ＥＲＲＯＲ");
        return -1;
    }
    nid->title_cnt=strscnt;
    printf("有%d个标题\n",strscnt);
    return 0;
}
struct st_nameid *new_nameid(const char *dbn){

    FILE *fsrc=fopen(dbn,"r");
    if (fsrc==NULL){
        ERROR_BY_ERRNO();
        return NULL;
    }
    fseek(fsrc,-4,SEEK_END);
    uint8_t byte4[4];
    uint32_t *p32=(uint32_t *)&byte4[0];
    size_t sz=fread(&byte4[0],4,1,fsrc);
    int32_t i32=*p32;
    uint8_t *buf=malloc(i32);
    fseek(fsrc,-i32,SEEK_END);

    sz=fread(buf,1,i32,fsrc);
    if (sz != i32){
        printf("出错误了%ld,%d\n",sz,i32);
        fclose(fsrc);
        free(buf);
        return NULL;
    }

    struct st_nameid *nid=malloc(sizeof(struct st_nameid));
    if(load_titles(nid,(const char *)buf,i32-4)){
        fclose(fsrc);
        free(buf);
        return NULL;
    }
    nid->dbh=fsrc;
    nid->cur=0L;
    free(buf);
    printf("title count:%d\n",nid->title_cnt);
    return nid;
}
void close_nameid(struct st_nameid *nid){
    fclose(nid->dbh);
}
