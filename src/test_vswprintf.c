#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>

wchar_t buf[2048];

void vari(const wchar_t* prefix,const wchar_t* fmt, ...)
{
    va_list vl;
    va_start(vl,fmt);
    int num=vswprintf(buf,2048,fmt,vl);
    va_end(vl);
    wprintf(L"长度:%d\n",num);
}

void test_wchar(void)
{
    vari(L"前缀",L"格式");
}