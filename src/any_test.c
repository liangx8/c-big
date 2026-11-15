#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "strlist.h"
#include "log.h"

void test1(void)
{
    strlist *sl=strlist_create();
    logname="/tmp/test.log";
    char buf[64];
    for(int ix=0;ix<30;ix++){
        snprintf(buf,64,"str-%03d",ix);
        strlist_add(sl,buf);
    }
    strlist_shuffle(sl);
    for(int ix=0;ix<strlist_size(sl);ix++){
        log_info(L"%s\n",strlist_get(sl,ix));
    }
    strlist_destory(sl);
}
int any_test(int argc,char *argv[])
{
    test1();
    return 0;
}