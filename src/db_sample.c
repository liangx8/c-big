#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <stdlib.h> // getenv()
#include <string.h>
#include "abstract_db.h"
#include "sort_range.h"

static char buf[128];
const char *dbfile="sample.db";
const char *dbdir="big";


int touch_dir(const char *dirname);
FILE *create_or_open_file(const char *dbname);
off_t filesize(const char *path);
long signed_rand(void);
#define DEV_SIZE 0

void generate_db(size_t size,void (*progress)(int))
{
    const char *home=getenv("HOME");
    int namesize=strlen(home);
    char tbuf[12];
    uint64_t *p64=(uint64_t*)tbuf;
    uint32_t *p32=(uint32_t*)&tbuf[8];
    memset(buf,0,128);
    strcpy(buf,home);
    buf[namesize]='/';
    strcat(buf,dbdir);
    if(touch_dir(buf)){
        return;
    }

    buf[namesize+4]='/';
    strcat(buf,dbfile);
    wprintf(L"HOME:%s\n",buf);

    FILE *db=create_or_open_file(buf);
    if(db==NULL){
        return;
    }
    const uint64_t dd=size/100;
    int pgr_cnt=0;
    uint64_t cnt=0;
    for(uint64_t ix=0;ix<size;ix++){
        if(progress){

            if(++cnt==dd){
                cnt=0;
                progress(++pgr_cnt);
            }
        }
        *p64=signed_rand();
        *p32=ix;
        fwrite(&tbuf[0],12,1,db);
    }
    wprintf(L"总共生成%lu条记录\n",size);
    fclose(db);
}
long ulong_lt(const void*,const void*);
long long_cmp(const void*,const void*);
int smp_print(const struct ABSTRACT_DB *db,long seq)
{
    long *ptr=(long*)(db->raw+seq * 12);
    unsigned int *iptr=(unsigned int *)(db->raw+seq * 12 +8);
    wprintf(L"%8ld:0x%016lx,%8d\n",seq,*ptr,*iptr);
    return 0;
}
int rd_close(struct ABSTRACT_DB *db); // db_random_mem.c
unsigned long smp_id(const struct ABSTRACT_DB *db,long seq)
{
    unsigned long *ptr= (unsigned long*)(db->raw+seq * 12);
    return *ptr;
}
const struct ENTITY smp_entity={
    (CMP)                                       ulong_lt,
    (CMP)                                       long_cmp,
    (int (*)(const void *,long))                smp_print,
    (int (*)(void *))                           rd_close,
    (unsigned long (*)(const void*db,long seq)) smp_id,
    12
};
struct ABSTRACT_DB* sample_db(void)
{
    const char *home=getenv("HOME");
    int namesize=strlen(home);
    memset(buf,0,128);
    strcpy(buf,home);
    buf[namesize]='/';
    strcat(buf,dbdir);
    buf[namesize+4]='/';
    strcat(buf,dbfile);
    off_t size= filesize(buf);
#if DEV_SIZE
    // 开发期间限制用比较小的内容
    if(size > DEV_SIZE*12){
        size=DEV_SIZE*12;
    }
#endif
    void *base=malloc(sizeof(struct ABSTRACT_DB)+size);
    if(base == NULL){
        wprintf(L"内存不足\n");
        return NULL;
    }
    uint64_t max=size/12;
    struct ABSTRACT_DB *db=base;
    struct RANGES *scops = rng_new();
    rng_push(scops,0L,max);
    db->scops=scops;
    db->entity=&smp_entity;
    db->raw=base+sizeof(struct ABSTRACT_DB);
    FILE *fdb=fopen(buf,"r");
    if(fdb==NULL){
        free(base);
        wprintf(L"打开数据库%s出错\n",buf);
        return NULL;
    }
    size_t cnt=fread(db->raw,12,max,fdb);
    if(max != cnt){
        free(base);
        fclose(fdb);
        wprintf(L"读取数据库%s出错\n",buf);
        return NULL;
    }
    fclose(fdb);
    log_info(L"几大:%ld    %ld\n",size,sizeof(off_t));
    return base;
}