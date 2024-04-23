#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "error_stack.h"
#include "entity.h"

int rand(void); // stdlib.h

struct test_unit{
    const char *name;
    int (*test)(const void *);
    const void *payload;
};


int qq_cmp(const void *,const void *);
const unsigned int ts_data[]={
    100,200,
    100,100,
    0xfffffffe,0xffffffff,
    0xffffff00,0xfffffe00,
    0,0
};
int test_work(const void *pl){
    char *buf=malloc(64);
    const char *fmt="number:%d";
    for(int ix=0;ix<4;ix++){
        int val=rand();
        int num=snprintf(NULL,0,fmt,val);
        snprintf(buf,num+1,fmt,val);
        printf("%d %s\n",num,buf);
    }
    ERRORV("输出文本:%d %d (%s)\n",1,2,"abc");
    print_error_stack(stdout);
    return 0;
}

extern const struct ENTITY qq_entity;


int test_partition(const void *);
int test_qq_entity(const void *pl);
int test_rand(const void *pl);
int test_strs(const void *pl);
const char *const unsorting="/tmp/unsorting.bin";
int mem_sort_test(const void*); // memsort_test.c
int test_qsort_partition(const void *);
const struct test_unit ut_array[]={
    {"mem_sort",mem_sort_test,(void *)100000},
    {"random",test_rand,0},
    {"work",test_work,0},
    {"qq_entity",test_qq_entity,0},
    {"qsort_part",test_qsort_partition,&qq_entity},
    {"part89",test_partition,&qq_entity},
    {"strs",test_strs,0},
    {0,0,0}
};


void unit_run(const char *name)
{
    int ix=0;
    int notfound;
    if(name){
        notfound=1;
    } else {
        notfound=0;
    }
    while(ut_array[ix].test){
        if(name){
            // 比较name是否与ut_array[ix].name相同
            if(strcmp(name,ut_array[ix].name)){
                ix++;
                continue;
            }
            notfound=0;
        }
        if(ut_array[ix].test(ut_array[ix].payload)){
            printf("run test %s \033[0;31mfailure\033[0m\n",ut_array[ix].name);
            print_error_stack(stderr);
        } else {
            printf("run test %s \033[0;35msuccess\033[0m\n",ut_array[ix].name);
        }
        ix++;
    }
    if(notfound){
        printf("there is no test unit \033[0;32m%s\033[0m exists\n",name);
    }
}

int test_rand(const void *pl)
{

    for(int ix=0;ix<10;ix++){
        int yy=rand();
        int zz=yy<<1;
        printf("%10d:%16x,%16x,%11d\n",yy,yy,zz,zz);
    }
    return 0;
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