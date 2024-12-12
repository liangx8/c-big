#include <stdio.h>
#include <wchar.h>
#include <malloc.h>


int test_realloc(void)
{
    void *m1=malloc(100);
    wprintf(L"mem1:%08lx\n",(long)m1);
    m1=realloc(m1,1000);
    wprintf(L"mem2:%08lx\n",(long)m1);
    m1=realloc(m1,10);
    wprintf(L"mem3:%08lx\n",(long)m1);
    free(m1);
    return 0;
}
int test_wchar(void);
int test_error(void);
int test_sort_range(void);
int test_main(void)
{
    test_sort_range();
    test_realloc();
    test_wchar();
    test_error();
    return 0;
}