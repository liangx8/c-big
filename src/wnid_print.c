#include <wchar.h>
#include <malloc.h>
#include <errno.h>
#include "wnid_meta.h"
#include "wnid_strs.h"
#include "werror.h"
#include "buffer.h"
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
        dst[17]='X';
    }
    return 0;
}
extern struct BUFFER *buf_pt;
int wnid_print(struct WNAMEID *wnid,uint64_t seq)
{

    uint64_t lead[2];
    if(fread((char *)&lead[0],8,2,wnid->dbh)<2){
        WERROR(L"数据库文件不完整","");
        return -1;
    }
    uint32_t size= lead[0] & MASK_SIZE;
    size_t lsize=size;
    if(lsize > wnid->raw_max){
        void *mem=realloc(wnid->cur_raw,lsize);
        if (mem==NULL){
            WERRNO(errno);
            return -1;
        }
        wnid->cur_raw=mem;
        wnid->raw_max=lsize;
    }
    long next=wnid->cur + size+16;
    if (next > wnid->end){
        WERROR(L"数据库文件不完整","");
        return -1;
    }
    if (next == wnid->end){
        return NAMEID_EOF;
    }
    size_t rsize=fread(wnid->cur_raw,1,lsize,wnid->dbh);
    if (rsize!=lsize){
        WERROR(L"未知错误%lu:%lu,%lu",seq,rsize,lsize);
        return -1;
    }
    if((seq & 0xfff)==0){
        long dcnt=19*sizeof(wchar_t);
        long scon;
        wchar_t *wid=(wchar_t *)buf_pt->data;
        wchinaid(wid,lead[1]);
        uint64_t mobile=(lead[0] >> 20) & (MASK_MOBILE);
        wchar_t *name=(wchar_t *)(buf_pt->data+dcnt);
        uint64_t ppp=load_wstr(name,wnid->cur_raw);
        dcnt += (pa1(ppp)+1) * sizeof(wchar_t);
        scon=pa2(ppp);
        wchar_t *addr=(wchar_t *)(buf_pt->data+dcnt);
        ppp=load_wstr(addr,wnid->cur_raw + scon);

        wprintf(L"%7lu:身份证:\033[1m%ls\033[0m,手机:%ld,姓名:\033[0;34m%ls\033[0m\n地址:%ls\n",seq,wid,mobile,name,addr);
    }
    wnid->cur +=lsize + 16;
    return 1;
}