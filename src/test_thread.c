#include <pthread.h>
#include <stdio.h>
#include <wchar.h>
#include <unistd.h>
#include <malloc.h>

#include "action_const.h"

const struct timespec MS1   = {0, 100000000};
const struct timespec NS100 = {0, 1000000};
const struct timespec S1    = {1, 0};



struct run_status{
    pthread_mutex_t *mut;
    pthread_cond_t *cond;
    int *counter;
    int tid;
    int run;
};
void *test_task(void *pl)
{
    struct run_status *rs=(struct run_status*)pl;
    pthread_mutex_lock(rs->mut);
    wprintf(L"任务%2d 锁定\n",rs->tid);
    while(rs->run){
        int cntr;
        wprintf(L"任务%2d 等待\n",rs->tid);
        pthread_cond_wait(rs->cond,rs->mut);
        cntr=*rs->counter;
        *(rs->counter)=cntr+1;
        wprintf(L"任务 %2d: %d\n",rs->tid,cntr);
    }
    pthread_mutex_unlock(rs->mut);
    wprintf(L"任务 %2d 结束\n",rs->tid);
    return NULL;
}

extern int cpunum;
extern int action;
int test_thread(const void *x)
{
    pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
    struct run_status *rss;
    pthread_t pid[cpunum];
    rss=(struct run_status *)malloc(cpunum * sizeof(struct run_status));
    int counter=0;
    for (int ix=0;ix<cpunum;ix++){
        struct run_status *rs=rss+ix;
        rs->tid=ix;
        rs->mut=&mut;
        rs->cond=&cond;
        rs->counter =&counter;
        rs->run=1;
        pthread_create(&pid[ix],NULL,test_task,rs);
    }
    int seq=0;
    while(1){
        pause();
        wprintf(L"[%3d]当前值: %d action:%d\n",seq,counter,action);
        pthread_mutex_lock(&mut);
        pthread_cond_signal(&cond);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mut);
        seq++;
        if(counter>10){
            break;
        }
    }
    wprintf(L"结束全部任务\n");
    for(int ix=0;ix<cpunum;ix++){
        struct run_status *rs=rss+ix;
        rs->run=0;
    }
    pthread_cond_broadcast(&cond);
    wprintf(L"广播完成\n");
    for(int ix=0;ix<cpunum;ix++){
        void *ret;
        pthread_join(pid[ix],&ret);
        if(ret){
            wprintf(L"something wrong at %d\n",ix);
        }
    }
    free(rss);
    wprintf(L"counter: %d\n",counter);
    return 0;
}