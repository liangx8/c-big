#include <stdio.h>
#include <wchar.h>
#include <libgen.h>
#include <malloc.h>
#include <string.h>
const char *file="aaaa";
int test_path(const void *a)
{
    char *src=strdup(file);
    wprintf(L"%lx %s\n",(long)src,src);
    char *ptr=dirname(src);
    wprintf(L"%lx dirname:%s\n",(long)ptr,ptr);
    strcpy(src,file);
    ptr=basename(src);

    wprintf(L"%lx basename:%s\n",(long)ptr,ptr);
    wprintf(L"src:%s\n",src);
    free(src);
    return 0;
}