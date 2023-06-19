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
    struct Bag *next;
    // 以1单位递增，但是数据本身是成对增加,
    uint32_t idx;
    uint32_t unused;
};

struct Bag *bag_create(void)
{
    struct Bag *pa=malloc(sizeof(struct Bag));
    pa->idx=0;
    pa->next=NULL;
    return pa;
}
struct Bag *bag_with_array(const int64_t *data,int size){
    assert((size & 1)==0);
    int si;
    const int64_t *sptr=data;
    struct Bag *ps=malloc(sizeof(struct Bag));
    struct Bag *head=ps;
    while(size){
        struct Bag *cur=ps;
        if(size>MAX){
            si=MAX;
            size=size-MAX;
            ps->next=malloc(sizeof(struct Bag));
            ps=ps->next;
            ps->next=NULL;
        } else {
            si=size;
            size=0;
        }
        cur->idx=si;
        for(int ix=0;ix<si;ix++){
            cur->data[ix]=*sptr;
            sptr++;
        }
    }
    return head;
}
void bag_free(struct Bag *ps){
    struct Bag *ptr=ps;
    while(ptr){
        struct Bag *old=ptr;
        ptr=old->next;
        free(old);
    }
}
int bag_is_empty(struct Bag *ps){
    struct Bag *p=ps;
    while(p){
        if(p->idx){
            return 0;
        }
        p=p->next;
    }
    return 1;
}
/**
 * @brief 获取一个数据
 * @param ps 数据对容器
 * @param ref 如果容器无内容, ref是NULL
 * @return 返回0，获取值成功。1，容器是空
*/
int bag_get(struct Bag *ps,int64_t **ref){
    struct Bag *ptr=ps;
    while(ptr){
        assert(ptr->idx >= 0);
        if(ptr->idx){
            int idx=ptr->idx-2;
            ptr->idx=idx;
            *ref=&ptr->data[idx];
            return 0;
        }
        ptr=ptr->next;
    }
    *ref=NULL;
    return EMPTY;
}
int bag_put(struct Bag *ps,int64_t p1,int64_t p2){
    struct Bag *ptr=ps;
    while(1){
        if(ptr->idx == MAX){
            if(ptr->next==NULL){
                ptr->next=malloc(sizeof(struct Bag));
                ptr->next->idx=0;
                ptr->next->next=NULL;
            }
            ptr=ptr->next;
        } else {
            int idx=ptr->idx;
            ptr->idx=idx+2;
            ptr->data[idx]=p1;
            ptr->data[idx+1]=p2;
            return 0;
        }
    }
    return 0;
}
int bag_print(struct Bag *ps,FILE *out,int limit)
{
    uint32_t total=0;
    int cnt=0;
    int print=1;
    limit = limit /2;
    while(ps){
        total+=ps->idx;
        uint32_t half=ps->idx/2;
        for(int ix=0;ix<half;ix++){
            if (print){
                if(cnt < limit){
                    cnt++;
                    fprintf(out,"[%10ld,%10ld]\n",ps->data[ix*2],ps->data[ix*2+1]);
                } else {
                    fprintf(out,"...\n");
                    print=0;
                }
            }
        }
        ps=ps->next;
    }
    fprintf(out,"total: %d\n",total);
    return 0;
}

/* unit test */
const static int64_t base[]={0,200,200,400,400,1000};
void bag_unit_test(void)
{
    int64_t *data;
    struct Bag *pps=bag_with_array(base,6);
    bag_print(pps,stdout,20);
    int seq=0;
    while(1){
        if(bag_get(pps,&data)){
            break;
        }
        int64_t left=data[0];
        int64_t right=data[1];
        int64_t mid=(left+right)/2;
        if(mid-left > 3){
            bag_put(pps,left,mid);
        }
        printf("%4d:left:%4ld,mid:%4ld,right:%4ld \n",seq++,left,mid,right);
        if(right-mid > 3){
            bag_put(pps,mid+1,right);
        }
    }
    bag_print(pps,stdout,20);
}