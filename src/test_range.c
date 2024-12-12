#include <stdio.h>
#include <wchar.h>
#include "sort_range.h"

int test_sort_range(void)
{
    struct RANGES *rgs=rng_new();
    long l1,l2;
    rng_push(rgs,1,2);
    int ret=rng_pop(rgs,&l1,&l2);
    wprintf(L"pop:%d:%ld,%ld\n",ret,l1,l2);
    ret=rng_pop(rgs,NULL,NULL);
    wprintf(L"pop:%d:%ld,%ld\n",ret,l1,l2);
    return 0;
}