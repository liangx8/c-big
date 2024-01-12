#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/**
 * @brief 返回路径
 * @return always 0
*/
int full_path(char *buf,const char *str)
{
    char *prefix=getenv("HOME");
    strcpy(buf, prefix);
    strcat(buf, "/");
    strcat(buf, str);
    return 0;
}

off_t filesize(const char*path)
{
    struct stat st;
    if(stat(path,&st)){
        return -1;
    } else {
        return st.st_size;
    }
    
}