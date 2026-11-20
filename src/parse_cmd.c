// #include <unistd.h>
// #include <stdio.h>
#include "parse_cmd.h"
#include "cfg_data.h"
#include <string.h>
#include <libgen.h>
#include <wchar.h>
#include <unistd.h> // optarg
#include <stdlib.h>




#define CFG_DEFAULT_NAME ".config/waybar/waybar.ini"


// 定义自动清理函数（参数必须是指针的指针）
void auto_free(void *p) {
    struct CFG_DATA *ptr=p;
    free_cfg(*ptr);
}

// 封装成宏方便使用
#define AUTO_FREE __attribute__((cleanup(auto_free)))

// void func() {
//     AUTO_FREE char *buf = (char *)malloc(100);
//     // 无论函数从哪里 return，离开 func 作用域时都会自动调用 auto_free(&buf)
// }



char* get_configname(int argc,char *argv[])
{
    int opt;
    while((opt=getopt(argc,argv,"c:"))!=-1){
        if(opt=='c'){
            return strdup(optarg);
        }
    }
    const char *dirname=getenv("HOME");
    size_t dirname_length = strlen(dirname);
    size_t name_length = sizeof(CFG_DEFAULT_NAME);
    int needs_separator = dirname_length > 0 && dirname[dirname_length - 1] != '/';


    size_t length = dirname_length + name_length + (size_t)needs_separator;
    char *path = malloc(length);
    if (!path) {
        return NULL;
    }

    memcpy(path, dirname, dirname_length);
    size_t offset = dirname_length;
    if (needs_separator) {
        path[offset++] = '/';
    }
    memcpy(path + offset, CFG_DEFAULT_NAME, name_length);
    return path;
}
int parse_cmd(int argc, char *argv[]) {
    char *cfgname=get_configname(argc,argv);
    wprintf(L"config file:%s\n",cfgname);
    AUTO_FREE struct CFG_DATA cfg=load_cfg(cfgname);
    if(cfg.status==STATUS_BAD){

    }
    free(cfgname);
    char *cmd_name = argv[0];
    for(int i = 0; wb_cmds[i].name != NULL; i++) {
        if(strcmp(basename(cmd_name), wb_cmds[i].name) == 0) {
            return wb_cmds[i].action(argc, argv);
        }
    }
    if(argc == 1) {
        wprintf(L"Unknown command: %s\n", cmd_name);
        return -1;
    }
    cmd_name=argv[1];
    for(int i = 0; wb_cmds[i].name != NULL; i++) {
        if(strcmp(cmd_name, wb_cmds[i].name) == 0) {
            return wb_cmds[i].action(argc-1, &argv[1]);
        }
    }
    wprintf(L"Unknown command: %s\n", cmd_name);
    return -1;
}