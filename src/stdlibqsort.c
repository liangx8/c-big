#include <stdlib.h>
#include <stdio.h>
#include "entity.h"
#define ITOTAL 10



int icmp(const void *p1,const void *p2)
{
    const int *i1=(const int *)p1;
    const int *i2=(const int *)p2;

    return *i1-*i2;
}
int int_test(void)
{
    int *im=malloc(sizeof(int)*ITOTAL);
    for(int ix=0;ix<ITOTAL;ix++){
        *(im+ix)=rand();
    }
    qsort(im,ITOTAL,sizeof(int),icmp);
    for(int ix=0;ix<ITOTAL;ix++){
        printf("%8d\n",*(im+ix));
    }
    free(im);
    return 0;
}
unsigned char *randomdata(long total);

extern const struct ENTITY qq_entity;


int test_qq_qsort()
{
    unsigned char *buf=randomdata(ITOTAL);
    qsort(buf,ITOTAL,qq_entity.unitsize,qq_entity.cmp);
    free(buf);
    return 0;
}
int test_qsort(const void *pl)
{
    return test_qq_qsort();    
}
