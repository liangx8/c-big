#include <stdlib.h>
#include <stdio.h>
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
int qq_cmp(const void *,const void *);
void qq_print(const void *,long ,const char*);
void test_print_all(const void *ptr)
{
    for(int ix=0;ix<ITOTAL;ix++){
        qq_print(ptr+(ix*12),ix,NULL);
    }
}
int test_qq_qsort()
{
    unsigned char *buf=randomdata(ITOTAL);
    test_print_all(buf);
    qsort(buf,ITOTAL,12,qq_cmp);
    printf("====================================\n");
    test_print_all(buf);
    free(buf);
    return 0;
}
int test_qsort(const void *pl)
{
    return test_qq_qsort();    
}
