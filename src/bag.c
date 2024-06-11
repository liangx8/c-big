/**
 * 保存数据对的容器，自动增长，非线程安全,无序
 * 基于对性能的考虑，只用于 int64_t[2] 的对象，其他对太对象不被支持
 */
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <wchar.h>
#include "werror.h"
#define EMPTY -2
#define MAX 1000
struct Bag{
    int64_t data[MAX];
    // 以1单位递增，但是数据本身是成对增加,
    int idx;
    
};
int bag_num(struct Bag *b)
{
    return b->idx;
}

struct Bag *bag_with_array(const int64_t *data,int size){
    if(size > MAX){
        return NULL;
    }
    struct Bag *ps=malloc(sizeof(struct Bag));
    if(size){
        for(int ix=0;ix<size;ix++){
            ps->data[ix]=data[ix];
        }
        ps->idx=size/2;
    } else {
        ps->idx=0;
    }
    return ps;
}
void bag_free(struct Bag *ps){
    free(ps);
}
/**
 * @brief 获取一个数据
 * @param ps 数据对容器
 * @param ref 如果容器无内容, ref保持原来的内容
 * @return 返回0，获取值成功。返回非0，容器是空
*/
int bag_get(struct Bag *ps,int64_t *ref){
    if(ps->idx){
        int idx=ps->idx-1;
        ps->idx=idx;
        idx=idx * 2;
        ref[0]=ps->data[idx];
        ref[1]=ps->data[idx+1];
        return 0;
    }
    return EMPTY;
}

/**
 * @brief put data
 * @return  0 successful, -1 full of container
*/
int bag_put2(struct Bag *ps,int64_t p1,int64_t p2){
    if((ps->idx*2)>=MAX){
        WERROR(L"增加最大值%ls",L"!");
        return -1;
    }
    int idx=ps->idx;
    ps->idx=idx+1;
    idx = idx *2;
    ps->data[idx]=p1;
    ps->data[idx+1]=p2;
    return 0;
}
/**
 * @brief 取得全部的数据
 * @param ps
 * @param dst
 * @return 返回dst的数量,-1 错误
*/
int bag_array(struct Bag *ps,long **dst){
    if(ps->idx){
        long total=sizeof(long) * ps->idx*2;
        int64_t *ary=malloc(total);
        if(ary == NULL){
            WERR_NO_MSG(errno,L"malloc() 错误%ld",total);
            return -1;
        }
        memcpy(ary,ps->data,total);
        *dst=ary;
        return ps->idx*2;
    } else {
        return 0;
    }

}
int bag_print(struct Bag *ps,FILE *out,int limit)
{
    limit = limit ;
    for(int ix=0;ix<limit;ix++){
        if(ix >= ps->idx){
            break;
        }
        int64_t d1=ps->data[ix*2];
        int64_t d2=ps->data[ix*2+1];
        fwprintf(out,L"[%10ld,%10ld] %10ld\n",d1,d2, d2-d1);
    }
    if(limit < ps->idx){
        fwprintf(out,L"...\n");
    }
    fwprintf(out,L"total: %d\n",ps->idx);
    return 0;
}

/* unit test */
const static int64_t base[]={0,200,200,400,400,1000,50,100,600,800};
int bag_unit_test(void *unused)
{
    int64_t data[2];
    struct Bag *pps=bag_with_array(base,10);
    bag_print(pps,stdout,4);
    int seq=0;
    while(1){
        if(bag_get(pps,&data[0])){
            break;
        }
        int64_t left=data[0];
        int64_t right=data[1];
        int64_t mid=(left+right)/2;
        if(mid-left > 3){
            bag_put2(pps,left,mid);
        }
        wprintf(L"%4d:left:%4ld,mid:%4ld,right:%4ld \n",seq++,left,mid,right);
        if(right-mid > 3){
            bag_put2(pps,mid+1,right);
        }
        if(seq>280){
            break;
        }
    }
    bag_print(pps,stdout,20);
    long *ary;
    int size=bag_array(pps,&ary);
    for(int ix=0;ix<size;ix++){
        wprintf(L"%4ld ",ary[ix]);
        if((ix % 6)==5){
            wprintf(L"\n");
        }
    }
    free(ary);
    wprintf(L"\n");
    return 0;
}