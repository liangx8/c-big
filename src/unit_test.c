#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <malloc.h>
#include <unistd.h>


#include "timestamp.h"
#include "werror.h"
#include "entity.h"
int rand(void); // stdlib.h

struct test_unit{
    const char *name;
    int (*test)(const void *);
};


int qq_cmp(const void *,const void *);
const unsigned int ts_data[]={
    100,200,
    100,100,
    0xfffffffe,0xffffffff,
    0xffffff00,0xfffffe00,
    0,0
};

void print_long(long val){
    wprintf(L"long value: %ld,%lx\n",val,val);
}

void werror_no_v(const wchar_t *,int ,int ,const wchar_t *, ...);
void werror_print(void);
void werror_init(void);

const wchar_t test[]={L'a',L'b',L'c',0};
int test_work(const void *pl){
    char *buf=malloc(64);
    werror_init();
    const char *fmt="number:%d";
    for(int ix=0;ix<4;ix++){
        int val=rand();
        int num=snprintf(NULL,0,fmt,val);
        snprintf(buf,num+1,fmt,val);
        wprintf(L"%d %s\n",num,buf);
    }
    wprintf(L"wchar_t:%d,wchar_t[2]:%d,%d,%ls\n",sizeof(wchar_t),sizeof(wchar_t[2]),sizeof(test),test);
    
    werror_no_v(L"unit_test.c",43,1,L"错误码%d",1);
    for (int ix=2;ix<18;ix++){
        werror_no_v(L"unit_test.c",45,ix,L"错误码%d",ix);
    }
    werror_print();
    free(buf);
    uint32_t u32=100;
    print_long(-(long)u32);

    return 0;
}

int test_rand(const void *pl)
{

    for(int ix=0;ix<10;ix++){
        int yy=rand();
        int zz=yy<<1;
        wprintf(L"%10d:%16x,%16x,%11d\n",yy,yy,zz,zz);
    }
    return 0;
}

int test_struct(const void *);

extern const struct ENTITY qq_entity;

int mem_sort_test(const void*); // memsort_test.c
int test_qsort_partition(const void *);
int testfilepos(const void *);
int test_path(const void *);
int bag_unit_test(const void *);
int test_thread(const void *);
int test_apart2(const void *);
int test_libsort(const void *);
int test_wstr(const void *);
int meaningless(const void *);
const struct test_unit ut_array[]={
    {"rand",test_rand},
    {"work",test_work},
    {"struct",test_struct},
    {"filepos",testfilepos},
    {"path",test_path},
    {"bag",bag_unit_test},
    {"thread",test_thread},
    {"apart2",test_apart2},
    {"libsort",test_libsort},
    {"wstr",test_wstr},
    {"ml",meaningless},
    {0,0}
};


void unit_run(const char *name,const void *opt)
{
    int ix=0;
    int notfound;
    if(name){
        notfound=1;
    } else {
        notfound=0;
    }
    char tmstr[64];
    while(ut_array[ix].test){
        if(name){
            // 比较name是否与ut_array[ix].name相同
            if(strcmp(name,ut_array[ix].name)){
                ix++;
                continue;
            }
            notfound=0;
        }
        long start=now();
        if(ut_array[ix].test(opt)){
            wprintf(L"run test %s \033[0;31mfailure\033[0m\n",ut_array[ix].name);
            werror_print();
        } else {
            wprintf(L"run test %s \033[0;35msuccess\033[0m\n",ut_array[ix].name);
        }
        long end=now();
        timestamp_str(tmstr, end-start);
        wprintf(L"test run in %s\n",tmstr);

        ix++;
    }
    if(notfound){
        wprintf(L"there is no test unit \033[0;32m%s\033[0m exists\n",name);
    }
}


void hex(char *buf,int total)
{
    printf("total %d(%x)\n",total,total);
    printf("                 ");
    for (int ix=0;ix<16;ix++){
        printf("%02x ",ix);
        if (ix == 7){
            printf("| ");
        }
    }
    printf("\n");
    for (int ix=0;ix<total;ix++){
        if ((ix & 0xf) ==0){
            printf("%016lx ",(long)(buf +ix));
        }
        if (((ix-1) & 0xf)==7){
            printf("| ");
        }
        printf("%02x ",(0xff & buf[ix]));
        if ((ix & 0xf) ==0xf){
            printf("\n");
        }
    }
    printf("\n");
}