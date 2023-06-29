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

#ifdef NDEBUG
#define PRE(m) while(1==2)
#else
#define PRE(body) printf("%s(%3d):",__FILE__,__LINE__);body
#endif
#define CACHE_AMOUNT 524288
#define MLOCK(mut)      if(pthread_mutex_lock(mut)) {ERROR("线程锁错误\n");return NULL;}
#define MUNLOCK(mut)    if(pthread_mutex_unlock(mut)) {ERROR("线程解锁错误\n");return NULL;}
#define MWAIT(cnd,mut)  if(pthread_cond_wait(cnd,mut)){ERROR("ptread_cond_wait() 错误\n");return NULL;}
#define MBOARDCAST(cnd) if(pthread_cond_broadcast(cnd)){ERROR("pthread_cond_broadcast() 错误\n"); return NULL;}
#define MSIGNAL(cnd)    if(pthread_cond_signal(cnd)){ERROR("pthread_cond_signal() 错误\n"); return NULL;}

#define OBJSIZE 12

struct run_data{
    struct Bag *jobs;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    const char *fname;
    // 处于活动状态的工作的数量
    int act_cnt;
    int working;
}sort_data;
extern int int_act; // define in main.c
extern int cpunum;


int quick_sort(uint8_t *,int ,int ,int (*)(void *,void*));
int qq_lt(void *,void *);

static inline int load(FILE *hdl,uint8_t *buf,int64_t idx)
{
    fseek(hdl,idx * OBJSIZE,SEEK_SET);
    if(fread(buf,OBJSIZE,1,hdl)==0){
        ERROR_BY_ERRNO();
        return -1;
    }
    return 0;
}
static inline int save(FILE *hdl,uint8_t *buf,int64_t idx)
{
    fseek(hdl,idx * OBJSIZE,SEEK_SET);
    if(fwrite(buf,OBJSIZE,1,hdl)==0){
        ERROR_BY_ERRNO();
        return -1;
    }
    return 0;
}
int order2(FILE *fh,int64_t pos)
{
    uint8_t buf[OBJSIZE*2];
    uint8_t fub[OBJSIZE*2];
    fseek(fh,pos*OBJSIZE,SEEK_SET);
    if(fread(buf,OBJSIZE,2,fh)<2){
        ERROR_BY_ERRNO();
        return -1;
    }
    if(qq_lt(buf+OBJSIZE,buf)){
        memcpy(fub,buf + OBJSIZE, OBJSIZE);
        memcpy(fub + OBJSIZE,buf, OBJSIZE);
        fseek(fh,pos*OBJSIZE,SEEK_SET);
        if(fwrite(fub,OBJSIZE,2,fh)<2){
            ERROR_BY_ERRNO();
            return -1;
        }
    }
    return 0;
}
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
        if(qq_lt(store,pivot)){
            store_idx++;
        } else {
            break;
        }
    }
    for(int64_t ix=store_idx+1;ix<pos2-1;ix++){
        uint8_t oix[OBJSIZE];
        if(load(fh,oix,ix)){
            ERROR_BY_ERRNO();
            return -1;
        }
        if(qq_lt(oix,pivot)){
            if(save(fh,oix,store_idx)){
                ERROR_BY_ERRNO();
                return -1;
            }
            if(save(fh,store,ix)){
                ERROR_BY_ERRNO();
                return -1;
            }
            store_idx++;
            if(load(fh,store,store_idx)){
                ERROR_BY_ERRNO();
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
void *sort32_task(void *obj)
{
    long id=(long)obj;
    struct run_data *sd=&sort_data;
    long pd[2];
    FILE *fh=fopen(sd->fname,"r+");
    if (fh==NULL)
    {
        ERROR(sd->fname);
        ERROR_BY_ERRNO();
        return NULL;
    }
    MLOCK(sd->mutex)
    while(sd->working){
        if(bag_get(sd->jobs,&pd[0])==EMPTY){
            if(sd->act_cnt==1){
                sd->working=0;
                MBOARDCAST(sd->cond);
                break;
            }
            sd->act_cnt--;
//            PRE(printf("id %2ld sleep\n",id));
            MWAIT(sd->cond,sd->mutex)
//            PRE(printf("id %2ld wakeup\n",id));
            sd->act_cnt++;
            continue;
        }
        MUNLOCK(sd->mutex);
        int64_t amount=pd[1]-pd[0];
        PRE(printf("id:%2ld[%8ld,%8ld] = %ld\n",id,pd[0],pd[1],amount));
        if(amount<=CACHE_AMOUNT){
            int64_t bufsize=amount*OBJSIZE;
            uint8_t *buf=malloc(bufsize);
            if(buf==NULL){
                ERROR_BY_ERRNO();
                sd->working=0;
                MBOARDCAST(sd->cond)
                break;
            }
            fseek(fh,pd[0]*OBJSIZE,SEEK_SET);
            if(fread(buf,OBJSIZE,amount,fh)<amount){
                ERROR_BY_ERRNO();
                sd->working=0;
                MBOARDCAST(sd->cond)
                break;
            }
            quick_sort(buf,amount*OBJSIZE,OBJSIZE, qq_lt);
            fseek(fh,pd[0]*OBJSIZE,SEEK_SET);
            if(fwrite(buf,OBJSIZE,amount,fh)<amount){
                ERROR_BY_ERRNO();
                sd->working=0;
                MBOARDCAST(sd->cond)
                break;
            }
            free(buf);
        } else {
            int64_t mid=qsort_partition(fh,pd[0],pd[1]);
//            PRE(printf("id:%ld(pivot index:%ld,pd0:%ld,pd1:%ld)\n",id,mid,pd[0],pd[1]));
            if(mid<0){
                ERROR_BY_ERRNO();
                sd->working=0;
                MBOARDCAST(sd->cond)
                break;
            }
            int64_t bw=mid-pd[0];
//            PRE(printf("id:%ld,%ld\n",id,bw));
            switch(bw){
                case 2:
                    if(order2(fh,pd[0])){
                        sd->working=0;
                        MBOARDCAST(sd->cond)
                        goto outer;
                    }
                case 0:
                case 1:
                // 不用再排序
                break;
                default:
                    MLOCK(sd->mutex)
                    bag_put2(sd->jobs,pd[0],mid);
                    MSIGNAL(sd->cond);
                    MUNLOCK(sd->mutex)
            }
            bw=pd[1]-mid;
//            PRE(printf("id:%ld,bw:%ld,pd1:%ld\n",id,bw,pd[1]));
            switch ((bw))
            {
            case 0:
                ERROR("内部错误：不可能发生");
                goto outer;
            case 3:
                if(order2(fh,mid+1)){
                    sd->working=0;
                    MBOARDCAST(sd->cond)
                    goto outer;
                }
            case 1:
            case 2:
            break;

            default:
                MLOCK(sd->mutex);
                bag_put2(sd->jobs,mid+1,pd[1]);
                MSIGNAL(sd->cond);
                continue;
            }
        }

        MLOCK(sd->mutex);
    }
outer:
    MUNLOCK(sd->mutex)
    fclose(fh);
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
    sort_data.working=1;
    sort_data.act_cnt=cpunum;
    char *fn=malloc(256);
    full_path(fn,sta->dst);
    sort_data.fname=fn;
    sort_data.jobs=bag_with_array(sta->scope,sta->scope_cnt);
    bag_print(sort_data.jobs,stdout,10);
    pid=malloc(sizeof(pthread_t)*cpunum);
    for(int ix=0;ix<cpunum;ix++){
        pthread_create(pid+ix,NULL,sort32_task,(void *)(uint64_t)ix);
    }

    for(int ix=0;ix<cpunum;ix++){
        pthread_join(*(pid+ix),NULL);
    }
    free(pid);
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
    while(1){
        size_t cnt=fread(buf,OBJSIZE,BUFSIZE,fh);
        for(size_t ix=0;ix<cnt-1;ix++){
            if(qq_lt(buf+(ix+1)*OBJSIZE,buf+ix*OBJSIZE)){
                printf("数据没有排序\n");
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