#include <stdio.h>
#include <wchar.h>
#include <malloc.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "error_stack.h"
#include "memdb.h"
#include "sort_range.h"

const struct timespec MS1   = {0, 100000000};
const struct timespec NS100 = {0, 1000000};

int debug_print;
struct sort_mgr{
    pthread_cond_t      *cond;
    pthread_mutex_t     *mutex;
    long                *works;
    struct RANGES       *scope;
    void                *data;
    const struct ENTITY *ent;
    long                 run;
};
struct sort_mgr smr;
extern struct ENV env;
void signalhandler(int signum)
{
    CP_MSG(L"signal number %d\n",signum);
    if(signum==SIGINT){
        smr.run=0;
        pthread_cond_broadcast(smr.cond);
        return;
    }
    if(signum==SIGUSR2){
        debug_print=1;
        return;
    }
    pthread_mutex_lock(smr.mutex);
    rng_print(smr.scope,4);
    for(int ix=0;ix<env.cpunum;ix++){
        long l1=*(smr.works+ix*2);
        long l2=*(smr.works+ix*2+1);
        if(l1!=0 || l2!=0){
            wprintf(L"task num %2d: %4ld (%9ld,%9ld)\n",ix,l2-l1,l1,l2);
        }
    }
    pthread_mutex_unlock(smr.mutex);
}

long partion(void *base,const struct ENTITY *ent,long p1,long p2){
    // 条件p2 - p1 > 1必须成立,在调用此函数时检查
    const int incrment=ent->unitsize;
    char store[incrment];
    if(p1+2==p2){
        void *pp1=smr.data + p1 * incrment;
        void *pp2=smr.data + p2 * incrment;
        if(smr.ent->lt(pp1,pp2)){
            
            memcpy(store,pp1,incrment);
            memcpy(pp1,pp2,incrment);
            memcpy(pp2,store,incrment);
        }
        return p1+1;
    }
    
    long store_idx=p1;
    void *povit=base+(p2-1) * incrment;
    void *pstore;
    while(1){
        pstore=base + (store_idx*incrment);
        if(ent->lt(pstore,povit)){
            store_idx++;
        }else{
            break;
        }
    }

    memcpy(store,pstore,incrment);
    for(long ix=store_idx+1;ix<p2-1;ix++){
        void *pix=base + ix * incrment;
        if(ent->lt(pix,povit)){
            memcpy(pstore,pix,incrment);
            memcpy(pix,store,incrment);
            store_idx++;
            pstore=base + (store_idx*incrment);
            memcpy(store,pstore,incrment);
        }
    }
    memcpy(pstore,povit,incrment);
    memcpy(povit,store,incrment);
    return store_idx;
}

void *task(void *arg)
{
    int id=(long)arg;
    pthread_mutex_lock(smr.mutex);
    while(smr.run){
        //wprintf(L"id %2d lock success\n",id);
        long l1,l2;
        if(rng_pop(smr.scope,&l1,&l2)<0){
            //检查全部线程是否都进入了等待
            int all_done=1;
            for(int ix=0;ix<env.cpunum;ix++){
                if(ix==id)continue;
                if(*(smr.works+ix*2)){
                    all_done=0;
                    break;
                }
                if(*(smr.works+ix*2+1)){
                    all_done=0;
                    break;
                }
            }
            if(all_done){
                pthread_cond_broadcast(smr.cond);
                smr.run=0;
                wprintf(L"task %d is last job\n",id);
                goto task_exit;
            }
            pthread_cond_wait(smr.cond,smr.mutex);
            continue;
        }
        pthread_mutex_unlock(smr.mutex);
        while(smr.run){
            *(smr.works+id *2)=l1;
            *(smr.works+id *2+1)=l2;
            long povit=partion(smr.data,smr.ent,l1,l2);
            if(debug_print){
                debug_print=0;
                CP_MSG(L"partition result(%2d):%ld,%ld,%ld\n",id,l1,povit,l2);
            }
            if(povit - l1 > 1){
                if(l2 - povit > 2){
                    pthread_mutex_lock(smr.mutex);
                    rng_push(smr.scope,povit+1,l2);
                    pthread_cond_signal(smr.cond);
                    pthread_mutex_unlock(smr.mutex);
                }
                l2=povit;
            } else {
                if(l2-povit > 2){
                    l1=povit+1;
                }else {
                    break;
                }
            }
        }
        *(smr.works+id *2)=0;
        *(smr.works+id *2+1)=0;
        pthread_mutex_lock(smr.mutex);
    }
task_exit:
    pthread_mutex_unlock(smr.mutex);
    return (void*)(long)id;
}
/**
 * @brief sort hurge amount of data in memory
 * @param cfgname 包含有多少单元要排序，目标文件名
 * @param cmp compare function
 */
void bigsort(struct MEMDB *db)
{
    //db->raw
    //db->total

    pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    smr.cond=&cond;
    smr.mutex=&mutex;
    smr.scope=db->scops;
    smr.data=db->raw;
    smr.ent=db->entity;
    
    pthread_t *pid=malloc(sizeof(pthread_t)*env.cpunum);
    smr.works=malloc(sizeof(long)*2*env.cpunum);
    bzero(smr.works,sizeof(long)*env.cpunum*2);
    smr.run=1;

    for(int ix=0;ix<env.cpunum;ix++){
        pthread_create(pid+ix,NULL,task,(void *)(long)ix);
    }
    for(int ix=0;ix<env.cpunum;ix++){
        void *retv;
        pthread_join(*(pid+ix),&retv);
        wprintf(L"task %2d is done\n",(int)(long)retv);
    }
    

    rng_print(db->scops,10);
    CP_MSG(L"continue%s","\n");

}