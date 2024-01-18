/**
 * 保存数据对的容器，自动增长，非线程安全,无序
 * 基于对性能的考虑，只用于 int64_t[2] 的对象，其他对太对象不被支持
 */
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <assert.h>
#define EMPTY -2
#define MAX 1000
struct Bag{
    int64_t data[MAX];
    // 以1单位递增，但是数据本身是成对增加,
    int idx;
    
};

struct Bag *bag_create(void)
{
    struct Bag *pa=malloc(sizeof(struct Bag));
    pa->idx=0;
    return pa;
}
struct Bag *bag_with_array(const int64_t *data,int size){
    assert((size & 1)==0);
    assert(size < MAX);
    struct Bag *ps=malloc(sizeof(struct Bag));
    for(int ix=0;ix<size;ix++){
        ps->data[ix]=data[ix];
    }
    ps->idx=size;
    return ps;
}
void bag_free(struct Bag *ps){
    free(ps);
}
/**
 * @brief 获取一个数据
 * @param ps 数据对容器
 * @param ref 如果容器无内容, ref是NULL
 * @return 返回0，获取值成功。1，容器是空
*/
int bag_get(struct Bag *ps,int64_t *ref){
    if(ps->idx){
        int idx=ps->idx-2;
        ps->idx=idx;
        ref[0]=ps->data[idx];
        ref[1]=ps->data[idx+1];
        return 0;
    }
    return EMPTY;
}
int bag_put(struct Bag *ps,int64_t val)
{
    if(ps->idx>=MAX){
        return -1;
    }
    struct Bag *ptr=ps;
    int idx=ptr->idx;
    ptr->idx=idx+1;
    ptr->data[idx]=val;
    return 0;
}
int bag_exist(struct Bag *ps,int64_t val){
    int idx=ps->idx;
    for(int ix=0;ix<idx;ix++){
        if(ps->data[ix]==val){
            return 1;
        }
    }
    return 0;
}
/**
 * @brief put data
 * @return  0 successful, -1 full of container
*/
int bag_put2(struct Bag *ps,int64_t p1,int64_t p2){
    if(ps->idx>=MAX){
        return -1;
    }
    struct Bag *ptr=ps;
    int idx=ptr->idx;
    ptr->idx=idx+2;
    ptr->data[idx]=p1;
    ptr->data[idx+1]=p2;
    return 0;
}
int bag_print(struct Bag *ps,FILE *out,int limit)
{
    limit = limit /2;
    for(int ix=0;ix<limit;ix++){
        if(ix *2 +1 >= ps->idx){
            break;
        }
        int64_t d1=ps->data[ix*2];
        int64_t d2=ps->data[ix*2+1];
        fprintf(out,"[%10ld,%10ld] %10ld\n",d1,d2, d2-d1);
    }
    if(limit * 2 < ps->idx){
        fprintf(out,"...\n");
    }
    fprintf(out,"total: %d\n",ps->idx);
    return 0;
}

/* unit test */
const static int64_t base[]={0,200,200,400,400,1000};
void bag_unit_test(void)
{
    int64_t data[2];
    struct Bag *pps=bag_with_array(base,6);
    bag_print(pps,stdout,20);
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
        printf("%4d:left:%4ld,mid:%4ld,right:%4ld \n",seq++,left,mid,right);
        if(right-mid > 3){
            bag_put2(pps,mid+1,right);
        }
    }
    bag_print(pps,stdout,20);
}