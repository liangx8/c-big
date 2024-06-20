#include <wchar.h>
#include <malloc.h>
#include <errno.h>
#include "wnid_cid_index_entity.h"
#include "wnid_meta.h"
#include "wnid_strs.h"
#include "werror.h"

const wchar_t mnls[]={0x738b,0x5c0f,0x4fca,0};
int meaningless(void *xx)
{
    wprintf(L"___________________\n");
    const wchar_t *p=mnls;
    while(*p){
        wprintf(L"%lc",*p);
        p++;
    }
    wprintf(L"\n");
    return 0;
}

/**
 * @brief 转换uint64_t成为身份证号
 * @param dst, 转换结果存放身份证号的内存空间指针,必须是非空值
 * @param id,  uint64_t身份证号存放的数字形式
 * @return alway 0
*/
int wchinaid(wchar_t *dst,uint64_t id)
{
    int isuf=id & 1;
    uint64_t real=id >> 1;
    swprintf(dst,19,L"%lu",real);
    //sprintf(dst,"%lu",real);
    if (isuf){
        dst[17]=L'X';
    }
    return 0;
}
struct BUFFER *new_buffer(int size);

struct wstrs *get_title(struct WNAMEID* wnid,int idx);
#define HEAD_SIZE 16
int wnid_print(long seq,const struct chnid_idx *idx,struct WNAMEID *wnid)
{
    wchar_t cid[19];
    wchinaid(&cid[0],idx->id);
    wprintf(L"%8ld:身份证:\033[0;33;42m%ls\033[0m",seq,&cid[0]);
    if (wnid){
        wprintf(L"偏移:%ld,",idx->pos);
        char head[HEAD_SIZE];
        uint64_t *p64=(uint64_t *)&head[0];
        fseek(wnid->dbh,idx->pos,SEEK_SET);
        if(fread(&head[0],1,HEAD_SIZE,wnid->dbh)!= 16){
            WERROR(L"读数据错误","");
            return -1;
        }
        uint64_t u64=*(p64+1);
        if(idx->id != u64){
            wprintf(L"\n");
            WERROR(L"索引记录错误\n idx:%16lx\nbase:%16lx\n",idx->id,u64);
            return -1;
        }
        u64=*p64;
        uint32_t usize=MASK_SIZE & u64;
        int title_idx=(u64 >> 55) & MASK_TITLES;
        wprintf(L",大小:%d,title:%d\n",usize,title_idx);
        struct wstrs *title=get_title(wnid,title_idx);
        
        struct BUFFER *buf=new_buffer(usize);
        char *cb=(char *)&buf->data[0];
        if(fread(cb,1,usize,wnid->dbh)!= usize){
            WERROR(L"读数据错误","");
            free(buf);
            return -1;
        }
        int init=buf->idx;
        int dstcnt=0;
        int posname=load_wstr(NULL,buf);
        int posaddr=load_wstr(NULL,buf);
        if(posname>0){
            dstcnt +=sizeof(wchar_t);
            
        }
        if(posaddr>0){
            dstcnt +=sizeof(wchar_t);
        }
        dstcnt+=posname+posaddr;
        //CP_MSG(L"namecnt:%d,addrcnt:%d,total:%d,%d,%d\n",posname,posaddr,dstcnt,init,buf->idx);
        buf->idx=init;
        wchar_t *name;
        wchar_t *addr;
        char *base=malloc(dstcnt);
        char *p8=base;
        if(posname){
            name=(wchar_t *)base;
            load_wstr(name,buf);
            wprintf(L"姓名: %ls",name);
            p8=base + posname+sizeof(wchar_t);
        } else {
            buf->idx++;
            name=NULL;
        }
        if(posaddr){
            addr=(wchar_t *)p8;
            load_wstr(addr,buf);
            wprintf(L" 地址: %ls\n",addr);
        } else {
            addr=NULL;
            buf->idx++;
            wprintf(L"\n");
        }
        free(base);
        //CP_MSG(L"title idx:%d,size:%d,buf->idx:%d/%d\n",title_idx,usize,buf->idx,buf->size);
        //buf->idx=8;
        struct wstrs *wss=load_wstrs(buf);
        if(wss==NULL){
            free(buf);
            return -1;
        }
        //CP_MSG(L"有%d个其他\n",wss->cnt);
        for(int ix=0;ix<wss->cnt;ix++){
            wprintf(L"%ls:%ls\n",title->str[ix],wss->str[ix]);
        }
        

        free(buf);
        free(wss);
        return 0;
    } else {
        wprintf(L",偏移:%10u\n",idx->pos);
    }
    return 0;
}