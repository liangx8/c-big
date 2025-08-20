#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <string.h> // memset()
#include <pthread.h>
#include <unistd.h>
#include "abstract_db.h"
#include "sort_range.h"
#include "log.h"
uint64_t _mem_partion(void *base,const struct ENTITY *ent,uint64_t p1,uint64_t p2); // mem_sort.c

struct MANAGER{
    struct ABSTRACT_DB *db;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    uint64_t *jobs;
    int run;
} smr;
extern struct ENV env;
void *sort_task(void *load)
{
    int id=(int)(long)load;
    uint64_t *idholder=smr.jobs + id * 2;
    log_info(L"task %2d enter\n",id);
    pthread_mutex_lock(smr.mutex);
    while(smr.run){
#if 1
        uint64_t l1,l2;
        if(rng_pop(smr.db->scops,&l1,&l2)){
            int all_done=1;
            *idholder=0;
            *(idholder+1)=0;
            for(int ix=0;ix<env.cpunum*2;ix++){
                if(*(smr.jobs+ix)){
                    all_done=0;
                    break;
                }
            }
            if(all_done){
                break;
            } else {
                pthread_cond_wait(smr.cond,smr.mutex);
            }
        } else {
            
            int pick;
            loop:
            *idholder=l1;
            *(idholder+1)=l2;
            pthread_mutex_unlock(smr.mutex);
            uint64_t povit = _mem_partion(smr.db->raw,smr.db->entity, l1,l2);
            pthread_mutex_lock(smr.mutex);
            pick=0;
            // 不用２个都入栈，有一个就继续跑，１个以上才如其中一个
            if (povit - l1 > 1) pick=1;
            if (l2-povit>1) pick = pick | 0b10;
            switch(pick){
                case 0:// 当前任务完成
                    continue;//申领新任务
                case 1:// 单个后续任务
                    l2=povit;
                    goto loop;
                case 3:
                    rng_push(smr.db->scops,l1,povit);
                    pthread_cond_signal(smr.cond);
                case 2:
                    l1=povit;
                    goto loop;

            }

        }
#else
        pthread_mutex_unlock(smr.mutex);
        log_info(L"task %2d is working...\n",id);
        sleep(id+3);
        pthread_mutex_lock(smr.mutex);
#endif
    }
    pthread_mutex_unlock(smr.mutex);
    log_info(L"task %2d is quit\n",id);
    return NULL;
}
int new_signal(int *);
void signal_init(void);
void mem_mult_quick_sort(struct ABSTRACT_DB *db)
{
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
    uint64_t jobs[env.cpunum*2];
    smr.mutex=&mutex;
    smr.cond=&cond;
    smr.db=db;
    smr.run=1;
    smr.jobs = &jobs[0];
    pthread_t thrs[env.cpunum];
    memset(&jobs[0],0,sizeof(uint64_t)*2 * env.cpunum);
    signal_init();
    for(int ix=0;ix<env.cpunum;ix++){
        pthread_create(&thrs[ix],NULL,sort_task,(void*)(long)ix);
    }
    while(1){
        int sig;
        sleep(1);
        if(new_signal(&sig)){
            smr.run=0;
            log_info(L"main thread exit by manual\n");
            break;
        }
        int all_done=1;
        for(int ix=0;ix<env.cpunum * 2;ix++){
            if(jobs[ix]){
                all_done=0;
                break;
            }
        }
        if(all_done){
            log_info(L"排序完成\n");
            pthread_cond_broadcast(&cond);
            break;
        }
    }
    for(int ix=0;ix<env.cpunum;ix++){
        pthread_join(thrs[ix],NULL);
    }
}