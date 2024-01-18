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