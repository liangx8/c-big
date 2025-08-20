#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include "log.h"
#define MAX 2048
/**
 * @brief 储存需要排序的范围
 */
struct RANGES{
    long idx;
    uint64_t data[MAX+MAX];
};

struct RANGES *rng_new(void)
{
    struct RANGES *rgs= malloc(sizeof(struct RANGES));
    rgs->idx=0;
    return rgs;
}
/**
 * @brief 入栈一对
 * @return -1到达入栈上限　0成功
 */
int rng_push(struct RANGES *rgs,uint64_t l1,uint64_t l2)
{
    long idx=rgs->idx*2;
    if(idx < MAX){
        rgs->data[idx]=l1;
        rgs->data[idx+1]=l2;
        rgs->idx++;
        return 0;
    }
    log_info(L"RANGES 超过%d\n",MAX);
    return -1;
}
/**
 * @brief pop a pair
 * @param rgs target
 * @param l1 第一个
 * @param l2 第二个
 * @return 0抽取成功，-1数据已经空
 */
int rng_pop(struct RANGES *rgs,uint64_t *l1,uint64_t *l2)
{

    long idx=rgs->idx;
    long idx2;
    if(idx==0){
        return -1;
    }
    idx--;
    rgs->idx=idx;
    idx2=idx*2;
    *l1=rgs->data[idx2];
    *l2=rgs->data[idx2+1];
    return 0;
}
void rng_release(struct RANGES *rgs)
{
    free(rgs);
    return;
}
void rng_print(struct RANGES *rgs,int limit)
{
    int cnt=0;
    uint64_t *ptr=rgs->data;
    wprintf(L"head----------------------\n");
    for(int ix=0;ix<rgs->idx;ix++){
        wprintf(L"(%9ld,%9ld)\n",*ptr,*(ptr+1));
        ptr ++;
        ptr ++;
        cnt++;
        if(cnt>=limit){
            wprintf(L"...\n");
            break;
        }
    }
    wprintf(L"total: %ld\n",rgs->idx);
}
