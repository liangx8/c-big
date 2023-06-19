#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include "status.h"
#include "error_stack.h"
#include "bag.h"

#define MLOCK(mut)   if(pthread_mutex_lock(mut)) {ERROR("线程锁错误\n");return NULL;}
#define MUNLOCK(mut) if(pthread_mutex_unlock(mut)) {ERROR("线程解锁错误\n");return NULL;}

struct run_data{
    struct Bag *jobs;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    const char *fname;
    int done;
}sort_data;
extern int int_act; // define in main.c
extern int numcpu;

int64_t sort32_partition(FILE *fh,int64_t *scope){
    return 0;
}
void *sort32_task(void *obj)
{
    long id=(long)obj;
    struct run_data *sd=&sort_data;
    while(1){
        MLOCK(sd->mutex)
        MUNLOCK(sd->mutex)
    }
    return NULL;
}
int full_path(char *,const char *);
int sort32(struct STATUS *sta)
{
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
    sort_data.cond=&cond;
    sort_data.mutex=&mutex;
    sort_data.done=0;
    char *fn=malloc(256);
    full_path(fn,sta->dst);
    sort_data.fname=fn;
    sort_data.jobs=bag_with_array(sta->scope,sta->scope_cnt);

    bag_print(sort_data.jobs,stdout,10);

    return -1;
}

/*
暂时的想法
master线程是空闲的因此会进入等待

slave不会空闲，只能是阻塞等待
mutex 需要2个(可能只需要一个)，cond 需要一个，在master 中进入 wait等待slave 来唤醒

***pthread_cond_wait() 使用的方法。***

一个调度进程(由主进程担任) master, 不会占用太多CPU,提供一个 (struct pthread_mutex_t *)mutex,空闲时调用pthread_cond_wait() 释放mutex

多个任务进程(pthread_create建立)task，任务完成后需要传输结果给master,先获取mutex,成功以后就存放数据到指定位置。然后pthread_cond_signal()
恢复master的运行.
*/