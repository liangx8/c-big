#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include <sys/stat.h>
#include <libgen.h>
#include "werror.h"
#include "app_path.h"

struct APP_PATH *path_info_new(const char *src)
{
    struct APP_PATH *pi=malloc(sizeof(struct APP_PATH));
    pi->preform_src=src;
    return pi;
}
const char *breakinfo_name="breakinfo.json";
void path_info_dst(struct APP_PATH *pi,const char *dst)
{
    char *a_root=strdup(pi->preform_src);
    char *root=dirname(a_root);
    int rootlen=strlen(root);
    if (a_root == root){
        // 当preform_src是一个相对路径时，dirname返回".",这是一个与源不同的地址
        root[rootlen]='/';
        rootlen++;
        root[rootlen]='\0';
    } else {
        root=a_root;
        root[0]='\0';
        rootlen=0;
    }
    int len=rootlen+strlen(dst)+1;
    pi->preform_dst = malloc(len);
    strcpy(pi->preform_dst,root);
    strcat(pi->preform_dst,dst);
    
    len=strlen(breakinfo_name);
    pi->preform_break_info=malloc(rootlen+len+1);
    strcpy(pi->preform_break_info,root);
    strcat(pi->preform_break_info,breakinfo_name);
    pi->dst=dst;
    free(a_root);
}
void path_info_free(struct APP_PATH *pi)
{
    free(pi->preform_dst);
    free(pi->preform_break_info);
    free(pi);
}
void repeat(FILE *out,int ch,int cnt)
{
    char rep[cnt+1];
    for(int ix=0;ix<cnt;ix++){
        rep[ix]=ch;
    }
    rep[cnt]='\0';
    printf("%s\n",rep);
}
off_t filesize(const char *path)
{
    struct stat st;
    if (stat(path, &st))
    {
        return -1;
    }
    else
    {
        return st.st_size;
    }
}

