/* 实践多线程的一下概念用法 */

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <malloc.h>
#include <stdint.h>
#include "bag.h"
const struct timespec MS1   = {0, 100000000};
const struct timespec NS100 = {0, 1000000};



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
// 两个锁，两个条件信号
struct Exam2Data{
    pthread_mutex_t *tm,*mm;
    pthread_cond_t *tc,*mc;
    int *chan;
    int seq;
};

void *child1(void *obj){
    struct Exam2Data *e2d=obj;
    for(int ix=0;ix<5;ix++){
        int val=e2d->seq *10 + ix;
        pthread_mutex_lock(e2d->tm);
        pthread_mutex_lock(e2d->mm);
        *e2d->chan=val;
        pthread_cond_signal(e2d->mc);
        pthread_cond_wait(e2d->mc,e2d->mm);

        pthread_mutex_unlock(e2d->mm);
        pthread_mutex_unlock(e2d->tm);
        nanosleep(&MS1, NULL);
    }
    return NULL;
}
/**
 * @brief 用 <code>条件信号</code>配合<code>互斥锁</code> 1对多线程的通信
 **/
void exam2(void){
    pthread_mutex_t mm=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t tm=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t mc=PTHREAD_COND_INITIALIZER;
    pthread_t tid[4];
    int val=99;
    for(int ix=0;ix<4;ix++){
        struct Exam2Data *e2d=malloc(sizeof(struct Exam2Data));
        e2d->mm=&mm;
        e2d->tm=&tm;
        e2d->mc=&mc;
        e2d->chan=&val;
        e2d->seq=ix;
        pthread_create(&tid[ix],NULL,child1,e2d);
    }
    for(int ix=0;ix<20;ix++){
        int this;
        pthread_mutex_lock(&mm);
        if(val==99){
            pthread_cond_wait(&mc,&mm);
        }
        this=val;
        val=99; // val 已经被取出来。
        pthread_cond_signal(&mc);
        pthread_mutex_unlock(&mm);
        printf("%2d:%2d\n",ix,this);
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
    exam2();
    //exam3();
}

// unit test 2 ---------------------------------------
// 类似快速排序的分区测试
struct test2_data{
    struct Bag *mp;
    pthread_mutex_t   *mutex;
    //线程之间都是满负荷，容器中空的可能性比较低，只在开始和最后才会出现
    pthread_cond_t    *cond;
    int               tnum;
    int               done;
    int               seq;
}t2d;
extern int int_act; // define in main.c
void *test2_child(void *obj){
    /*
    备忘：
    非线程安全，跑起来没问题，但是还没考虑如何验证结果
    */
    struct test2_data *td=&t2d;
    //int64_t id=(int64_t) obj;
    int64_t data[2];
    int idle;
    pthread_mutex_lock(td->mutex);
    idle=1;
    int64_t id=(int64_t)obj;
    while(1){
        int res=bag_get(td->mp,&data[0]);
        if(res==EMPTY){
            // 没有新的任务数据
            // 在空闲状态吗？
            if(idle==0){
                // 进入空闲状态
                idle=1;
                td->tnum--;
            }
            pthread_cond_wait(td->cond,td->mutex);
            if(td->done){
                // 判断是否任务已经结束
                break;
            }
            continue;
        }
        // data中肯定有数据了
        if(idle){
            // 退出空闲状态
            td->tnum++;
        }
        pthread_mutex_unlock(td->mutex);
        int64_t left=data[0];
        int64_t right=data[1];
        int64_t mid=(right+left)/2;
        idle=0;
        if(int_act==1){
            int_act=0;
            bag_print(td->mp,stdout,10);
        }
        printf("seq:%d:id:%ld,(%ld,%ld)\n",td->seq,id,data[0],data[1]);
        td->seq ++;
        nanosleep(&NS100,NULL);
        pthread_mutex_lock(td->mutex);
       
        int noData=1;
        if(mid-left>2){
            bag_put(td->mp,left,mid);
            pthread_cond_signal(td->cond);
            noData=0;
        }
        if(right-mid>2){
            bag_put(td->mp,mid,right);
            pthread_cond_signal(td->cond);
            noData=0;
        }
        if(noData && td->tnum==1){
            // 没有进程在运行。通知全部进程退出
            pthread_cond_broadcast(td->cond);
            td->done=1;
            break;
        }

    }
    pthread_mutex_unlock(td->mutex);
    return NULL;
}
const int64_t init_data[]={0,400,400,1000};
extern int cpunum;
void unit_test2(void)
{

    struct Bag *mp=bag_with_array(init_data,4);
    pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    
    pthread_t pid[cpunum];
    t2d.cond=&cond;
    t2d.mutex=&mutex;
    t2d.tnum=0;
    t2d.done=0;
    t2d.seq=0;
    t2d.mp=mp;
    bag_print(mp,stdout,10);
    for(int ix=0;ix<cpunum;ix++){
        pthread_create(&pid[ix],NULL,test2_child,(void *)(uint64_t)ix);
    }
    for(int ix=0;ix<cpunum;ix++){
        pthread_join(pid[ix],NULL);
    }
    bag_free(mp);

}
void unit_test(void){
    unit_test2();
}
