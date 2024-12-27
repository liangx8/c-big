#include <stdio.h>
#include <malloc.h>
#include <wchar.h>
#include "memdb.h"
#include "error_stack.h"
#include "sort_range.h"
#define UNITSIZE 16

struct _db_payload{
    const char *fname;
    char *buf;
    long total;
};

int id_lt(const unsigned long *l,const unsigned long *r)
{
    return *l < *r;
}
int id_cmp(const unsigned long *l,const unsigned long *r)
{
    unsigned long lv=*l;
    unsigned long rv=*r;
    if(lv<rv) return -1;
    if(lv>rv) return 1;
    return 0;
}
void id_str(long id,wchar_t str[]);
/**
 * @brief 显示记录内容
 * @param pl 包含创建到内存，大小等信息
 * @param row 由２个元素组成,0是身份证，1是数据在数据库中的偏移
 */
int id_print (struct _db_payload *pl,const long *row,long seq)
{
    wchar_t id18[19];
    id_str(*row,id18);
    wprintf(L"%ld:%ls,%ld\n",seq,id18,*(row+1));
    return 0;
}
int id_close(struct _db_payload *pl)
{
    wprintf(L"关闭数据库%s\n",pl->fname);
    free(pl->buf);
    free(pl);
    return 0;
}
const struct ENTITY chinaid_entity={
    (int (*)(const void*,const void*))  id_lt,
    (int (*)(const void*,const void*))  id_cmp,
    (int (*)(void *,const void *,long)) id_print,
    (int (*)(void *))                   id_close,
    //L"居民身份证查询数据库",
    UNITSIZE
};

struct MEMDB* _loaddb(FILE *dbf)
{
    char b16[16];

    fseek(dbf,-16-12,SEEK_END);
    size_t cnt=fread(b16,1,12,dbf);
    if(cnt != 12){
        ERRORV(L"读不到１２个字节,只有%d",cnt);
        return NULL;
    }
    long total=*((long *)&b16[4]);
    // 申请total * 16到内存用于存放索引数据
    char *idxbuf=malloc(total * UNITSIZE);
    

    int end_size=*((int *)b16);
    fseek(dbf,-end_size-16,SEEK_END);
    long data_end=ftell(dbf);

    rewind(dbf);
    long incr=0;
    while(1){
        long *longptr;
        if(feof(dbf)){
            ERROR(L"unexpect eof");
            return NULL;
        }
        cnt=fread(b16,1,16,dbf);
        if(cnt != 16){
            ERRORV(L"读不到１６个字节,只有%d",cnt);
            return NULL;
        }
        longptr=(long *)(&b16[8]);
        long id=*longptr;
        longptr=(long *)b16;
        int offset = *longptr & 0x7ffff;
        fseek(dbf,offset,SEEK_CUR);
        long pos=ftell(dbf);
        longptr=(long*)(idxbuf + (incr * 16));
        *longptr=id;
        *(longptr+1)=pos;
        incr++;
        if(pos==data_end){
            break;
        }
        if(pos > data_end){
            ERRORV(L"期望位置%ld,但是出现了%ld\n",data_end,pos);
            free(idxbuf);
            return NULL;
        }
    }
    wprintf(L"总共有%ld条记录,期望%ld条记录\n",incr,total);
    struct _db_payload *pl=malloc(sizeof(struct MEMDB)+sizeof(struct _db_payload));
    struct MEMDB *ndb=((void*)pl) + sizeof(struct _db_payload);
    //CP_MSG(L"%x,%x\n",(long)pl,(long)ndb);
    pl->buf=idxbuf;
    pl->total=total;

    ndb->raw=idxbuf;
    ndb->entity=&chinaid_entity;
    ndb->payload=pl;
    return ndb;
}
const struct MEMDB* loaddb(const char *fname)
{
    FILE *f=fopen(fname,"r");
    if(f ==NULL){
        ERROR_BY_ERRNO();
        return NULL;
    }
    struct MEMDB *db=_loaddb(f);
    if(db==NULL){
        fclose(f);
        return NULL;
    }
    struct _db_payload *pl=db->payload;
    pl->fname=fname;

    db->scops=rng_new();
    rng_push(db->scops,0,pl->total);
    return db;
}
