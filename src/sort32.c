#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
//#include <time.h>

#include "entity.h"
#include "status.h"
#include "error_stack.h"
#include "bag.h"


#define MLOCK(mut,ret)      if(pthread_mutex_lock(mut)) {ERROR("线程锁错误\n");ret;}
#define MUNLOCK(mut,ret)    if(pthread_mutex_unlock(mut)) {ERROR("线程解锁错误\n");ret;}
#define MWAIT(cnd,mut)  if(pthread_cond_wait(cnd,mut)){ERROR("ptread_cond_wait() 错误\n");goto finish;}
//#define MBOARDCAST(cnd,ret) 
#define MSIGNAL(cnd)    if(pthread_cond_signal(cnd)){ERROR("pthread_cond_signal() 错误\n");goto finish;}

#define MEM_SORT_SIZE  20480

struct run_data{
    struct Bag              *jobs;
    pthread_mutex_t         *mutex;
    pthread_cond_t          *cond;
    const struct ENTITY     *ent;
    const char              *fname;
    // 处于活动状态的工作的数量
    int64_t                 *working;
    int                     running;
}sort_data;
extern int cpunum;

int int_act;
void repeat(FILE *out,int ch,int cnt);
void sighandler(int signum)
{
    int_act=signum;
    switch(signum){
        case SIGUSR1:
        int_act=SHOW_PROGRESS;
        break;
        case SIGUSR2:
        int_act=SHOW_PARTITION;
        repeat(stdout,'-',40);
        break;
        case SIGINT:
        int_act=SORTING_BREAK;
        break;
    }
}
int load4(FILE *fh,int64_t pos,uint8_t *buf,const struct ENTITY *ent)
{
    if(fseek(fh,pos * ent->unitsize,SEEK_SET)){
        ERROR_BY_ERRNO();
        return -1;
    }
    if(fread(buf,ent->unitsize,1,fh)<1){
        ERROR_BY_ERRNO();
        return -1;
    }
    return 0;
}
int save2(FILE *fh,int64_t pos,uint8_t *buf, const struct ENTITY *ent)
{
    fseek(fh,pos * ent->unitsize,SEEK_SET);
    if(fwrite(buf,ent->unitsize,1,fh)<1){
        return -1;
    }
    return 0;
}
uint32_t value(const void *src){
    uint32_t *pi= (uint32_t *)src;
    return *pi;
}
int64_t qsort_partition(FILE *fh,int64_t pos1,int64_t pos2,const struct ENTITY *ent){
    const int us=ent->unitsize;
    uint8_t *pivot=malloc(us*3);
    uint8_t *store=pivot + us;
    uint8_t *oix=store+us;
    int64_t store_idx=pos1;
    if(load4(fh,pos2-1,pivot,ent)){
        ERROR("读文件错误");
        goto exit_with_error;
    }
    while(1){
        if(store_idx==pos2-1){
            free(pivot);
            return store_idx;
        }
        if(load4(fh,store_idx,store,ent)){
            ERROR("读文件错误");
            goto exit_with_error;
        }
        if(ent->lt(store,pivot)){
            store_idx++;
        } else {
            break;
        }
    }
    for(int64_t ix=store_idx+1;ix<pos2-1;ix++){
        if(load4(fh,ix,oix,ent)){
            ERROR("读文件错误");
            goto exit_with_error;
        }
        if(ent->lt(oix,pivot)){
            if(save2(fh,store_idx,oix,ent)){
                ERROR("写文件错误");
                goto exit_with_error;
            }
            if(save2(fh,ix,store,ent)){
                ERROR("写文件错误");
                goto exit_with_error;
            }
            store_idx++;
            if(load4(fh,store_idx,store,ent)){
                ERROR("读文件错误");
                goto exit_with_error;
            }

        }
        if(int_act==SHOW_PARTITION){
            int64_t di=pos2-pos1;
            int precent = (ix-pos1) * 100 /di;
            printf("%10ld (%10ld,%10ld,%10ld) %3d%%\n",store_idx,pos1,ix,pos2,precent);
            int_act=NO_ACTION;
        }
    }
    if(save2(fh,pos2-1,store,ent)){
        ERROR("写文件错误");
        goto exit_with_error;
    }
    if(save2(fh,store_idx,pivot,ent)){
        ERROR("写文件错误");
        goto exit_with_error;
    }
    free(pivot);
    return store_idx;
exit_with_error:
    free(pivot);
    return -1;

}

int mem_sort(FILE *fh,int64_t pos,int64_t amount,char *buf,const struct ENTITY *ent)
{
    const int us=ent->unitsize;
    fseek(fh,pos * us,SEEK_SET);
    int64_t total=fread(buf,us,amount,fh);
    if(total != amount ){
        ERROR("内部错误");
        return -1;
    }
    qsort(buf,total,us,ent->cmp);
    fseek(fh,pos * us,SEEK_SET);
    total=fwrite(buf,us,total,fh);
    if(total != amount){
        ERROR("内部错误");
        return -1;
    }
    return 0;
}
void show_detail(struct run_data *rd)
{
    while(1){
/*         int all_stop=1;
        for(int ix=0;ix<cpunum;ix++){
            if(rd->working[ix*2+1]){
                all_stop=0;
                break;
            }
        }
        if(all_stop){
            break;
        }
 */
        pause();
        if(int_act==SORTING_BREAK){
            if(rd->running){
                rd->running=0;
                printf("程序将会在所有任务结束后保存进度\n");
                int_act=SHOW_PARTITION;
                break;
            }
        }
        if(int_act==SHOW_PROGRESS){
            repeat(stdout,'=',40);
            for(int ix=0;ix<cpunum;ix++){
                if(rd->working[ix*2+1]){
                    int64_t d1=rd->working[ix*2];
                    int64_t d2=rd->working[ix*2+1];
                    CP_MSG("id:%2d work on (%10ld,%10ld) %10ld\n",ix,d1,d2,d2-d1);
                }
            }
            bag_print(rd->jobs,stdout,8);
        }
    }
}
//extern struct timespec NS100;
void *sort32_task(void *obj)
{
    int id=(long)obj;
    struct run_data *sd=&sort_data;
    int64_t scope[2];
    FILE *fh=fopen(sd->fname,"r+");
    char *mem_buf=malloc(MEM_SORT_SIZE*sd->ent->unitsize);
    if(fh==NULL){
        ERROR_BY_ERRNO();
        return (void *)-1;
    }
    MLOCK(sd->mutex,goto finish)
    while(sd->running)
    {
        if(bag_get(sd->jobs,&scope[0])){
            // 工作队列无内容
            // 检测其他任务是否正在工作
            int done=1;
            sd->working[id*2]=0;
            sd->working[id*2+1]=0;
            for(int ix=0;ix<cpunum;ix++){
                if(ix==id)continue;
                // 右边界为０，意味任务进入idle
                if(sd->working[ix*2+1]){
                    done=0;
                    break;
                }
            }
            if(done){
                // 所有任务都停止了
                //sd->running=0;
                break;
            } else {
                // 等待其他的线程出结果
                MWAIT(sd->cond,sd->mutex)
            }
        } else {
            // 工作队列有内容
            sd->working[id*2]=scope[0];
            sd->working[id*2+1]=scope[1];
            MUNLOCK(sd->mutex,goto finish)
            int64_t pivot=qsort_partition(fh,scope[0],scope[1],sd->ent);
            //printf("id %2d (%10ld,%10ld,%10ld)\n",id,scope[0],pivot,scope[1]);
            if(pivot < 0){
                goto finish;
            }
            int64_t sco=pivot-scope[0];
            if(sco > 1){
                // 有效边界
                if(sco < MEM_SORT_SIZE){
                    // 用内存排序
                    if(mem_sort(fh,scope[0],sco,mem_buf,sd->ent)){
                        goto finish;
                    }
                } else {
                    MLOCK(sd->mutex,goto finish)
                    bag_put2(sd->jobs,scope[0],pivot);
                    MSIGNAL(sd->cond)
                    MUNLOCK(sd->mutex,goto finish)
                }
            }
            sco = scope[1]-pivot;
            if(sco > 2){
                if(sco < MEM_SORT_SIZE){
                    if(mem_sort(fh,pivot+1,sco-1,mem_buf,sd->ent)){
                        goto finish;
                    }
//                    PRE("id %2d acq lock\n",id);
                    MLOCK(sd->mutex,goto finish)
                } else {
                    MLOCK(sd->mutex,goto finish)
                    bag_put2(sd->jobs,pivot+1,scope[1]);
                    MSIGNAL(sd->cond)
                }
            }
        }
    }
    MSIGNAL(sd->cond)
//    sd->working[id*2]=0;
//    sd->working[id*2+1]=0;
    MUNLOCK(sd->mutex,goto finish)
    //sd->working[id]=0;
    free(mem_buf);
    fclose(fh);
    return NULL;
finish:
    sd->working[id*2]=0;
    sd->working[id*2+1]=0;
    MSIGNAL(sd->cond)
    MUNLOCK(sd->mutex,goto finish)
    if(pthread_cond_broadcast(sd->cond))
    {
        ERROR("pthread_cond_broadcast() 错误\n");
    }
    free(mem_buf);
    fclose(fh);
    return (void *)-1;
}
void sort32(struct STATUS *sta)
{
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
    pthread_t *thid;
    uint8_t *bytes;
    void *res;
    sort_data.cond=&cond;
    sort_data.mutex=&mutex;
    bytes=malloc(16*cpunum+sizeof(pthread_t) * cpunum);
    // working 占用2个64位的数据
    sort_data.working=(int64_t *)bytes;
    thid=(pthread_t *)(bytes+16*cpunum);
    bzero(sort_data.working,16*cpunum);
    sort_data.ent=sta->payload;
    sort_data.fname=sta->preform_dst;
    sort_data.jobs=bag_with_array(sta->scope,sta->scope_cnt);
    sort_data.running=1;
    for(int ix=0;ix<cpunum;ix++){
        pthread_create(thid + ix,NULL,sort32_task,(void *)(long) ix);
        printf("id:%2d=>%ld\n",ix,*(thid+ix));
    }
    show_detail(&sort_data);
    for(int ix=0;ix<cpunum;ix++){
        pthread_join(*(thid+ix),&res);
        if(res){
            printf("return error on id %2d\n",ix);
        }
    }
    if(has_error()){
        print_error_stack(stderr);
    }else {
        int size;
        int64_t *data=bag_array(sort_data.jobs,&size);
        if(size){
            sta->scope_cnt=size;
            sta->scope=data;
        } else {
            sta->scope_cnt=0;
            sta->scope=NULL;
            printf("排序完成\n");
        }
    }
    bag_free(sort_data.jobs);
    free(bytes);
}

int test_partition(const struct ENTITY *ent)
{
    FILE *fd=fopen("/home/tec/big/qq-sorted.bin","r+");
    if(fd==NULL){
        ERROR_BY_ERRNO();
        return -1;
    }
    long mid=qsort_partition(fd,89,82567,ent);
    fclose(fd);
    if(mid<0){
        return -1;
    }
    printf("===========>%10ld\n",mid);
    return 0;
}
/*
暂时的想法
master线程是空闲的因此会进入等待

slave不会空闲，只能是阻塞等待
mutex 需要2个(可能只需要一个)，cond 需要一个，在master 中进入 wait等待slave 来唤醒

***pthread_cond_wait() 使用的方法。***

一个调度进程(由主进程担任) master, 不会占用太多CPU,提供一个 (struct pthread_mutex_t *)mutex,空闲时调用pthread_cond_wait() 释放mutex

多个任务进程(pthread_create建立)task，任务完成后需要传输结果给master,先获取mutex,成功以后就存放数据到指定位置。然后pthread_cond_signal()
恢复master的运行.
*/