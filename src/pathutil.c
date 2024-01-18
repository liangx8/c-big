#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include "error_stack.h"

/**
 * @brief 返回路径
 * @return always 0
 */
int full_path(char *buf, const char *str)
{
    char *prefix = getenv("HOME");
    strcpy(buf, prefix);
    strcat(buf, "/");
    strcat(buf, str);
    return 0;
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
size_t copy(const char *dst, const char *src, size_t size)
{
    FILE *fsrc=fopen(src,"r");
    if(fsrc==NULL){
        ERROR_BY_ERRNO();
        return 0;
    }
    FILE *fdst=fopen(dst,"w+");
    if(fdst==NULL){
        ERROR_BY_ERRNO();
        fclose(fsrc);
        return 0;
    }
    size_t cnt=0;
    while(1){
        if(cnt==size){
            break;
        }
        int c=fgetc(fsrc);
        if(c==EOF){
            break;
        }
        fputc(c,fdst);
        cnt++;
    }
    fclose(fsrc);
    fclose(fdst);
    return cnt;
}
