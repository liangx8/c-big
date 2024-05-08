#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include "wnid_meta.h"
#include "entity.h"
#include "werror.h"

int chnid_cmp(const uint64_t *p1,const uint64_t *p2)
{
    if(*p1 > *p2){
        return 1;
    }
    if(*p1 == *p2){
        return 0;
    }
    return -1;
}
int chnid_lt(const uint64_t *p1,const uint64_t *p2)
{
    return *p1 < *p2;
}
int chnid_print(const uint64_t *idx,struct WNAMEID *wnid){
    return 0;
}
int wnid_next(const void *,char *);
const struct ENTITY chnid_index_entity={
    (int (*)(const char *,const char *))chnid_cmp,
    (int (*)(const char *,const char *))chnid_lt,
    (int (*)(const char *,const void *))chnid_print,
    wnid_next,
    16
};
const char *id_idx_file="/home/com/big-data/chinaid/id.idx";
int _index_init(struct WNAMEID *wnid,const char *idxname)
{
    // 思路还未清晰，转到排序设计
    FILE *idx;
    const char *filename;
    if(idxname==NULL){
        filename=id_idx_file;
    } else {
        filename=idxname;
    }
    idx=fopen(filename,"w");
    if(idx == NULL){
        WERR_NO_MSG(errno,L"建立文件错误%s\n",filename);
        return -1;
    }
    return 0;
}