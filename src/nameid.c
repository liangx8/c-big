#include <stdint.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "options.h"
#include "error_stack.h"
#include "buffer.h"
#include "nameid.h"


const char *srcname="/home/tec/big/nameid.bin";
const wchar_t *wtest=L"零壹贰叁肆务陆柒捌玖";

int app_nameid(struct OPTION *opt)
{
    struct NAMEID *ptr=new_nameid(srcname);
    if (ptr == NULL){
        ERROR_WRAP();
        return -1;
    }

    int seq=0;
    struct BUFFER *sb=new_buffer(2048);
    if(sb == NULL){
        nameid_close(ptr);
        ERROR_WRAP();
        return -1;
    }
    int retval=0;
    while(1){

        if(nameid_print1(ptr,sb)){
            ERROR_WRAP();
            retval = -1;
            break;
        }
        seq++;
        int rs=nameid_next(ptr);
        if (rs){
            if (rs<0){
                retval = -1;
                ERROR_WRAP();
                break;
            }
        } else {
            break;
        }
    }
    buffer_free(sb);
    nameid_close(ptr);
    printf("all:%d\n",seq);
    return retval;
}