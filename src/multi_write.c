#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>


extern int cpunum;
extern struct timespec NS100;

const char *file_tmp="/tmp/mutiwrite-test.bin";
struct mt_data{
    FILE *fp;
    pthread_mutex_t *mutex;
} run_data;


int write16(FILE *fh,uint16_t val)
{
    uint8_t b2[2];
    b2[0]=val & 0xff;
    b2[1]=0xff & (val >> 8);
    if(fwrite(&b2[0],2,1,fh)==0){
        return -1;
    }
    return 0;
}
void *task_write(void *pl)
{
    int idx=(long)pl;
    struct mt_data *rd=&run_data;
    for(int ix=0;ix<10;ix++){
        printf("%2d:%3d \n",idx,ix);
        pthread_mutex_lock(rd->mutex);
        fseek(rd->fp,(idx * 1000 + ix * 100)*2,SEEK_SET);
        for(int iy=0;iy<100;iy++){
            if(write16(rd->fp,idx * 1000+ix*100+iy)){
                pthread_mutex_unlock(rd->mutex);
                printf("%d thread: error at %s\n",idx,strerror(errno));
                return NULL;
            }
        }
        pthread_mutex_unlock(rd->mutex);
    }
    return NULL;
}
uint16_t get16(FILE *fh)
{
    uint8_t b2[2];
    uint16_t *p=(uint16_t *)&b2[0];
    fread(p,2,1,fh);
    return *p;
}
#define TASKNUM 16
void test_mt1(void)
{
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    run_data.mutex=&mutex;
    pthread_t pid[cpunum];
    FILE *fp=fopen(file_tmp,"w+");
    if(fp == NULL){
        printf("create file error\n");
        return;
    }
    run_data.fp=fp;
    for(int ix=0;ix<cpunum;ix++){
        pthread_create(&pid[ix],NULL,task_write,(void *)(long)ix);
    }
    for(int ix=0;ix<cpunum;ix ++){
        pthread_join(pid[ix],NULL);
    }
    rewind(fp);
    uint16_t v1=get16(fp);
    while(!feof(fp)){
        uint16_t v2=get16(fp);
        if(v2<v1){
            printf("not in order\n");
            return;
        }
        v1=v2;
    }
    printf("test ok\n");
    fclose(fp);
}

void *task_mw2(void *pl)
{
    int id=(int)(long)pl;
    FILE *fp=fopen(file_tmp,"r+");
    if(fp==NULL){
        printf("%d open file error\n",id);
        return NULL;
    }
    fseek(fp,id * 10*2,SEEK_SET);
    for(int ix=0;ix<10;ix++){
        write16(fp,id*10+ix);
        nanosleep(&NS100,NULL);
    }
    fclose(fp);
    return NULL;
}
void test_multi_write2(void)
{
    pthread_t pid[cpunum];
    for(int ix=0;ix<cpunum;ix++){
        pthread_create(&pid[ix],NULL,task_mw2,(void *)(long)ix);
    }
    for(int ix=0;ix<cpunum;ix++){
        pthread_join(pid[ix],NULL);
    }
}

void test_multi_write(void)
{
    test_multi_write2();
}