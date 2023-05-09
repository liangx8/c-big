/* 实践多线程的一下概念用法 */

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <malloc.h>
#include "pairs.h"
const struct timespec MS1 = {0, 100000000};



// 实验1:  1主线程 1 子线程
struct MUTEX_COND
{
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    pthread_t tid;
    int *chan;
    int seq;
};
void *child(void *obj)
{
    struct MUTEX_COND *pd = obj;
    int base = pd->seq * 10;
    for (int ix = 0; ix < 5; ix++)
    {
        int val = base + ix;
        nanosleep(&MS1, NULL);

        pthread_mutex_lock(pd->mutex);
        *(pd->chan) = val;
        pthread_mutex_unlock(pd->mutex);
        pthread_cond_signal(pd->cond);
    }
    return NULL;
}
void one_master_one_slave(void){
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    int chan;
    pthread_t tid;
    struct MUTEX_COND *data[1];
    chan=0;
    {
        struct MUTEX_COND *ld = malloc(sizeof(struct MUTEX_COND));
        ld->mutex = &mutex;
        ld->cond = &cond;
        ld->chan = &chan;
        ld->seq = 0;
        pthread_create(&tid,NULL,child,ld);
        ld->tid=tid;
        data[0]=ld;
    }
    pthread_mutex_lock(&mutex);
    for(int ix=0;ix<5;ix++){
        pthread_cond_wait(&cond,&mutex);
        printf("%d\n",chan);
        chan=0;
    }
    pthread_mutex_unlock(&mutex);
    pthread_join(data[0]->tid,NULL);

}
// 实验1 结束

// 实验2:  1主线程 4 子线程
// sem_init,sem_wait,sem_post,sem_destroy
// 当一次交互完成后, 会产生生产/消费 双方对mlock的抢夺，事实上，在这个时候应该是生产方获取mlock以继续生产
struct Exam2Data{
    pthread_mutex_t *tlock,*mlock;
    pthread_cond_t *cond;
    int *chan;
    int seq;
};

void *child1(void *obj){
    struct Exam2Data *e2d=obj;
    for(int ix=0;ix<5;ix++){
        int val=e2d->seq *10 + ix;
        pthread_mutex_lock(e2d->tlock);
        printf("seq:%2d locked tlock\n",e2d->seq);
        pthread_mutex_lock(e2d->mlock);
        *e2d->chan=val;
        pthread_cond_wait(e2d->cond,e2d->mlock);
        pthread_mutex_unlock(e2d->mlock);
        pthread_mutex_unlock(e2d->tlock);

        nanosleep(&MS1, NULL);
    }
    return NULL;
}
/**
 * @brief 用 <code>条件信号</code>配合<code>互斥锁</code> 1对多线程的通信
 **/
void one_master_four_slave(void){
    pthread_mutex_t ml=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t tl=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
    pthread_t tid[4];
    int val=99;
    for(int ix=0;ix<4;ix++){
        struct Exam2Data *e2d=malloc(sizeof(struct Exam2Data));
        e2d->mlock=&ml;
        e2d->tlock=&tl;
        e2d->chan=&val;
        e2d->cond=&cond;
        e2d->seq=ix;
        pthread_create(&tid[ix],NULL,child1,e2d);
    }
    sleep(1);
    for(int ix=0;ix<20;ix++){
        int this;
        pthread_mutex_lock(&ml);
        this=val;
        pthread_mutex_unlock(&ml);
        printf("%2d:%2d\n",ix,this);
        pthread_cond_signal(&cond);
    }
    for(int ix=0;ix<4;ix++){
        pthread_join(tid[ix],NULL);
    }
    printf("Exam2 Done\n");
}
// 实验2 结束

// 实验3 在cond_wait之前就cond_signal
void exam3(void){
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
    printf("begin\n");
    pthread_mutex_lock(&mutex);
    printf("before signal wait\n");
    pthread_cond_signal(&cond);
    printf("before wait\n");
    pthread_cond_wait(&cond,&mutex);
    printf("after wait\n");
    pthread_mutex_unlock(&mutex);
    printf("end\n");
}
// 得出结果：调用一次pthread_cond_signal,如果没有pthread_cond_wait在等待
// 实验3 结束
void test_mutex_lock(void)
{
    one_master_four_slave();
    //exam3();
}
