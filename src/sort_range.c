#include <malloc.h>
#include "error_stack.h"
#define MAX 256
/**
 * @brief 储存需要排序的范围
 */
struct RANGES{
    long idx;
    struct RANGES *next;
    long data[MAX];
};

struct RANGES *rng_new(void)
{
    struct RANGES *rgs= malloc(sizeof(struct RANGES));
    rgs->idx=0;
    rgs->next=NULL;
    return rgs;
}
void rng_push(struct RANGES *rgs,long l1,long l2)
{
    long idx=rgs->idx*2;
    if(idx < MAX){
        rgs->data[idx]=l1;
        rgs->data[idx+1]=l2;
        rgs->idx++;
        return;
    }
    struct RANGES *next= malloc(sizeof(struct RANGES));
    if(next==NULL){
        FATAL(L"内存不够了");
    }
    rgs->next=next;
    next->data[0]=l1;
    next->data[1]=l2;
    next->idx=1;
    next->next=NULL;
    return;
}
/**
 * @brief pop a pair
 * @param rgs target
 * @param l1 第一个
 * @param l2 第二个
 * @return 返回到值是对象中还有多少，如果对象中没有，返回小于０
 */
int rng_pop(struct RANGES *rgs,long *l1,long *l2)
{
    struct RANGES *ptr=rgs;
    struct RANGES *parent=NULL;
    if(ptr->idx==0){
        return -1;
    }
    int cnt=0;
    while(ptr->next){
        cnt += MAX;
        parent=ptr;
        ptr=ptr->next;
    }
    long idx=ptr->idx-1;
    long idx2=idx*2;
    ptr->idx=idx;
    *l1=ptr->data[idx2];
    *l2=ptr->data[idx2+1];
    if(idx==0){
        if (parent){
            free(ptr);
            parent->next=NULL;
        }
    }

    return cnt+idx;
}
int rng_release(struct RANGES *rgs)
{
    struct RANGES *ptr;
    while(rgs){
        ptr=rgs->next;
        free(rgs);
        rgs=ptr;
    }
    return 0;
}
int rng_print(struct RANGES *rgs,int limit)
{
    int cnt=0;
    while(rgs){
        long *ptr=rgs->data;
        for(int ix=0;ix<rgs->idx;ix++){
            wprintf(L"(%9ld,%9ld)\n",*ptr,*(ptr+1));
            ptr ++;
            ptr ++;
            cnt++;
            if(cnt>=limit){
                return 0;
            }
        }
        rgs=rgs->next;
    }
    return 0;
}
