#include <stdio.h>
#include <wchar.h>
#include <sys/stat.h>
#include <errno.h>

/**
 * @brief 检查或者建立目录
 */
int touch_dir(const char *dirname)
{
    struct stat stb;
    if(lstat(dirname,&stb)){
        if(errno==ENOENT){
            if(mkdir(dirname,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH)){
                wprintf(L"建立目录错误:%s\n",dirname);
            }
        }else {
            wprintf(L"error code:%d\n",errno);
            return -1;
        }
    }
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