#include <stdio.h>
#include <wchar.h>
#include <malloc.h>
#include <stdint.h>

struct LONG_BASE{
    int8_t  i1;
    int16_t i2;
    int32_t i4;
    int64_t i8;
};
struct LONG1{
    struct LONG_BASE base;
    int64_t i8;
};
struct STR{
    const wchar_t *x;
};
const wchar_t *hello=L"hello";

int test_struct(const void *none){
    int s1=sizeof(struct LONG_BASE);
    int s2=sizeof(struct LONG1);
    struct LONG_BASE *base=malloc(s2);
    struct LONG1 *l1=(struct LONG1*)base;
    wprintf(L"%d,%d\n%x\n%x\n",s1,s2,&base->i8,&l1->i8);
    struct STR str;
    str.x=hello;
    const wchar_t **ps=(const wchar_t **)&str;
    wprintf(L"[%ls]\n",*ps);
    return 0;
}