#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "status.h"
#include "error_stack.h"
#include "bag.h"

typedef int (*itcmp)(const void *,const void *);

#ifdef NDEBUG
#define PRE(m) while(1==2)
#else
#define PRE(fmt,args...) printf("%s(%3d):" fmt,__FILE__,__LINE__,args)
#endif
#define CACHE_AMOUNT 524288
#define MLOCK(mut)      if(pthread_mutex_lock(mut)) {ERROR("线程锁错误\n");int_act=SORTING_BREAK;goto finish;}
#define MUNLOCK(mut)    if(pthread_mutex_unlock(mut)) {ERROR("线程解锁错误\n");int_act=SORTING_BREAK;goto finish;}
#define MWAIT(cnd,mut)  if(pthread_cond_wait(cnd,mut)){ERROR("ptread_cond_wait() 错误\n");int_act=SORTING_BREAK;goto finish;}
#define MBOARDCAST(cnd) if(pthread_cond_broadcast(cnd)){ERROR("pthread_cond_broadcast() 错误\n");int_act=SORTING_BREAK;goto finish;}
#define MSIGNAL(cnd)    if(pthread_cond_signal(cnd)){ERROR("pthread_cond_signal() 错误\n");int_act=SORTING_BREAK;goto finish;}

#define OBJSIZE 12

struct run_data{
    struct Bag *jobs;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    const char *fname;
    // 处于活动状态的工作的数量
    int64_t *working;
}sort_data;
extern int cpunum;


void detail(void)
{
    struct run_data *sd=&sort_data;
    printf("-------------------------------------------\n");
    bag_print(sd->jobs,stdout,4);
    for(int ix=0;ix<cpunum;ix++){
        printf("id:%2d %10ld,%10ld\n",ix,sd->working[ix*2],sd->working[ix*2+1]);
    }
}
int int_act;
void sighandler(int signum)
{
    switch (signum)
    {
    case SIGUSR1:
        int_act=99;
        break;
    case SIGINT:
        int_act = SORTING_BREAK;
    }
}

void quick_sort(uint8_t *,size_t ,size_t ,itcmp);
int qq_cmp(const void *,const void *);

static inline int load(FILE *hdl,uint8_t *buf,int64_t idx)
{
    fseek(hdl,idx * OBJSIZE,SEEK_SET);
    fread(buf,OBJSIZE,1,hdl);
    if(ferror(hdl)){
        ERROR_BY_ERRNO();
        return -1;
    }
    return 0;
}
static inline int save(FILE *hdl,uint8_t *buf,int64_t idx)
{
    fseek(hdl,idx * OBJSIZE,SEEK_SET);
    fwrite(buf,OBJSIZE,1,hdl);
    if(ferror(hdl)){
        ERROR_BY_ERRNO();
        return -1;
    }
    return 0;
}
#ifndef NDEBUG
FILE *out;

int same_block(const uint8_t *src,const uint8_t *dst,int len);
#endif
int64_t qsort_partition(FILE *fh,int64_t pos1,int64_t pos2){
    uint8_t pivot[OBJSIZE];
    uint8_t store[OBJSIZE];

    if(load(fh,pivot,pos2-1)){
        return -1;
    }
    int64_t store_idx=pos1;
    while(1){
        if(store_idx==pos2-1){
            return store_idx;
        }
        if(load(fh,store,store_idx)){
            return -1;
        }
        if(qq_cmp(store,pivot)>0){
            store_idx++;
        } else {
            break;
        }
    }
    for(int64_t ix=store_idx+1;ix<pos2-1;ix++){
        uint8_t oix[OBJSIZE];
        if(load(fh,oix,ix)){
            return -1;
        }
        if(qq_cmp(oix,pivot)>0){
            if(save(fh,oix,store_idx)){
                return -1;
            }
            if(save(fh,store,ix)){
                return -1;
            }
            store_idx++;
            if(load(fh,store,store_idx)){
                return -1;
            }

        }
    }
    if(save(fh,store,pos2-1)){
        ERROR_BY_ERRNO();
        return -1;
    }
    if(save(fh,pivot,store_idx)){
        ERROR_BY_ERRNO();
        return -1;
    }
    return store_idx;
}
void qq_fprint(FILE*,void *,int64_t ,const char *);
void *sort32_task(void *obj)
{
    int id=(long)obj;
    struct run_data *sd=&sort_data;
    long pd[2];

    FILE *fh;
    fh=fopen(sd->fname,"r+");
    if (fh==NULL)
    {
        ERROR(sd->fname);
        ERROR_BY_ERRNO();
        int_act=SORTING_BREAK;
        return NULL;
    }
    while(1){
        MLOCK(sd->mutex)
        sd->working[id*2]=0;
        sd->working[id*2+1]=0;
        if(bag_get(sd->jobs,&pd[0])==EMPTY){
            int all_zero=1;
            for(int ix=0;ix<cpunum*2;ix++){
                if(sd->working[ix]){
                    all_zero=0;
                    break;
                }
            }
            if(all_zero){
                // 全部线程停止，已经没有任务。排序结束
                MBOARDCAST(sd->cond)
                MUNLOCK(sd->mutex)
                goto finish;
            }
            MWAIT(sd->cond,sd->mutex)
            MUNLOCK(sd->mutex)
            continue;
        }
#ifndef NDEBUG
        fprintf(out,"(%4ld,%4ld)\n",pd[0],pd[1]);
#endif
        sd->working[id*2]=pd[0];
        sd->working[id*2+1]=pd[1];
        MUNLOCK(sd->mutex)
        // 线程业务开始
        
        int64_t pidx=qsort_partition(fh,pd[0],pd[1]);
        if(pidx<0){
            int_act=SORTING_BREAK;
            PRE("id: %d,出错了",id);
            goto finish;
        }

        if(pidx-pd[0]>1){
        //if(pd[0]<(pidx-1)){
            MLOCK(sd->mutex)
            bag_put2(sd->jobs,pd[0],pidx);
            MSIGNAL(sd->cond);
            MUNLOCK(sd->mutex)
        }
        if(pd[1]-pidx>2){
        //if(pidx<(pd[1]-2)){
            MLOCK(sd->mutex)
            bag_put2(sd->jobs,pidx+1,pd[1]);
            MSIGNAL(sd->cond);
            MUNLOCK(sd->mutex)
        }
    }
finish:
    fclose(fh);
    printf("id %d is quit!\n",id);
    sd->working[id*2]=0;
    sd->working[id*2+1]=0;

    return NULL;
}
int full_path(char *,const char *);
void sort32(struct STATUS *sta)
{
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
    pthread_t *pid;
    sort_data.cond=&cond;
    sort_data.mutex=&mutex;
    sort_data.working=malloc(8*cpunum*2);

    char *fn=malloc(256);
    bzero(sort_data.working,cpunum * 16);
#ifndef NDEBUG
    out=fopen("/home/tec/big.log","w+");
#endif
    full_path(fn,sta->dst);
    sort_data.fname=fn;
    sort_data.jobs=bag_with_array(sta->scope,sta->scope_cnt);
    pid=malloc(sizeof(pthread_t)*cpunum);
    for(int ix=0;ix<cpunum;ix++){
        pthread_create(pid+ix,NULL,sort32_task,(void *)(uint64_t)ix);
    }
    while(1){
        pause();
        if(int_act==SHOW_PROGRESS){
        }
        if(int_act==SORTING_BREAK){
            break;
        }
    }
    if(has_error()){
        print_error_stack(stdout);
    }
    for(int ix=0;ix<cpunum;ix++){
        pthread_join(*(pid+ix),NULL);
    }
    free(fn);
    free(pid);
    free(sort_data.working);
#ifndef NDEBUG
    fclose(out);
#endif
    return;
}
#define BUFSIZE 1024 * 1024
#define OBJSIZE 12
int sort_test(const char *fname)
{
    FILE *fh=fopen(fname,"r");
    if(fh==NULL){
        ERROR_BY_ERRNO();
        return -1;
    }
    printf("测试文件%s\n",fname);
    uint8_t *buf=malloc(BUFSIZE * OBJSIZE);
    size_t offset=0;
    while(1){
        size_t cnt=fread(buf,OBJSIZE,BUFSIZE,fh);
        for(size_t ix=0;ix<cnt-1;ix++){
            offset ++;
            if(qq_cmp(buf+(ix+1)*OBJSIZE,buf+ix*OBJSIZE)>0){
                printf("数据没有排序(偏移:%ld)\n",offset);
                free(buf);
                fclose(fh);
                return 0;
            }
        }
        if(cnt < BUFSIZE){
            break;
        }
        fseek(fh,-OBJSIZE,SEEK_CUR);
    }
    printf("data is in order\n");
    free(buf);
    fclose(fh);
    return 0;
}


void *test_task(void *x)
{
    printf("pid:%d\n",getpid());
    for(int i=0;i<10;i++){
        sleep(1);
        printf("%d ",i);
        fflush(stdout);
    }
    kill(getpid(),SIGINT);
    return NULL;
}
void test_signal(void){
    pthread_t id;
    pthread_create(&id,NULL,test_task,NULL);
    while(1){
        pause();
        if(int_act == SHOW_PROGRESS){
            printf("show progress\n");
        }
        if(int_act== SORTING_BREAK){
            printf("中断了\n");
            break;
        }
    }
    pthread_join(id,NULL);
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