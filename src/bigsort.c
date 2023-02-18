#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include "status.h"
#include "error_stack.h"
#include "bag.h"

#define ITEM_SIZE 100000
struct BAG *jobs;

int64_t apart(const char *src,const char *dst){
    FILE *fsrc,*fdst;
    fsrc=fopen(src,"r");
    char *buf;
    if(fsrc==NULL){
        ERROR_BY_ERRNO();
        return -1;
    }
    fdst=fopen(dst,"w");
    if (fdst == NULL){
        ERROR_BY_ERRNO();
        fclose(fsrc);
        return -1;
    }
    buf=malloc(ITEM_SIZE * 12);
    free(buf);
    return 0;
}
int bigsort(struct STATUS *stu){
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t ma=PTHREAD_COND_INITIALIZER;
    pthread_cond_t sa=PTHREAD_COND_INITIALIZER;
    jobs=bag_create();
    if(jobs==NULL){
        return -1;
    }
    if (stu->step1time==0){
        apart(stu->src,stu->dst);
    }
    bag_free(jobs);
    return 0;
}

/*
//pthread_cond_wait() 使用的方法。

一个调度进程(由主进程担任) master, 不会占用太多CPU,提供一个 (struct pthread_mutex_t *)mutex,空闲时调用pthread_cond_wait() 释放mutex

多个任务进程(pthread_create建立)task，任务完成后需要传输结果给master,先获取mutex,成功以后就存放数据到指定位置。然后pthread_cond_signal()
恢复master的运行.

有2个 pthread_cond_t, 一个是master用， 另外一个由多个task共用。

*/