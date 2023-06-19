#include <stdlib.h>
#include <string.h>

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