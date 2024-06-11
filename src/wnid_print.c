#include <wchar.h>
#include <malloc.h>
#include <errno.h>
#include "wnid_cid_index_entity.h"
#include "wnid_meta.h"
#include "wnid_strs.h"
#include "werror.h"
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

const void *nid_get(const struct WNAMEID *nid,struct chnid_idx *idx);
int nid_print(const void *);

int wnid_print(long seq,const struct chnid_idx *idx,struct WNAMEID *wnid)
{


    wchar_t cid[19];
    wchinaid(&cid[0],idx->id);
    wprintf(L"%8ld:身份证:%ls",seq,&cid[0]);
    if (wnid){
        fseek(wnid->dbh,idx->pos,SEEK_SET);
        wprintf(L"\n");
        return 0;
    } else {
        wprintf(L",偏移:%10u\n",idx->pos);
    }
    return 0;
}