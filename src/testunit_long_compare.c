#include <stdio.h>
#include <wchar.h>

long ulong_lt(const void*,const void*);
long long_cmp(const void*,const void*);
long signed_rand(void);
#define TOTAL_NUM 10
unsigned long lnumber[TOTAL_NUM];
int test_compare(void)
{
#if 1
    for(int ix=0;ix<TOTAL_NUM;ix++){
        lnumber[ix]=signed_rand();
    }
    for(int ix=1;ix<TOTAL_NUM;ix++){
        int bo=0;
        unsigned long l1=lnumber[ix-1];
        unsigned long l2=lnumber[ix];
        long l3=long_cmp(&l1,&l2);
        if(l3){
            if(l3>0)bo=1;else bo=-1;
        }
        wprintf(L" \033[0;33m0x%16lx-0x%16lx = %2d\033[0m\n",l1,l2,bo);
        bo = ulong_lt(&l1,&l2);
        wprintf(L" 0x%16lx<0x%16lx = %2d\n",l1,l2,bo);
    }
#else
    while(1){
        long num=signed_rand();
        wprintf(L"0x%016x %ld\n",num,num);
        if(num<0)break;
    }
#endif
    return 0;
}