#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <pthread.h>
#include "status.h"
#include "entity.h"
#include "error_stack.h"


#define BUF_SIZE  1024 * 200
#define UNIT_SIZE 12
// 多没有排序的文件查找
struct seq_find_data{
    pthread_mutex_t *mutex;
    FILE            *fh;
    const struct ENTITY *ent;
    const void       *obj;
    int              finding;

} sfd;
void *find_task(void *obj)
{

    return NULL;
}
extern int cpunum;
void seq_find(const struct STATUS *stu,uint32_t val,int limit)
{
    sfd.ent=stu->payload;
}
off_t filesize(const char *);
/**
 * @brief 快速查找，dst的内容必须是递增排序，ent->vallt(v,idx)必须是 v <= ary[idx]
 * 查找第一个符合vallt返回真值的索引
*/
off_t bfind(const char *dst,uint64_t ival,const struct ENTITY *ent)
{
    char buf[ent->unitsize];
    long size=filesize(dst);
    long right = size / ent->unitsize;
    FILE *fdst=fopen(dst,"r");
    long left=0;
    long op;
    if(fdst==NULL){
        ERROR_BY_ERRNO();
        return -1;
    }

    while(1){
        if(right==left){
            fclose(fdst);
            return right;
        }
        op=(left+right)/2;
        fseek(fdst,op * ent->unitsize,SEEK_SET);
        if(fread(&buf[0],ent->unitsize,1,fdst)<1){
            fclose(fdst);
            ERROR("INTERNAL ERROR");
            return -1;
        }
        if(ent->vallt(ival,buf)){
            right=op;
        } else {
            left=op;
        }
        if(left+1==right){
            return right;
        }
    }
}