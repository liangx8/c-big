#include <stdint.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "../status.h"
#include "../options.h"
#include "../error_stack.h"
#include "../buffer.h"
#include "nameid.h"


const char *srcname="/home/tec/big/nameid.bin";
const wchar_t *wtest=L"零壹贰叁肆务陆柒捌玖";

int app_nameid(struct STATUS *stu,struct OPTION *opt)
{
    struct NAMEID *ptr=new_nameid(srcname);
    if (ptr == NULL){
        return -1;
    }
    int seq=0;
    struct BUFFER *sb=new_buffer(2048);
    while(1){
        printf("seq: %3d ",seq);
        seq++;
        nameid_print1(ptr,sb);
        if (nameid_next(ptr)){
            break;
        }
    }
    buffer_free(sb);
    nameid_close(ptr);
    return 0;
}