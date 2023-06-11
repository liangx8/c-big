/**
 * - 从空的容器中取对象会阻塞
 * - 有一个封条状态，在此状态下，容器不能在压入对象，但可以取出对象。封条状态不能取消
*/

#include <stdint.h>
#include <malloc.h>
#include <pthread.h>
#include <assert.h>
#include "error_stack.h"
#define MAX 1000
#define SEALING -2
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
    pthread_mutex_t pm; // producer mutex，必须写成实例对象，在建立的时候不用特意用另外3次malloc
    pthread_mutex_t cm; // consumer mutex
    pthread_cond_t cond;
    struct Pairs *head;
    // 当seal 为真,就put不再接受任何数据,get不会阻塞
    uint64_t seal;
};

struct MutexPairs *mutex_create(void)
{
    struct MutexPairs *mp = malloc(sizeof(struct MutexPairs));
    pthread_mutex_init(&mp->pm, NULL);
    pthread_mutex_init(&mp->cm, NULL);
    pthread_cond_init(&mp->cond, NULL);

    mp->head=malloc(sizeof(struct Pairs));
    mp->head->idx=0;
    mp->head->next=NULL;

    mp->seal=0;

    return mp;
}
struct MutexPairs *mutex_with_array(pthread_mutex_t *mutex, const int64_t *data, int size)
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
    pthread_mutex_destroy(&mp->pm);
    pthread_mutex_destroy(&mp->cm);
    pthread_cond_destroy(&mp->cond);
    pairs_free(mp->head);
    free(mp);
}

/**
 * @brief 获取一个数据
 * @param ps 数据对容器
 * @param ref 如果容器无内容, 堵塞
 * @return 出错返回-1，否则 0
 */
int mutex_get(struct MutexPairs *mps, int64_t **ref)
{

    if (pthread_mutex_lock(&mps->cm))
    {
        ERROR("锁错误");
        return -1;
    }
    while (1)
    {
        struct Pairs *ptr = mps->head;
        while (ptr)
        {
            if (ptr->idx)
            {
                int idx;
                ptr->idx--;
                idx = ptr->idx;
                *ref = &ptr->data[idx * 2];
                if (pthread_mutex_unlock(&mps->cm))
                {
                    ERROR("解锁锁错误");
                    return -1;
                }
                return 0;
            }
            ptr = ptr->next;
        }
        // 没有数据,当前线程等待 条件信号
        if(mps->seal != 0){
            if (pthread_mutex_unlock(&mps->cm))
            {
                ERROR("解锁锁错误");
                return -1;
            }
            return SEALING;
        }
        pthread_cond_wait(&mps->cond, &mps->cm);
        if(mps->seal != 0){
            if (pthread_mutex_unlock(&mps->cm))
            {
                ERROR("解锁锁错误");
                return -1;
            }
            return SEALING;
        }
    }
}
int mutex_put(struct MutexPairs *mps, int64_t p1, int64_t p2)
{
    if (pthread_mutex_lock(&mps->pm))
    {
        ERROR("锁错误");
        return -1;
    }
    if (mps->seal){
        if (pthread_mutex_unlock(&mps->pm))
        {
            ERROR("解锁锁错误");
            return -1;
        }
        return SEALING;
    }
    if (pthread_mutex_lock(&mps->cm))
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
        }
        else
        {
            int idx = ptr->idx * 2;
            ptr->idx++;
            ptr->data[idx] = p1;
            ptr->data[idx + 1] = p2;
            if(pthread_cond_signal(&mps->cond)){
                ERROR("条件信号错误");
                return -1;
            }
            if (pthread_mutex_unlock(&mps->cm))
            {
                ERROR("解锁锁错误");
                return -1;
            }
            if (pthread_mutex_unlock(&mps->pm))
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
    if (pthread_mutex_lock(&mps->cm))
    {
        ERROR("锁错误");
        return -1;
    }
    pairs_print(mps->head, out, limit);
    if (pthread_mutex_unlock(&mps->cm))
    {
        ERROR("解锁锁错误");
        return -1;
    }
    return 0;
}
/**
 * @brief 对象mps不再接受数据，如果对象已经空，mutex_get()不再阻塞，并且返回数据为空
 * @param mps 对象
 * @return 出错返回非零值
*/
int mutex_seal(struct MutexPairs *mps){
    // 只需要生产者锁(pm),
    if (pthread_mutex_lock(&mps->pm))
    {
        ERROR("解锁锁错误");
        return -1;
    }
    mps->seal=1;
    if(pthread_cond_broadcast(&mps->cond)){
        ERROR("条件信号错误");
        return -1;
    }
    if (pthread_mutex_unlock(&mps->pm))
    {
        ERROR("解锁锁错误");
        return -1;
    }
    return 0;
}
// unit test ------------------------------
extern struct timespec MS1;

struct test_struct{
    struct MutexPairs *mp;
    int id;
    int sem;
    pthread_mutex_t *mutex;
};
void *test_pair_child(void *obj)
{
    struct test_struct *ts=obj;
    int res;
    int64_t *ptr;
    pthread_mutex_lock(ts->mutex);
    ts->sem++;
    pthread_mutex_unlock(ts->mutex);
    while(1){
        int64_t left,right,mid;

        pthread_mutex_lock(ts->mutex);
        ts->sem--;
        pthread_mutex_unlock(ts->mutex);

        res=mutex_get(ts->mp,&ptr);
        if(res==SEALING){
            break;
        }
        if(res){
            return (void *)-1;
        }

        pthread_mutex_lock(ts->mutex);
        ts->sem++;
        pthread_mutex_unlock(ts->mutex);

        left=ptr[0];
        right=ptr[1];
        if(left + 1==right){

            continue;
        }

    }

    return NULL;
}
void test_pair(void)
{
    struct MutexPairs *mp=mutex_create();
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    int64_t *ptr;
    pthread_t tid[4];
    mutex_put(mp,0,100000);
    for(int ix=0;ix<3;ix++){
        struct test_struct *ts=malloc(sizeof(struct test_struct));
        ts->id=ix;
        ts->mp=mp;
        ts->mutex=&mutex;
        pthread_create(&tid[ix],NULL,test_pair_child,ts);
    }

    for(int ix=0;ix<20;ix++){
        if(mutex_get(mp,&ptr)) {
            print_error_stack(stdout);
            break;
        }
        printf("%2d:%ld,%ld\n",ix,ptr[0],ptr[1]);
    }
    for(int ix=0;ix<4;ix++){
        void *retval;
        pthread_join(tid[ix],&retval);
        if(retval){
            print_error_stack(stdout);
            break;
        }
    }

}