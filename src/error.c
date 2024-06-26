#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>

struct runtime_error{
    const char *file;
    const char *msg;
    int line;
    struct runtime_error *wrap;
};


static struct runtime_error *s_error=NULL;


void error_stack(const char *file,int line,const char* msg){
    struct runtime_error *rte=malloc(sizeof(struct runtime_error));
    rte->file=file;
    rte->line=line;
    rte->msg=msg;
    rte->wrap=s_error;
    s_error=rte;
}
void error_stack_v(const char *file,int line,const char* fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
    int num=vsnprintf(NULL,0,fmt,ap);
    va_end(ap);
    if(num==0){
        error_stack(file,line,"()");
    } else {
        char *msg=malloc(num+1);
        va_start(ap,fmt);
        num=vsnprintf(msg,num+1,fmt,ap);
        va_end(ap);
        if (num<0){
            free(msg);
            return;
        }
        error_stack(file,line,msg);
    }
    
}

void error_stack_by_errno(const char *file,int line){
    struct runtime_error *rte=malloc(sizeof(struct runtime_error));
    rte->file=file;
    rte->line=line;
    rte->msg=strerror(errno);
    rte->wrap=s_error;
    s_error=rte;
}

void print_error_stack(FILE *out){
    struct runtime_error *ptr=s_error;
    int lead=0;
    while(ptr){
        for(int xx=0;xx<lead;xx++){
            fputc(' ',out);
        }
        fprintf(out,"%s(%d):%s\n",ptr->file,ptr->line,ptr->msg);
        ptr=ptr->wrap;
        lead +=2;
    }
}
int has_error(void)
{
    return s_error != NULL;
}
void print_current_error(const char *prefix)
{
    printf("%s error code(%d) %s\n",prefix,errno,strerror(errno));
}
