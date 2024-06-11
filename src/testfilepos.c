#include <stdio.h>
#include <wchar.h>

const char ch[]={0,1,2,3,4,5,6,7};
int testfilepos(const void *dy)
{
    FILE *af=fopen("/tmp/a.bin","w+");
    fwrite(&ch[0],8,1,af);
    fseek(af,0,SEEK_END);
    wprintf(L"%d\n",ftell(af));
    return 0;
}