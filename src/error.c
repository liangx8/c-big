#include <errno.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdlib.h>
#define BUF_SIZE 4096

struct runtime_error{
    const char *file;
    const wchar_t *msg;
    int line;
    struct runtime_error *wrap;
};

static struct runtime_error *s_error=NULL;
long *mgr;

void error_init(void)
{
    mgr=malloc(BUF_SIZE);
    *mgr=sizeof(long);
}
void error_release()
{
    free(mgr);
}

void error_stack(const char *file,int line,const wchar_t *msg){
    
    struct runtime_error *rte=(void *)mgr+*mgr;
    *mgr +=sizeof(struct runtime_error);
    rte->file=file;
    rte->line=line;
    rte->msg=msg;
    rte->wrap=s_error;
    s_error=rte;
}
void error_stack_v(const char *file,int line,const wchar_t* fmt, ...)
{

    va_list ap;
    wchar_t *msg;
    msg=(void *)mgr+*mgr;
    va_start(ap,fmt);
    int num=vswprintf(msg,BUF_SIZE-*mgr,fmt,ap);
    va_end(ap);
    if(num>=0){
        *mgr += (num+1)*sizeof(wchar_t);
        *(msg+num)=L'\0';
        error_stack(file,line,msg);
    }
}

void error_stack_by_errno(const char *file,int line){

    const char *msg=strerror(errno);
    error_stack_v(file,line,L"%s",msg);
}

void print_error_stack(FILE *out){
    struct runtime_error *ptr=s_error;
    int lead=0;
    while(ptr){
        for(int xx=0;xx<lead;xx++){
            fputwc(L' ',out);
        }
        fwprintf(out,L"%s(%d):%ls\n",ptr->file,ptr->line,ptr->msg);
        ptr=ptr->wrap;
        lead +=2;
    }
}
int has_error(void)
{
    return s_error != NULL;
}

void fatal(const char *file,int line,const wchar_t *msg)
{
    fwprintf(stderr,msg);
    exit(-1);
}