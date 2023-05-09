/**
 * 保存数据对的容器，自动增长，非线程安全,无序
 * 
 */
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <assert.h>
#define MAX 1000
struct Pairs{
    int64_t data[MAX*2];
    struct Pairs *next;
    // 以1单位递增，但是数据本身是成对增加,
    uint32_t idx;
    uint32_t unused;
};

struct Pairs *pairs_create(void)
{
    struct Pairs *pa=malloc(sizeof(struct Pairs));
    pa->idx=0;
    pa->next=NULL;
    return pa;
}
struct Pairs *pairs_with_array(const int64_t *data,int size){
    assert((size & 1)==0);
    struct Pairs *ps=pairs_create();
    for (int xx=0;xx<size;xx++){
        ps->data[xx]=data[xx];
    }
    ps->idx=size >> 1;
    return ps;
}
void pairs_free(struct Pairs *ps){
    struct Pairs *ptr=ps;
    while(ptr){
        struct Pairs *old=ptr;
        ptr=old->next;
        free(old);
    }
}
int pairs_is_empty(struct Pairs *ps){
    struct Pairs *p=ps;
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
 * @return 永远返回0
*/
int pairs_get(struct Pairs *ps,int64_t **ref){
    struct Pairs *ptr=ps;
    while(ptr){
        if(ptr->idx){
            int idx;
            ptr->idx --;
            idx=ptr->idx;
            *ref=&ptr->data[idx*2];
            return 0;
        }
        ptr=ptr->next;
    }
    *ref=NULL;
    return 0;
}
int pairs_put(struct Pairs *ps,int64_t p1,int64_t p2){
    struct Pairs *ptr=ps;
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
            return 0;
        }
    }
    return 0;
}
int pairs_print(struct Pairs *ps,FILE *out,int limit)
{
    uint32_t total=0;
    int cnt=0;
    int print=1;
    while(ps){
        total+=ps->idx;
        if (print){
            for(int ix=0;ix<ps->idx;ix++){
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