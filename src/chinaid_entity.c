#include <stdio.h>
#include <malloc.h>
#include "entity.h"
#include "error_stack.h"
#define UNITSIZE 16

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
struct nameiddb{
    long total;
    FILE *dbh;
};
// struct nameid_tail{
//     long rows;

//     struct wstrs *titles[];
// };

void id_str(long id,wchar_t str[]);
struct nameiddb* id_index(FILE *dbf,char **buf)
{
    char b16[16];
    wchar_t id18[19];

    fseek(dbf,-16-12,SEEK_END);
    size_t cnt=fread(b16,1,12,dbf);
    if(cnt != 12){
        ERRORV(L"读不到１２个字节,只有%d",cnt);
        return NULL;
    }
    long total=*((long *)&b16[4]);
    // 申请total * 16到内存用于存放索引数据
    char *idxbuf=malloc(total * UNITSIZE);
    *buf=idxbuf;
    int end_size=*((int *)b16);
    fseek(dbf,-end_size-16,SEEK_END);
    long data_end=ftell(dbf);

    rewind(dbf);
    long incr=0;
    while(1){
        if(feof(dbf)){
            ERROR(L"unexpect eof");
            return NULL;
        }
        cnt=fread(b16,1,16,dbf);
        if(cnt != 16){
            ERRORV(L"读不到１６个字节,只有%d",cnt);
            return NULL;
        }
        long *ptr=(long *)(&b16[8]);
        id_str(*ptr,id18);
        if(incr>9726600){
            wprintf(L"%7ld:%ls",incr,id18);
            putwchar(L'\n');
        }
        ptr=(long *)b16;
        int offset = *ptr & 0x7ffff;
        fseek(dbf,offset,SEEK_CUR);
        long pos=ftell(dbf);
        incr++;
        if(pos==data_end){
            break;
        }
        if(pos > data_end){
            ERRORV(L"期望位置%ld,但是出现了%ld\n",data_end,pos);
            return NULL;
        }
    }
    wprintf(L"总共有%ld条记录,期望%ld条记录\n",incr,total);
    struct nameiddb *ndb=malloc(sizeof(struct nameiddb));
    ndb->dbh=dbf;
    ndb->total=total;
    return ndb;
}
/**
 * @brief 显示记录内容
 * @param dbf 数据库句柄，用fopen打开
 * @param row 由２个元素组成,0是身份证，1是数据在数据库中的偏移
 */
int   print (struct nameiddb* ndb,const long *row)
{
    wprintf(L"total:%ld\n",ndb->total);
    return 0;
}
const struct Entity chinaid_entity={
    (int        (*)(const void*,const void*)) id_lt,
    (int        (*)(const void*,const void*)) id_cmp,
    (struct db* (*)(void*,char **))           id_index,
    (int        (*)(struct db*,const void *)) print,
    {99,104,105,110,97,105,100,32,32,32,32,32,32,32,32,32},
    L"居民身份证查询数据库",
    UNITSIZE
};
