/**
 * 非阻塞的容器
 */

#include <stdint.h>
#include <malloc.h>
#include <pthread.h>
#include <assert.h>
#include "error_stack.h"
#define MAX 1000
struct Pairs
{
    int64_t data[MAX * 2];
    struct Pairs *next;
    // 以1单位递增，但是数据本身是成对增加,
    uint32_t idx;
    uint32_t unused;
};

struct MutexPairs
{
    // 非阻塞式的容器，应该只需要１个排他锁
    pthread_mutex_t mutex;
    struct Pairs *head;
};

struct MutexPairs *mutex_create(void)
{
    struct MutexPairs *mp = malloc(sizeof(struct MutexPairs));
    pthread_mutex_init(&mp->mutex, NULL);

    mp->head=malloc(sizeof(struct Pairs));
    mp->head->idx=0;
    mp->head->next=NULL;
    return mp;
}
struct MutexPairs *mutex_with_array(const int64_t *data, int size)
{
    assert((size & 1) == 0);
    struct MutexPairs *mp = mutex_create();
    mp->head = malloc(sizeof(struct Pairs));
    struct Pairs *ptr = mp->head;

    while (size >= 0)
    {
        int sec;
        if (size < MAX)
        {
            sec = size;
        }
        else
        {
            sec = MAX;
        }
        size = size - MAX;
        for (int ix = 0; ix < sec; ix++)
        {
            ptr->data[ix] = data[ix];
            ptr->idx = sec;
        }
        if (size >= 0)
        {
            ptr->next = malloc(sizeof(struct Pairs));
            ptr = ptr->next;
        }
        else
        {
            break;
        }
    }
    return mp;
}
void pairs_free(struct Pairs *);
void mutex_free(struct MutexPairs *mp)
{
    pthread_mutex_destroy(&mp->mutex);
    pairs_free(mp->head);
    free(mp);
}

/**
 * @brief 获取一个数据
 * @param ps 数据对容器
 * @param ref 如果容器无内容, 堵塞
 * @return 出错返回-1,空容器返回 1,否则 0
 */
int mutex_get(struct MutexPairs *mps, int64_t **ref)
{

    if (pthread_mutex_lock(&mps->mutex))
    {
        ERROR("锁错误");
        return -1;
    }
    struct Pairs *ptr = mps->head;
    while (ptr)
    {
        if (ptr->idx)
        {
            int idx;
            ptr->idx--;
            idx = ptr->idx;
            *ref = &ptr->data[idx * 2];
            if (pthread_mutex_unlock(&mps->mutex))
            {
                ERROR("解锁锁错误");
                return -1;
            }
            return 0;
        }
        ptr = ptr->next;
    }
    if (pthread_mutex_unlock(&mps->mutex))
    {
        ERROR("解锁锁错误");
        return -1;
    }
    return 1;
}
int mutex_put(struct MutexPairs *mps, int64_t p1, int64_t p2)
{
    if (pthread_mutex_lock(&mps->mutex))
    {
        ERROR("锁错误");
        return -1;
    }
    struct Pairs *ptr = mps->head;
    while (1)
    {
        if (ptr->idx == MAX)
        {
            if (ptr->next == NULL)
            {
                ptr->next = malloc(sizeof(struct Pairs));
            }
            ptr = ptr->next;
            ptr->idx=0;
        }
        else
        {
            int idx = ptr->idx * 2;
            ptr->idx++;
            ptr->data[idx] = p1;
            ptr->data[idx + 1] = p2;
            if (pthread_mutex_unlock(&mps->mutex))
            {
                ERROR("解锁锁错误");
                return -1;
            }
            return 0;
        }
    }
}
void pairs_print(struct Pairs *ps, FILE *out, int limit);
int mutex_print(struct MutexPairs *mps, FILE *out, int limit)
{
    if (pthread_mutex_lock(&mps->mutex))
    {
        ERROR("锁错误");
        return -1;
    }
    pairs_print(mps->head, out, limit);
    if (pthread_mutex_unlock(&mps->mutex))
    {
        ERROR("解锁锁错误");
        return -1;
    }
    return 0;
}
extern struct timespec MS1;
// unit test 1------------------------------
// 简单的４进程５次循环测试
const struct timespec NS100={0,10000000};

struct test_struct{
    struct MutexPairs *mp;
    int64_t id;
};
void *test_pair_child(void *obj)
{
    struct test_struct *ts=obj;
    for(long lx=0;lx<5;lx++){
        mutex_put(ts->mp,ts->id,lx);
        nanosleep(&MS1,NULL);
    }

    return NULL;
}
void unit_test1(void){
    pthread_t id[4];
    struct MutexPairs *mp=mutex_create();
    int64_t *pd;
    for(int px=0;px<4;px++){
        struct test_struct *pts=malloc(sizeof(struct test_struct));
        pts->mp=mp;
        pts->id=px;
        pthread_create(&id[px],NULL,test_pair_child,pts);
    }
    for(int ix=0;ix<20;ix++){
        while(1){
            int res=mutex_get(mp,&pd);
            if(res==-1){
                printf("出错了\n");
                return;
            }
            if(res==0){
                break;
            }
        }
        printf("id:%4ld num:%4ld\n",pd[0],pd[1]);
    }
    for(int ix=0;ix<4;ix++){
        pthread_join(id[ix],NULL);
    }

}
// unit test 2 ---------------------------------------
// 类似快速排序的分区测试
struct test2_data{
    struct MutexPairs *mp;
    pthread_mutex_t   *mutex;
    //线程之间都是满负荷，容器中空的可能性比较低，只在开始和最后才会出现
    pthread_cond_t    *cond;
    int               *tnum;
    int64_t           id;
};
void *test2_child(void *obj){
    // 设计一个空闲标记，当检测到容器中没有新的数据，进入空闲状态并且tnum减一
    // 进入pthread_cond_wait()
    // 当容器中取得数据，并且是空闲状态取消空闲状态，tnum加一
    struct test2_data *t2d=obj;
    int64_t *pair;
    int idle=1;
    while(1){
        pthread_mutex_lock(t2d->mutex);
        int res=mutex_get(t2d->mp,&pair);
        if(res==-1){
            return NULL;
        }
        if(res==1){
            // container is empty,此时tnum一定大于0
            int tnum=*(t2d->tnum);
            *(t2d->tnum)=tnum-1;
            idle=1;
            pthread_cond_wait(t2d->cond,t2d->mutex);
            pthread_mutex_unlock(t2d->mutex);
            continue;
        }
        if(idle){
            int tnum;
            idle=0;
            tnum=*(t2d->tnum);
            *(t2d->tnum)=tnum+1;
        }
        pthread_mutex_unlock(t2d->mutex);

        int64_t left=pair[0];
        int64_t right=pair[1];
        int64_t mid=(right+left)/2;
        nanosleep(&NS100,NULL);
        pthread_mutex_lock(t2d->mutex);
        if(mid-left>2){
            mutex_put(t2d->mp,left,mid);
            pthread_cond_signal(t2d->cond);
        }
        if(right-mid>2){
            mutex_put(t2d->mp,mid,right);
            pthread_cond_signal(t2d->cond);
        }
        pthread_mutex_unlock(t2d->mutex);
    }
    return NULL;
}
const int64_t init_data[]={0,400,400,1000};
void unit_test2(void)
{
    struct MutexPairs *mp=mutex_with_array(init_data,4);
    pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    int sem;
    pthread_t pid[4];
    sem=0;
    for(int ix=0;ix<4;ix++){
        struct test2_data *t2d=malloc(sizeof(struct test2_data));
        t2d->cond=&cond;
        t2d->mutex=&mutex;
        t2d->tnum=&sem;
        t2d->id=ix;
        t2d->mp=mp;
        pthread_create(&pid[ix],NULL,test2_child,t2d);
    }

}
// unit test3 ---------------------------------------------------------
struct test3_data{
    pthread_mutex_t   *mutex;
    pthread_cond_t    *cond;
    int               *val;
    int64_t           id;

};
void *test3_child(void * payload)
{
    struct test3_data *t3d = payload;
    while(1){
        pthread_mutex_lock(t3d->mutex);
        pthread_cond_wait(t3d->cond,t3d->mutex);
        int val=*t3d->val;
        *t3d->val=val+1;
        if(val<120){
            pthread_cond_signal(t3d->cond);
        } else {
            pthread_cond_broadcast(t3d->cond);
        }
        pthread_mutex_unlock(t3d->mutex);
        if(val<120){
            printf("id(%ld),%d\n",t3d->id,val);
        } else {
            break;
        }
        nanosleep(&MS1,NULL);
    }
    return NULL;
}
void sleep(long);
void unit_test3(void)
{
    /*没有得到预期结果。已知ｂｕｇ:signal与wait不能确保重叠*/
    pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    int sem;
    pthread_t pid[4];
    for(int ix=0;ix<4;ix++){
        struct test3_data *td=malloc(sizeof(struct test3_data));
        td->cond=&cond;
        td->mutex=&mutex;
        td->val=&sem;
        td->id=ix;
        pthread_create(&pid[ix],NULL,test3_child,td);
    }
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    sem=100;
    pthread_mutex_unlock(&mutex);
    for(int ix=0;ix<4;ix++){
        pthread_join(pid[ix],NULL);
    }

}
void test_pair(void)
{
    unit_test3();
}