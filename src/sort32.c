#include <stdio.h>
#include <malloc.h>
//#include <pthread.h>
#include "status.h"
#include "error_stack.h"
#include "bag.h"
#include "run_status.h"


int apart32(const char *,const char *,struct run_status *);

int sort32(struct STATUS *stu)
{
    struct run_status rs;
    if (stu->step1time == 0)
    {
        if (apart32(stu->src,stu->dst,&rs) < 0)
        {
            goto error_return;
        }
    }
    stu->total=rs.size;
    return 0;
error_return:
    return -1;
}

/*
//pthread_cond_wait() 使用的方法。

一个调度进程(由主进程担任) master, 不会占用太多CPU,提供一个 (struct pthread_mutex_t *)mutex,空闲时调用pthread_cond_wait() 释放mutex

多个任务进程(pthread_create建立)task，任务完成后需要传输结果给master,先获取mutex,成功以后就存放数据到指定位置。然后pthread_cond_signal()
恢复master的运行.

有2个 pthread_cond_t, 一个是master用， 另外一个由多个task共用。

*/