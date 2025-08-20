#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <stdlib.h> // getenv()
#include <string.h>
#include "abstract_db.h"
#include "sort_range.h"
#include "log.h"

static char buf[128];
const char *dbfile="sample.db";
const char *dbdir="big";


int touch_dir(const char *dirname);
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

    FILE *db=fopen(buf,"w+");
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
uint64_t total_size;
int sm_close(struct ABSTRACT_DB *db)
{
    const char *home=getenv("HOME");
    int namesize=strlen(home);
    memset(buf,0,128);
    strcpy(buf,home);
    buf[namesize]='/';
    strcat(buf,dbdir);
    if(touch_dir(buf)){
        log_err(L"建立目录%s错误\n",buf);
        rng_release(db->scops);
        free(db);
        return -1;
    }

    buf[namesize+4]='/';
    strcat(buf,dbfile);
    FILE* fdb=fopen(buf,"w+");
    if(fdb==NULL){
        rng_release(db->scops);
        free(db);
        log_err(L"打开文件错误%s\n",buf);
        return -1;
    }
    size_t res=fwrite(db->raw,12,total_size,fdb);
    if(res != total_size){
        log_err(L"期望保存%ld条记录，实际保存%ld\n",total_size,res);
    }else {
        log_info(L"生成排序数据库%ld条记录\n",total_size);
    }
    fclose(fdb);
    return 0;
    
}
unsigned long smp_id(const struct ABSTRACT_DB *db,long seq)
{
    unsigned long *ptr= (unsigned long*)(db->raw+seq * 12);
    return *ptr;
}
const struct ENTITY smp_entity={
    (CMP)                                       ulong_lt,
    (CMP)                                       long_cmp,
    (int (*)(const void *,long))                smp_print,
    (int (*)(void *))                           sm_close,
    (unsigned long (*)(const void*db,long seq)) smp_id,
    12
};
struct ABSTRACT_DB* sample_db(uint64_t size)
{
    void *base=malloc(sizeof(struct ABSTRACT_DB)+size*12);
    if(base == NULL){
        wprintf(L"内存不足\n");
        return NULL;
    }
    struct ABSTRACT_DB *db=base;
    struct RANGES *scops = rng_new();
    rng_push(scops,0L,size);
    db->scops=scops;
    db->entity=&smp_entity;
    db->raw=base+sizeof(struct ABSTRACT_DB);
    char *pb=db->raw;
    for(uint64_t ix=0;ix<size;ix++){
        uint64_t *p64=(uint64_t*)pb;
        uint32_t *p32=(uint32_t*)(pb+8);
        *p64=signed_rand();
        *p32=ix;
        pb +=12;
    }
    log_info(L"总共生成%lu条记录\n",size);
    total_size=size;

    return base;
}