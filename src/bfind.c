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
off_t bfind(const char *dst,const void *val,const struct ENTITY *ent)
{
    char buf[ent->unitsize];
    long size=filesize(dst);
    long right = size / ent->unitsize;
    FILE *fdst=fopen(dst,"r");
    long left=0;
    long op=(right-left)/2;
    while(1){
        fseek(fdst,op * ent->unitsize,SEEK_SET);
        if(fread(&buf[0],ent->unitsize,1,fdst)<1){
            return -1;
        }
        if((ent->cmp(val,buf))<0){
            right=op;
            op=(left + right)/2;
            continue;
        }
        if(ent->cmp(val,buf)==0){
            return op;
        }
        // greater then
        left=op;
        op=(left + right)/2;

    }
}