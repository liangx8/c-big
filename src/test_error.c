#include <stdio.h>
#include "error_stack.h"


int test_error(void)
{
    error_init();
    ERROR(L"出错了１");
    ERRORV(L"出错了２:%d",12);
    print_error_stack(stdout);
    error_release();
    return 0;
}