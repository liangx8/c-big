#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

#include "error_stack.h"
#include "status.h"
#ifdef FIXING_BUG
#include "bag.h"
#endif

#define UNIT_SIZE 12
#define BUF_CNT 102400


extern int int_act; // define in main.c
extern int cpunum;

struct run_data{
    FILE *fdst,*fsrc;
    pthread_mutex_t *mu_read;
    pthread_mutex_t *mu_writ;
    int64_t pos_src,pos_left,pos_right;
    uint8_t pivot[UNIT_SIZE];
#ifdef FIXING_BUG
    struct Bag *bag;
#endif

} apart_data;
#ifdef FIXING_BUG
#define TEST_SIZE 1024000
extern const struct timespec MS1;
int64_t gix=0;
uint8_t *gbuf;
void *test_task(void *obj)
{
    int id=(long)obj;
    struct run_data *ad=&apart_data;
    while(1){
        long pos;
        if(feof(ad->fsrc)){
            break;
        }
        //pthread_mutex_lock(ad->mu_read);
        pos=ftell(ad->fsrc);
        fread(gbuf,UNIT_SIZE,TEST_SIZE,ad->fsrc);
        //pthread_mutex_unlock(ad->mu_read);
        pthread_mutex_lock(ad->mu_writ);
        if(bag_exist(ad->bag,pos)){
            printf("id %d: position(%ld) duplicate error\n",id,pos);
        } else {
            if(bag_put(ad->bag,pos)){
                printf("Bag容器容量不够，无法测试\n");
                fseek(ad->fsrc,0,SEEK_END);
                pthread_mutex_unlock(ad->mu_writ);
                return NULL;
            }
        }
        pthread_mutex_unlock(ad->mu_writ);
        pthread_mutex_lock(ad->mu_read);
        if(int_act==SHOW_PROGRESS){
            int_act=0;
            pthread_mutex_unlock(ad->mu_read);
            printf("id:%2d->%9ld\n",id,pos);
        } else {
            pthread_mutex_unlock(ad->mu_read);
        }
    }
    int_act=SORTING_BREAK;
    return NULL;
}
#endif
void *apart_task(void *obj)
{
    int id=(long)obj;
    struct run_data *ad=&apart_data;
    uint32_t pvalue= *((uint32_t *)ad->pivot);
    int src_idx,p1_idx,p2_idx;
    uint8_t *src,*p1,*p2;
    src=malloc(BUF_CNT*UNIT_SIZE);
    if(src==NULL){
        ERROR_BY_ERRNO();
        return NULL;
    }
    p1=malloc(BUF_CNT*UNIT_SIZE);
    if(p1==NULL){
        ERROR_BY_ERRNO();
        return NULL;
    }
    p2=malloc(BUF_CNT*UNIT_SIZE);
    if(p2==NULL){
        ERROR_BY_ERRNO();
        return NULL;
    }
    src_idx=BUF_CNT;
    p1_idx=0;
    p2_idx=0;
    // 源数据的缓存，如果到了文件结尾，就不会是BUF_CNT的大小
    int64_t last_size;
    while(1){
        uint32_t *i32;
        if(src_idx==BUF_CNT){
            //读文件是进程安全，因此不需要排他锁
            last_size=fread(src,UNIT_SIZE,BUF_CNT,ad->fsrc);
            src_idx=0;
            pthread_mutex_lock(ad->mu_read);
            //fseek(ad->fsrc,ad->pos_src * UNIT_SIZE,SEEK_SET);
            ad->pos_src += last_size;
            pthread_mutex_unlock(ad->mu_read);
            if(last_size< BUF_CNT){
                //最后一块
                break;
            }
        }
        if(p1_idx==BUF_CNT){
            pthread_mutex_lock(ad->mu_writ);
            fseek(ad->fdst,ad->pos_left*UNIT_SIZE,SEEK_SET);
            int64_t size=fwrite(p1,UNIT_SIZE,BUF_CNT,ad->fdst);
            ad->pos_left += BUF_CNT;
            pthread_mutex_unlock(ad->mu_writ);
            p1_idx=0;
            if(size < BUF_CNT){
                ERROR_BY_ERRNO();
                return NULL;
            }
        }
        if(p2_idx==BUF_CNT){
            pthread_mutex_lock(ad->mu_writ);
            ad->pos_right -= BUF_CNT;
            fseek(ad->fdst,ad->pos_right*UNIT_SIZE,SEEK_SET);
            int64_t size=fwrite(p1,UNIT_SIZE,BUF_CNT,ad->fdst);
            pthread_mutex_unlock(ad->mu_writ);
            p2_idx=0;
            if(size < BUF_CNT){
                ERROR_BY_ERRNO();
                return NULL;
            }
        }
        i32=(uint32_t*)(src + src_idx * UNIT_SIZE);
        src_idx ++;
        if(*i32 < pvalue){
            memcpy(p1+p1_idx*UNIT_SIZE,src+src_idx * UNIT_SIZE,UNIT_SIZE);
            p1_idx++;
        } else {
            memcpy(p2+p2_idx*UNIT_SIZE,src+src_idx * UNIT_SIZE,UNIT_SIZE);
            p2_idx++;
        }
        pthread_mutex_lock(ad->mu_read);
        if(int_act==SHOW_PROGRESS){
            int_act=0;
            pthread_mutex_unlock(ad->mu_read);
            printf("id:%2d,pivot:%u,left:%9ld,right:%9ld,src-index:%9ld\n",id,pvalue,ad->pos_left,ad->pos_right,ad->pos_src);
        }else {
            pthread_mutex_unlock(ad->mu_read);
        }
    }

    for(int64_t ix=0;ix<last_size;ix++){
        uint8_t *psrc=src+ix * UNIT_SIZE;
        uint32_t *i32=(uint32_t*)psrc;
        if(*i32 < pvalue){
            memcpy(p1+p1_idx*UNIT_SIZE,psrc,UNIT_SIZE);
            p1_idx++;
        } else {
            memcpy(p2+p2_idx*UNIT_SIZE,psrc,UNIT_SIZE);
            p2_idx++;
        }
        if(p1_idx==BUF_CNT){
            p1_idx=0;
            pthread_mutex_lock(ad->mu_writ);
            fseek(ad->fdst,ad->pos_left*UNIT_SIZE,SEEK_SET);
            int64_t size=fwrite(p1,UNIT_SIZE,BUF_CNT,ad->fdst);
            ad->pos_left += BUF_CNT;
            pthread_mutex_unlock(ad->mu_writ);
            if(size < BUF_CNT){
                ERROR_BY_ERRNO();
                return NULL;
            }
        }
        if(p2_idx==BUF_CNT){
            p2_idx=0;
            pthread_mutex_lock(ad->mu_writ);
            ad->pos_right -= BUF_CNT;
            fseek(ad->fdst,ad->pos_right*UNIT_SIZE,SEEK_SET);
            int64_t size=fwrite(p1,UNIT_SIZE,BUF_CNT,ad->fdst);
            pthread_mutex_unlock(ad->mu_writ);
            if(size < BUF_CNT){
                ERROR_BY_ERRNO();
                return NULL;
            }
        }
    }
    // 最后把剩下的p1,p2都写到文件中
    if(p1_idx){
        pthread_mutex_lock(ad->mu_writ);
        fseek(ad->fdst,ad->pos_left*UNIT_SIZE,SEEK_SET);
        int64_t size=fwrite(p1,UNIT_SIZE,p1_idx,ad->fdst);
        if(size < p1_idx){
            //出错了。
            ERROR_BY_ERRNO();
        } else {
            ad->pos_left += p1_idx;
        }
        pthread_mutex_unlock(ad->mu_writ);
        if(has_error()){
            return NULL;
        }
    }
    if(p2_idx){
        pthread_mutex_lock(ad->mu_writ);
        ad->pos_right -= p2_idx;
        fseek(ad->fdst,ad->pos_right * UNIT_SIZE,SEEK_SET);
        int64_t size=fwrite(p2,UNIT_SIZE,p2_idx,ad->fdst);
        pthread_mutex_unlock(ad->mu_writ);
        if(size < p2_idx){
            //出错了。
            ERROR_BY_ERRNO();
        }
    }
    free(src);
    free(p1);
    free(p2);
    return NULL;
}
#ifdef FIXING_BUG
void seq_ok(struct Bag *bag);
#endif
int full_path(char *buf,const char *str);

/**
 * @brief 把记录分开2部分，第一部分的值小于分界位置的值，第二部分大于等于
 * @param sta 配置
 * @return 返回分界的位置,返回的值属于第二部分
 */
void apart32(struct STATUS *stu)
{
    FILE *fh;
    int64_t amount;
    pthread_t *pid;
    pthread_mutex_t mu_read=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mu_writ=PTHREAD_MUTEX_INITIALIZER;

    char *fn=malloc(256);
    if(stu->preform_dst){
        fh=fopen(stu->preform_dst,"w");
        if(fh==NULL){
            ERROR(fn);
            ERROR_BY_ERRNO();
            return;
        }
        
    }else {
        full_path(fn,stu->dst);
        fh=fopen(fn,"w");
        if(fh==NULL){
            ERROR(fn);
            ERROR_BY_ERRNO();
            return;
        }
    }
    apart_data.fdst=fh;
    full_path(fn,stu->src);
    fh=fopen(fn,"r");
    if(fh==NULL){
        fclose(apart_data.fdst);
        ERROR(fn);
        ERROR_BY_ERRNO();
        return;
    }
    fseek(fh,0,SEEK_END);
    amount=ftell(fh);

    //assert((amount % UNIT_SIZE) ==0 );
    if(amount % UNIT_SIZE){
        ERROR(fn);
        ERROR("文件大小不能对齐１２字节\n");
        fclose(fh);
        fclose(apart_data.fdst);
        return;
    }
    amount = amount /UNIT_SIZE;
    apart_data.fsrc      = fh;
    if(stu->step==1){
        //assert(stu->scope_cnt == 3);
        // step 1检测scope_cnt无意义，固定是３个记录，因此不会更新此变量
        apart_data.pos_src   = stu->scope[0];
        apart_data.pos_left  = stu->scope[1];
        apart_data.pos_right = stu->scope[2];

#if 0
        int64_t ix=0;
        while(1){
            if(ix>=amount){
                printf("逻辑错误\n");
                break;
            }
            if(ix == apart_data.pos_src){
                printf("正确\n");
                break;
            }
            ix += BUF_CNT;
        }
#endif
    } else {
        apart_data.pos_right = amount;
        apart_data.pos_left  = 0;
        apart_data.pos_src   = 1;

    }
    fseek(fh,0,SEEK_SET);
    if(fread(&apart_data.pivot[0],UNIT_SIZE,1,fh)<1){
        ERROR_BY_ERRNO();
        return;
    }
    fseek(fh,apart_data.pos_src * UNIT_SIZE,SEEK_SET);
    apart_data.mu_read   = &mu_read;
    apart_data.mu_writ   = &mu_writ;
    pid=malloc(sizeof(pthread_t)*(cpunum-1));
#ifdef FIXING_BUG
    apart_data.bag       = bag_create();
    gbuf = malloc(TEST_SIZE*UNIT_SIZE);
    for(int ix=0;ix<cpunum-1;ix++){
        pthread_create(pid+ix,NULL,test_task,(void *)(long)ix);
    }
    test_task((void *)(long)99);
    for(int ix=0;ix<cpunum-1;ix++){
        pthread_join(pid[ix],NULL);
    }
    free(gbuf);
    seq_ok(apart_data.bag);
    bag_print(apart_data.bag,stdout,10);
    bag_free(apart_data.bag);
#else
    for(int ix=0;ix<cpunum-1;ix++){
        pthread_create(pid+ix,NULL,apart_task,(void *)(long)ix);
    }
    apart_task((void *)(long)99);
    for(int ix=0;ix<cpunum-1;ix++){
        pthread_join(pid[ix],NULL);
    }
#endif
    free(pid);
    if(has_error()){
        return;
    }
    if(apart_data.pos_right - apart_data.pos_left != 1){
        printf("left:%ld,right:%ld,src:%ld\n",apart_data.pos_left , apart_data.pos_right,apart_data.pos_src);
        ERROR("结果不正确。");
        return;
    }
    fseek(apart_data.fdst,apart_data.pos_left*UNIT_SIZE,SEEK_SET);
    if(fwrite(&apart_data.pivot[0],UNIT_SIZE,1,apart_data.fdst)<1){
        ERROR_BY_ERRNO();
        return;
    }
    fclose(apart_data.fdst);
    fclose(apart_data.fsrc);
    if(int_act==SORTING_BREAK){
        stu->scope_cnt=3;
        stu->scope=malloc(24);
        stu->scope[0]=apart_data.pos_src;
        stu->scope[1]=apart_data.pos_left;
        stu->scope[2]=apart_data.pos_right;
        stu->step=1;
    }else {
        stu->scope_cnt=4;
        stu->scope=malloc(32);
        stu->scope[0]=0;
        stu->scope[1]=apart_data.pos_left;
        stu->scope[2]=apart_data.pos_right;
        stu->scope[3]=amount;
        stu->step=2;
    }
}

#ifdef FIXING_BUG
// 这个值是目前的测试环境得出来的。这个程序主要是验证fread在并发是的顺序性
#define LENG 703
int seq_exists(long val)
{
    long mod=(val-12) % (TEST_SIZE*UNIT_SIZE);

    return mod;
}
void seq_ok(struct Bag *bag){
    long *p64=(long *)bag;
    int ix=0;
    while(1){
        if(ix>=LENG){
            printf("数据正确\n");
            break;
        }
        long val=*(p64+ix);
        if(seq_exists(val)){
            printf("数据错误Sseq:%d,%ld\n",ix,val);
            break;
        }
        ix ++;
    }
}
#endif
#define LOG(m) printf("%s(%d):" m,__FILE__,__LINE__)
void s32_apart_exam(struct STATUS *sta)
{
    FILE *fh;
    char pivot[UNIT_SIZE];
    int64_t pidx=sta->scope[1];
    if(sta->preform_dst){
        fh=fopen(sta->preform_dst,"r");
        if(fh==NULL){
            ERROR(sta->preform_dst);
            ERROR_BY_ERRNO();
            return;
        }
    } else {
        char fn[256];
        full_path(fn,sta->dst);
        fh=fopen(fn,"r");
        if(fh==NULL){
            printf("open file \"%s\"",fn);
            ERROR_BY_ERRNO();
            return;
        }
    }
    fseek(fh,sta->scope[1]*UNIT_SIZE,SEEK_SET);
    if(fread(&pivot[0],UNIT_SIZE,1,fh)<1){
        ERROR_BY_ERRNO();
        fclose(fh);
        return;
    }
    uint32_t pv=*((uint32_t *)pivot);
    printf("value:%u\n",pv);
    rewind(fh);
    char *buf=malloc(BUF_CNT * UNIT_SIZE);
    int64_t ix=0;
    int64_t bix=BUF_CNT;
    long size=0;
    while(1){
        uint32_t *ppv;
        if(feof(fh)){
            LOG("内部错误！");
            goto nopass;
        }
        if(bix == BUF_CNT){
            // more code here
            size = fread(buf,UNIT_SIZE,BUF_CNT,fh);
            bix = 0;
            if(size < BUF_CNT){
                break;
            }
        }
        ppv = buf+bix*UNIT_SIZE;
        if(*ppv >= pv){
            goto nopass;
        }
        bix ++;
        ix ++;
        if(ix==pidx){
            // more code here
        }
    }

    while(!feof(fh)){

    }
    fclose(fh);
    return;
nopass:
    printf("数据没有被排序\n");
    fclose(fh);
}