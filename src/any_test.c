#include <wchar.h>
#include <stdlib.h>
int any_test(int argc,char *argv[])
{
    if (argc<=1){
        wprintf(L"2:parameter is expected!\n");
        return -1;
    }
    long val=atol(argv[1]);
    wprintf(L"origin:%ld,converted:%ld\n",val,(val + 0x08)&0xfffffffff8);
    return 0;
}