#include <wchar.h>
#include <stdio.h>
#include <string.h>
#include "log.h"
struct TEST_STRUCT{
    const char *name;
    int (*test_func)(void);
};

int test_compare(void);

void random_init(void);
long random_long(void);
void random_close(void);
int test_other(void)
{
    random_init();
    for(int ix=0;ix<5;ix++){
        wprintf(L"%d 0x%016lx\n",ix,random_long());
    }
    random_close();
    return 1;
}
int test_sqlite3(void);
const struct TEST_STRUCT all_test[]={
    {"compare",test_compare},
    {"sqlite3",test_sqlite3},
    {"other",test_other}
};

void test_run(const char *test_name)
{
    if(test_name==NULL){
        const int total_test=sizeof(all_test)/16;
        for(int test_idx=0;test_idx<total_test;test_idx++){
            int name_len=strlen(all_test[test_idx].name);
            int space_len=30-name_len;
            int res=all_test[test_idx].test_func();
            wprintf(L"运行测试 \033[1;0;33m%s\033[0m",all_test[test_idx].name);
            for(int ix=0;ix<space_len;ix++){
                putwchar(L' ');
            }
            if(res){
                wprintf(L"\033[0;31m失败\033[0m\n");
            } else {
                wprintf(L"\033[0;36m成功\033[0m\n");
            }
        }
        return;
    }
    int len=strlen(test_name);
    for (int ix=0;ix<3;ix++){
        if(strncmp(all_test[ix].name,test_name,len+1)==0){
            int res=all_test[ix].test_func();
            if(res){
                wprintf(L"运行测试 \033[1;0;33m%s\033[0m \033[0;31m失败\033[0m\n",all_test[ix].name);
            } else {
                wprintf(L"运行测试 \033[1;0;33m%s\033[0m \033[0;36m成功\033[0m\n",all_test[ix].name);
            }
        }
    }
}