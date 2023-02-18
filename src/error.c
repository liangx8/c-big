#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <malloc.h>

struct runtime_error{
    const char *file;
    const char *msg;
    int line;
    struct runtime_error *wrap;
};

static struct runtime_error *s_error=NULL;

static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

void error_stack(const char *file,int line,const char* msg){
    struct runtime_error *rte=malloc(sizeof(struct runtime_error));
    rte->file=file;
    rte->line=line;
    rte->msg=msg;
    pthread_mutex_lock(&mutex);
    rte->wrap=s_error;
    s_error=rte;
    pthread_mutex_unlock(&mutex);
}
void error_stack_by_errno(const char *file,int line){
    struct runtime_error *rte=malloc(sizeof(struct runtime_error));
    rte->file=file;
    rte->line=line;
    rte->msg=strerror(errno);
    pthread_mutex_lock(&mutex);
    rte->wrap=s_error;
    s_error=rte;
    pthread_mutex_unlock(&mutex);
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

