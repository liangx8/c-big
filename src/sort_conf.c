#include <malloc.h>
#include <stdio.h>
#include <wchar.h>
#include "sort_conf.h"
#include "error_stack.h"
#define BUF_MAX 1024
char linebyte[1024];
int readline(FILE *fh)
{
    int idx=0;
    while(1){
        int ch=fgetc(fh);
        if(ch == EOF){
            if(idx==0){
                return -1;
            }
            return 0;
        }
        if(ch=='\n'){
            linebyte[idx]=ch;
            linebyte[idx+1]='\0';
            return 0;
        }
        linebyte[idx]=ch;
        idx++;
        if(idx==1024){
            return -1;
        }
    }
}
struct CONF *load_config(const char *filename)
{
    FILE *cfg=fopen(filename,"r");
    if(cfg==NULL){
        ERROR_BY_ERRNO();
        return NULL;
    }
    struct CONF *conf;
    conf=malloc(BUF_MAX);
    conf->scope_cnt=0;
    while(!readline(cfg)){
        wprintf(L"%s",linebyte);
    }
    fclose(cfg);
    return conf;
}