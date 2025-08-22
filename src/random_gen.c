
#ifdef linux
#include <stdio.h>
#include <malloc.h>
#include "log.h"
static FILE *rd;
void random_init(void){
    rd=fopen("/dev/random","r");
    if(rd==NULL){
        log_err(L"打开/dev/random错误\n");
    }
}
long random_long(void){
    long val;
    fread(&val,1,8,rd);
    return val;
}
void random_close(void){
    fclose(rd);
}

#endif
