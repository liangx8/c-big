#include <stdio.h>
#include <wchar.h>
#include <malloc.h>
#include "buffer.h"
#include "werror.h"
#include "options.h"
#ifndef NULL
#define NULL (void *)0
#endif

void * new_wnameid(const char *);
int wnid_close(void *);
long wnid_next(void *,const void *);

int wnid_print(void *,long);
struct BUFFER *buf_pt;
const char *nid_file="/home/com/big-data/chinaid/nameid.bin";

int app_wnameid(const struct OPTION *opt)
{
    const char *nsrc;
    if (opt->srcname == NULL){
        wprintf(L"使用缺省文件名%s\n",nid_file);
        nsrc=nid_file;
    } else {
        nsrc=opt->srcname;
    }
    werror_init();
    wprintf(L"读文件简信:\033[0;36m%s\033[0m\n",nsrc);
    void *res=new_wnameid(nsrc);
    if (res == NULL){
        return -1;
    }
    long cnt=0;
    buf_pt=new_buffer(1024*128);
    while(1){
        int ok=wnid_print(res,cnt);
        if(ok ==-1){
            werror_print();
            break;
        }

        if(ok==-2){
            wprintf(L"all done in total records %d\n",cnt);
            break;
        }
        cnt++;
    }
    wnid_close(res);
    free(buf_pt);
    return 0;
}

