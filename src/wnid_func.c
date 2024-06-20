#include <string.h>
#include <malloc.h>
#include <errno.h>
#include "wnid_strs.h"
#include "werror.h"
#include "wnid_meta.h"
#include "wnid_cid_index_entity.h"

struct titles{
    long count;
    struct wstrs *titles[];
};

struct BUFFER *new_buffer(int size);


off_t filesize(const char *path);

// 文件结尾的12个字节: 4字节记录titles的储存大小，8字节记录总共的记录数
// titles记录包括12字节
#define MAGIC_NUM_WIDTH 12

int calc_strss(const char *buf,uint32_t size);
struct wstrs *get_title(struct WNAMEID* wnid,int idx)
{
    struct titles *ttls=(struct titles*)wnid->subtitles;
    return ttls->titles[idx];
}
struct titles* load_titles(FILE *fsrc,long offset){

    if(fseek(fsrc,-offset,SEEK_END)){
        WERRNO(errno);
        return NULL;
    }
    long size=offset-MAGIC_NUM_WIDTH;
    
    struct BUFFER *src=new_buffer(size);
    
    long rsize=fread((char *)&src->data[0],1,size,fsrc);
    if(rsize< size){
        WERROR(L"期望读取%ld字节，实际只有%ld",size,rsize);
        return NULL;
    }
    int title_cnt=calc_strss(&src->data[0],size);
    if(title_cnt<0){
        return NULL;
    } else {
        CP_MSG(L"有%d组标题\n",title_cnt);
    }
    struct titles *ttls=malloc(8 + title_cnt * 8);
    ttls->count=title_cnt;
    for(int ix=0;ix<title_cnt;ix++){
        struct wstrs *wss=load_wstrs(src);
        if(wss){
            ttls->titles[ix]=wss;
        }
    }
    
    free(src);
    return ttls;
}


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
        wnid->subtitles=load_titles(fsrc,end_size);
        if(wnid->subtitles==NULL){
            return NULL;
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
    CP_MSG(L"发现%lu条记录\n",wnid->total);
    return wnid;
}
int wnid_close(struct WNAMEID *wnid)
{
    if(fclose(wnid->dbh)){
        wprintf(L"关闭文件错误：%s\n",strerror(errno));
        return -1;
    }
    struct titles *ttl= wnid->subtitles;
    for(int ix=0;ix<ttl->count;ix++){
        free(ttl->titles[ix]);
    }
    free(ttl);
    free(wnid);
    return 0;
}
struct name_idx;

struct name_idx * wnid_next_name(struct WNAMEID *wnid){
    // not implements
    return NULL;
}
struct chnid_idx _idx_data;
/**
 * @brief 单指向下一个
*/
struct chnid_idx* wnid_next_chnid(struct WNAMEID *wnid)
{
    if(wnid->end==wnid->cur){
        return NULL;
    }
    fseek(wnid->dbh,wnid->cur,SEEK_SET);
    uint64_t lead[2];
    if(fread((char *)&lead[0],8,2,wnid->dbh)<2){
        WERROR(L"数据库文件不完整","");
        return NULL;
    }
    uint32_t size= lead[0] & MASK_SIZE;
    long next=wnid->cur + size+16;
    if (next > wnid->end){
        WERROR(L"数据库文件不完整","");
        return NULL;
    }
    _idx_data.id=lead[1];
    if(wnid->cur & 0xffffffff00000000){
        WERROR(L"文件大于2^32个字节(%ls)",wnid->cur);
        return NULL;
    }
    _idx_data.pos=wnid->cur;
    wnid->cur=next;
    return &_idx_data;
}
int wnid_status(struct WNAMEID *wnid){
    if(wnid->end==wnid->cur){
        return NAMEID_EOF;
    }
    return 0;
}

