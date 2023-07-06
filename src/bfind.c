#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <pthread.h>
#include "error_stack.h"


#define BUF_SIZE  1024 * 200
#define UNIT_SIZE 12
// 多没有排序的文件查找
struct seq_find_data{
    pthread_mutex_t *mutex;
    FILE            *fh;
    int64_t          res;
    uint32_t         value;
    int              finding;

} sfd;
void *find_task(void *obj)
{
//    int id=(long)obj;
    struct seq_find_data *sp=&sfd;
    char *buf=malloc(BUF_SIZE * UNIT_SIZE);
    if(buf == NULL){
        sp->finding=0;
        ERROR_BY_ERRNO();
        return NULL;
    }
    long ocnt=BUF_SIZE;
    long onum=BUF_SIZE;
    long segment=0;
//    printf("id:%d\n",id);
    while(sp->finding){
        uint32_t *pv;

        if(ocnt==onum){
            if(onum < BUF_SIZE){
                break;
            }
            ocnt=0;
            pthread_mutex_lock(sp->mutex);
            segment=ftell(sp->fh);
            onum=fread(buf,UNIT_SIZE,BUF_SIZE,sp->fh);
            pthread_mutex_unlock(sp->mutex);

        }
        if(onum==0){
            break;
        }
        pv=(uint32_t *)(buf+ocnt * UNIT_SIZE);

        if(*pv == sp->value){
            sp->res=segment/12 + ocnt;
            sp->finding=0;
           
            break;
        }
        ocnt ++;
    }
    free(buf);
    return NULL;
}
extern int cpunum;
int list(const char *fname, int64_t offset, int limit,int hl);
void seq_find(const char *fname,uint32_t val,int limit)
{
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    pthread_t pid[cpunum-1];
    sfd.finding =1;
    sfd.res     =-1;
    sfd.value   =val;
    sfd.mutex   =&mutex;
    printf("finding value(%u) in \"%s\"\n",val,fname);
    FILE *fh=fopen(fname,"r");
    if(fh==NULL){
        ERROR_BY_ERRNO();
        return ;
    }
    sfd.fh=fh;
    for(int ix=0;ix<cpunum-1;ix++){
        pthread_create(&pid[ix],NULL,find_task,(void *)(long)ix);
    }
    find_task((void *)99);
    for(int ix=0;ix<cpunum-1;ix++){
        pthread_join(pid[ix],NULL);
    }
    if(has_error()){
        print_error_stack(stdout);
        return;
    }

    if(sfd.res<0){
        printf("not found\n");
    } else {
        printf("find at %ld\n",sfd.res);
        int hl=limit/2;
        long offset=sfd.res - hl;
        if(offset<0){
            offset=0;
        }
        list(fname,offset,limit,hl);
    }
    fclose(fh);
}