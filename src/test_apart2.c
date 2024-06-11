#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include "entity.h"

#define TEST_MAX (1024L*1024*512)

extern struct sort_status{
    const char *data;
    const struct ENTITY *ent;
    void *mut;
    void *cond;
    void *jobs;
    void *working;
    int run;

} sorts;

int int_lt(int *p1,int *p2)
{
    return *p1 < *p2;
}
int int_cmp(int *p1,int *p2)
{
    if(*p1<*p2){
        return -1;
    }
    if(*p1==*p2){
        return 0;
    }
    return 1;
}

const struct ENTITY test_apart2_entity={
        0,
    (int             (*)(const void *,const void *))int_lt,
    0,
    0,
    0,
    0,
    sizeof(int)
};

const int xdata[]={2,4,46,7,84,5,8,9,97,67,77,8,100,155,1543,1234,2123,1221,4432,334,4332,321,443};
long apart2(long left,long right);
int test_run_apart2(char *buf,long p1,long p2)
{
    const struct ENTITY *ent=&test_apart2_entity;
    sorts.ent=ent;
    const char *p8;
    sorts.data=buf;
    long mid=apart2(p1,p2);
    wprintf(L"%8ld,%8ld,%8ld\n",p1,mid,p2);
    p8=sorts.data + (mid * ent->unitsize);
    int pivot=*((int *)p8);
    wprintf(L"pivot:%d\n",pivot);
    for(long lx=p1;lx<mid;lx++){
        int v=*((int *)(sorts.data + (lx * ent->unitsize)));
        if(v > pivot){
            wprintf(L"s1 bad value:%d of pos: %ld\n",v,lx);
            return -1;
        }
    }
    for(long lx= mid+1;lx < p2;lx ++){
        int v=*((int *)(sorts.data + (lx * ent->unitsize)));
        if(v < pivot){
            wprintf(L"s2 bad value:%d of pos: %ld\n",v,lx);
            return -1;
        }
    }
    return 0;

}
char *create_int_data(size_t size)
{
    int *pi=malloc(size * sizeof(int));
    if(pi ==NULL){
        return NULL;
    }
    for(long lx=0;lx<size;lx++){
        *(pi+lx)=rand();
    }
    return (char *)pi;
}
int test_apart2(void *na)
{

#if 1
    const long MAX=TEST_MAX;
    char *buf=create_int_data(MAX);
    wprintf(L"生成数据:%ld\n",MAX);
#else
    int cap=sizeof(xdata);
    int *ibuf=malloc(cap);
    cap=cap / ent->unitsize;
    const long MAX=cap;
    for(int ix=0;ix<cap;ix++){
        *(ibuf+ix)=xdata[ix];
    }
    char *buf=(char *)ibuf;
#endif
    int res=test_run_apart2(buf,0,MAX);
    free(buf);
    return res;
}
int is_order(char *data,size_t total, int us,int (*lt)(const void *,const void *));
int test_libsort(void *na)
{
    const int MAX=TEST_MAX;
    char *int_data=create_int_data(MAX);
    wprintf(L"sorting ...\n");
    qsort(int_data,MAX,sizeof(int),(int (*)(const void*,const void*))int_cmp);
    wprintf(L"checking ...\n");
    if(is_order(int_data,MAX,sizeof(int),(int (*)(const void*,const void*))int_lt)){
        return 0;
    }
    return -1;
}