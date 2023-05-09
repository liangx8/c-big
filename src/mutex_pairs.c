#include <stdint.h>
#include <malloc.h>
#include <pthread.h>
#include <assert.h>
#include "error_stack.h"
#define MAX 1000

struct Pairs{
    int64_t         data[MAX*2];
    struct Pairs    *next;
    // 以1单位递增，但是数据本身是成对增加,
    uint32_t        idx;
    uint32_t        unused;
};

struct MutexPairs{
    pthread_mutex_t *mutex;
    struct Pairs    *head;
};

struct MutexPairs *mutex_create(void)
{
    struct MutexPairs *pa=malloc(sizeof(struct MutexPairs));
    pa->head=NULL;
    pthread_mutex_t *mutex=malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex,NULL);
    pa->mutex=mutex;
    return pa;
}
struct MutexPairs *mutex_with_array(pthread_mutex_t *mutex,const int64_t *data,int size){
    assert((size & 1)==0);
    struct MutexPairs *mp=mutex_create();
    mp->head=malloc(sizeof(struct Pairs));
    struct Pairs *ptr=mp->head;

    while(size>=0){
        int sec;
        if(size<MAX){
            sec=size;
        } else {
            sec=MAX;
        }
        size=size-MAX;
        for(int ix=0;ix<sec;ix++){
            ptr->data[ix]=data[ix];
            ptr->idx=sec;
        }
        if(size>=0){
            ptr->next=malloc(sizeof(struct Pairs));
            ptr=ptr->next;
        } else {
            break;
        }
    }
    return mp;
}
void pairs_free(struct Pairs *);
void mutex_free(struct MutexPairs *ps){
    pthread_mutex_t *mutex=ps->mutex;
    pairs_free(ps->head);
    free(ps);
    pthread_mutex_destroy(mutex);
}
int mutex_is_empty(struct MutexPairs *mps){
    if(pthread_mutex_lock(mps->mutex)){
        ERROR("锁错误");
        return -1;
    }
    struct Pairs *p=mps->head;
    while(p){
        if(p->idx){
            if(pthread_mutex_unlock(mps->mutex)){
                ERROR("解锁锁错误");
                return -1;
            }
            return 0;
        }
        p=p->next;
    }
    if(pthread_mutex_unlock(mps->mutex)){
        ERROR("解锁锁错误");
        return -1;
    }
    return 1;
}
/**
 * @brief 获取一个数据
 * @param ps 数据对容器
 * @param ref 如果容器无内容, ref是NULL
 * @return 出错返回-1，否则 0
*/
int mutex_get(struct MutexPairs *mps,int64_t **ref){
    if(pthread_mutex_lock(mps->mutex)){
        ERROR("锁错误");
        return -1;
    }

    struct Pairs *ptr=mps->head;
    while(ptr){
        if(ptr->idx){
            int idx;
            ptr->idx --;
            idx=ptr->idx;
            if(pthread_mutex_unlock(mps->mutex)){
                ERROR("解锁锁错误");
                return -1;
            }
            *ref= &ptr->data[idx*2];
            return 0;
        }
        ptr=ptr->next;
    }
    if(pthread_mutex_unlock(mps->mutex)){
        ERROR("解锁锁错误");
        return -1;
    }
    *ref=NULL;
    return 0;
}
int mutex_put(struct MutexPairs *mps,int64_t p1,int64_t p2){
    if(pthread_mutex_lock(mps->mutex)){
        ERROR("锁错误");
        return -1;
    }
    struct Pairs *ptr=mps->head;
    while(1){
        if(ptr->idx == MAX){
            if(ptr->next==NULL){
                ptr->next=malloc(sizeof(struct Pairs));
            }
            ptr=ptr->next;
        } else {
            int idx=ptr->idx*2;
            ptr->idx++;
            ptr->data[idx]=p1;
            ptr->data[idx+1]=p2;
            if(pthread_mutex_unlock(mps->mutex)){
                ERROR("解锁锁错误");
                return -1;
            }
            return 0;
        }
    }
    if(pthread_mutex_unlock(mps->mutex)){
        ERROR("解锁锁错误");
        return -1;
    }
    return 0;
}
void pairs_print(struct Pairs *ps,FILE *out,int limit);
int mutex_print(struct MutexPairs *mps,FILE *out,int limit)
{
    if(pthread_mutex_lock(mps->mutex)){
        ERROR("锁错误");
        return -1;
    }
    pairs_print(mps->head,out,limit);
    if(pthread_mutex_unlock(mps->mutex)){
        ERROR("解锁锁错误");
        return -1;
    }
    return 0;
}