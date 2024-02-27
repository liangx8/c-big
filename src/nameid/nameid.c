#include <stdint.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "../status.h"
#include "../options.h"
#include "../error_stack.h"
#include "nameid.h"


const char *srcname="/tmp/nameid.bin";
const wchar_t *wtest=L"零壹贰叁肆务陆柒捌玖";

int app_nameid(struct STATUS *stu,struct OPTION *opt)
{
    struct NAMEID *ptr=new_nameid(srcname);
    if (ptr == NULL){
        return -1;
    }
    if (ptr!=NULL){
        close_nameid(ptr);
    }
    return 0;
}