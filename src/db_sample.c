#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <stdlib.h> // getenv()
#include <string.h>
#include "abstract_db.h"
#include "sort_range.h"
#include "log.h"
#include "random_gen.h"

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
int smp_print(const struct ABSTRACT_DB *db,uint64_t seq,int flag)
{
    uint64_t *ptr=(uint64_t*)(db->raw+seq * 12);
    unsigned int *iptr=(unsigned int *)(db->raw+seq * 12 +8);
    if(flag==0){
        wprintf(L"\033[0;36m");
    }
    wprintf(L"%8ld:0x%016lx,%8d\033[0m\n",seq,*ptr,*iptr);
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
uint64_t smp_id(const struct ABSTRACT_DB *db,uint64_t seq)
{
    uint64_t *ptr= (unsigned long*)(db->raw+seq * 12);
    return *ptr;
}
const struct ENTITY smp_entity={
    (CMP)                                ulong_lt,
    (CMP)                                long_cmp,
    (int (*)(const void *,uint64_t,int)) smp_print,
    (int (*)(void *))                    sm_close,
    (uint64_t (*)(const void*,uint64_t)) smp_id,
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
    random_init();
    FILE *org=fopen("/home/ram/big/sample.bin","w+");
    for(uint64_t ix=0;ix<size;ix++){
        uint64_t *p64=(uint64_t*)pb;
        uint32_t *p32=(uint32_t*)(pb+8);
        *p64=random_long();
        *p32=ix;
        pb +=12;
        fwrite(p64,1,8,org);
    }
    fclose(org);
    random_close();
    log_info(L"总共生成%lu条记录\n",size);
    total_size=size;

    return base;
}
void dbname(char *buf,const char *file)
{
    const char *home=getenv("HOME");
    int namesize=strlen(home);
    memset(buf,0,128);
    strcpy(buf,home);
    buf[namesize]='/';
    strcat(buf,dbdir);
    buf[namesize+4]='/';
    strcat(buf,file);
}
void sm_search(uint64_t val)
{
    dbname(buf,dbfile);
    off_t size=filesize(buf);
    uint64_t end=size/smp_entity.unitsize;
    const uint64_t total=end;
    uint64_t start=0;
    FILE* fdb=fopen(buf,"r");
    if(fdb==NULL){
        log_err(L"打开数据库错误%s\n",buf);
        return;
    }
    char data[12*10];
    uint64_t *ptr=(uint64_t*)&data[0];
    uint64_t mid=0;
    while(1){
        if(start+1==end){
            break;
        }
        mid=(start+end)/2;
        fseek(fdb,mid*smp_entity.unitsize,SEEK_SET);
        fread(data,12,1,fdb);
        if(*ptr > val){
            end=mid;
        }else{
            start=mid;
        }
    }
    if(total-mid<5){
        start=total-10;
    }else {
        start=mid>=5?mid-5:0;
    }
    fseek(fdb,start*12,SEEK_SET);
    fread(data,12,10,fdb);
    for(int ix=0;ix<10;ix++){
        uint64_t *ptr=(uint64_t*)&data[ix *12];
        uint32_t *iptr=(uint32_t*)&data[ix *12+8];
        if(ix+start==mid){
            wprintf(L"\033[0;33m%8ld:0x%016lx,%11u\033[0m\n",ix+start,*ptr,*iptr);
        } else{
            wprintf(L"%8ld:0x%016lx,%11u\033[0m\n",ix+start,*ptr,*iptr);
        }
    }
    fclose(fdb);

}