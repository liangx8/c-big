#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "strlist.h"
#include "log.h"
#include "tomlc17.h"
int test3(int argc,char *argv[])
{
    if(argc==1){
        wprintf(L"请提供设置文件名\n  test <toml_file>\n");
        return -1;
    }
    toml_result_t result=toml_parse_file_ex(argv[1]);
    if(!result.ok){
        wprintf(L"文件格式错误:%s\n",result.errmsg);
        return -1;
    }
    toml_datum_t aa=toml_seek(result.toptab,"aa");
    toml_datum_t bb=toml_seek(result.toptab,"bb");
    wprintf(L"aa=%s\nbb=%ld\n",aa.u.s,bb.u.int64);
    toml_free(result);
    return 0;
}
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
    //test2(argc,argv);
    return test3(argc,argv);
}