#include <string.h>
#include <malloc.h>
#include <errno.h>
#include "werror.h"
#include "wnid_meta.h"



off_t filesize(const char *path);

// 文件结尾的12个字节: 4字节记录titles的储存大小，8字节记录总共的记录数
// titles记录包括12字节
#define MAGIC_NUM_WIDTH 12

struct WNAMEID* new_wnameid(const char *fname)
{
    off_t size=filesize(fname);
    if(size<0){
        wprintf(L"错误文件:%s(%s)\n",fname,strerror(errno));
        return NULL;
    } else if (size < MAGIC_NUM_WIDTH){
        wprintf(L"文件错误:%ld\n",size);
        return NULL;
    }
    FILE *fsrc=fopen(fname,"r");
    if (fsrc==NULL){
        wprintf(L"打开文件%s(%s)\n",fname,strerror(errno));
        return NULL;
    }

    fseek(fsrc,-MAGIC_NUM_WIDTH,SEEK_END);
    uint8_t magic[MAGIC_NUM_WIDTH];
    uint32_t *p32=(uint32_t *)&magic[0];
    size_t sz=fread(&magic[0],MAGIC_NUM_WIDTH,1,fsrc);
    if (sz != 1){
        wprintf(L"期望读取12字节失败:%ld\n",sz);
        fclose(fsrc);
        return NULL;
    }
    uint32_t end_size=*p32;
    if(end_size<MAGIC_NUM_WIDTH){
        wprintf(L"读标题数据错误\n");
        fclose(fsrc);
        return NULL;
    }
    struct WNAMEID *wnid=malloc(sizeof(struct WNAMEID));
    if(end_size>MAGIC_NUM_WIDTH){
        size_t title_size=end_size - MAGIC_NUM_WIDTH;
        if(title_size){
            // stuff of loading titles
        } else {
            wnid->subtitles=NULL;
        }
    }

    rewind(fsrc);
    wnid->end=size-end_size;
    wnid->dbh=fsrc;
    wnid->cur=0L;
    wnid->total = *((uint64_t*) (&magic[4]));
    //If size is 0, then
    //   malloc() returns a unique pointer value that can later be successfully
    //   passed to free(). 
    wnid->cur_raw=malloc(0);
    wnid->raw_max=0;
    //CP_MSG(L"发现%lu条记录\n",wnid->total);
    wprintf(L"发现 \033[1m%lu\033[0m 条记录\n",wnid->total);
    return wnid;
}
int wnid_close(struct WNAMEID *wnid)
{
    if(fclose(wnid->dbh)){
        wprintf(L"关闭文件错误：%s\n",strerror(errno));
        return -1;
    }
    free(wnid->cur_raw);
    free(wnid);
    return 0;
}
/**
 * @brief 单指向下一个
*/
int wnid_next(struct WNAMEID *wnid,uint64_t *lead)
{
    fseek(wnid->dbh,wnid->cur,SEEK_SET);
    if(fread((char *)lead,8,2,wnid->dbh)<2){
        WERROR(L"数据库文件不完整","");
        return -1;
    }
    uint32_t size= lead[0] & MASK_SIZE;
    long next=wnid->cur + size+16;
    if (next > wnid->end){
        WERROR(L"数据库文件不完整","");
        return -1;
    }
    if (next == wnid->end){
        return NAMEID_EOF;
    }
    wnid->cur=next;
    return 1;
}

