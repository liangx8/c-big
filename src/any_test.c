#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "strlist.h"
#include "log.h"
void test2(int argc,char *argv[])
{
    logname="/tmp/test.log";
    if(argc==1){
        wprintf(L"测试2\n");
        return;
    }
    FILE *txt=fopen(argv[1],"r");
    if(txt==NULL){
        wprintf(L"打开文件错误%s\n",argv[1]);
        return;
    }
    strlist *list=all_line(txt);
    fclose(txt);
    if(list==NULL){
        wprintf(L"读文件错误%s",argv[1]);
        return;
    }
    for(int ix=0;ix<strlist_size(list);ix++){
        const char *val=strlist_get(list,ix);
        wprintf(L"%s\n",val);
        log_info(L"%s\n",val);
    }
    strlist_destory(list);

}
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
    test2(argc,argv);
    return 0;
}