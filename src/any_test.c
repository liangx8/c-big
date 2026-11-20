#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "file2strs.h"
#include "log.h"
#include "cfg_data.h"
int test3(int argc,char *argv[])
{
    if(argc==1){
        wprintf(L"请提供设置文件名\n  test <toml_file>\n");
        return -1;
    }
    struct CFG_DATA cfg=load_cfg(argv[1]);
    if(cfg.status==STATUS_OK){
        wprintf(L"wallpaper path:%s\nlog file:%s,count_total:%d\n",cfg.wallpaper_path,cfg.logname,cfg.wallpaper_count_total);
        return 0;
    }
    free_cfg(cfg);
    return -1;
}

int test2_callback(size_t seq,const char *str,void *payload)
{
    wprintf(L"%d:%s\n",seq,str);
    return 0;
}
int sfilter(size_t seq,const char *str)
{
    wprintf(L"%d,%s\n",seq,str);
    return seq-1;
}
void test2(int argc,char *argv[])
{
    logname="/tmp/test.log";
    if(argc==1){
        wprintf(L"测试2\n");
        return;
    }

    struct STRINGS *strs=file_to_strings(argv[1], sfilter);
    if(strs->errmsg){
        wprintf(L"错误:%s\n",strs->errmsg);
    } else {
        strings_iterate(strs,test2_callback,NULL);
    }
    strings_destroy(strs);

}
void test1(void)
{
    logname="/tmp/test.log";
    for(int ix=0;ix<10;ix++){
        log_info(L"%d\n",ix);
    }
}
int any_test(int argc,char *argv[])
{
    //test2(argc,argv);
    //return 0;
    return test3(argc,argv);
}