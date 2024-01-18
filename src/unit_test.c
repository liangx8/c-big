#include <stdio.h>
#include <string.h>
#include "error_stack.h"
#include "entity.h"


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
int test_signed(const void *pl){
    unsigned int ui1;
    unsigned int ui2;
    int ix=0;
    while(ts_data[ix*2] || ts_data[ix*2+1]){
        ui1=ts_data[ix*2];
        ui2=ts_data[ix*2+1];
        printf("%2d:%11d and %11d: %11d\n",ix,ui1,ui2,qq_cmp(&ui1,&ui2));
        ix++;
    }
    return 0;
}

extern const struct ENTITY qq_entity;

int test_qq_entity(const void *pl);
int test_rand(const void *pl);
const char *const unsorting="/tmp/unsorting.bin";
int mem_sort_test(const void*); // memsort_test.c
int test_qsort_partition(const void *);
const struct test_unit ut_array[]={
    {"mem_sort",mem_sort_test,(void *)100000},
    {"random",test_rand,0},
    {"signed",test_signed,0},
    {"qq_entity",test_qq_entity,0},
    {"qsort_part",test_qsort_partition,&qq_entity},
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

int rand(void); // stdlib.h
int test_rand(const void *pl)
{

    for(int ix=0;ix<10;ix++){
        int yy=rand();
        int zz=yy<<1;
        printf("%10d:%16x,%16x,%11d\n",yy,yy,zz,zz);
    }
    return 0;
}