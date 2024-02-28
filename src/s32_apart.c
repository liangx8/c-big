#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

#include "entity.h"
#include "error_stack.h"
#include "status.h"

//#include "test_item.h"


#define BUF_CNT 102400
#define DBUF_CNT 1000

//#define DEBUG_LOG(x) printf("%s(%3d):",__FILE__,__LINE__); x
#define DEBUG_LOG(fmt,args...) printf("%s(%3d):" fmt ,__FILE__,__LINE__,args)



extern int int_act; // define in main.c
extern int cpunum;
extern struct timespec NS100;

struct run_data{
    FILE *fdst,*fsrc;
    pthread_mutex_t *mu_writ;
    const struct ENTITY *ent;
    int64_t pos_left,pos_right;
    uint8_t *pivot;
} apart_data;
void *apart_task(void *obj)
{
    int id=(long)obj;
    struct run_data *rd=&apart_data;
    const int us= rd->ent->unitsize;
    char *buf= malloc(BUF_CNT * us);
    char *bl = malloc(DBUF_CNT * us);
    char *br = malloc(DBUF_CNT * us);
    long idx=0;
    int cntl=0;
    int cntr=0;
    long num=0;
    num=fread(buf,us,BUF_CNT,rd->fsrc);
    if(num){
        while(1){
            char *sptr=buf+(idx*us);
            // if(id==0){
            //     printf("%s %x\n",rd->ent->str(sptr),rd->ent->cmp(sptr,rd->pivot));
            // }
            if(rd->ent->lt(sptr,rd->pivot)){
                memcpy(bl+(cntl * us),sptr,us);
                cntl ++;
                if(cntl == DBUF_CNT){
                    // 写入目标文件，重置ｃｎｔ
                    pthread_mutex_lock(rd->mu_writ);
                    fseek(rd->fdst,rd->pos_left * us,SEEK_SET);
                    if(fwrite(bl,us,DBUF_CNT,rd->fdst)<DBUF_CNT){
                        pthread_mutex_unlock(rd->mu_writ);
                        ERROR("写入数据不完整");
                        return NULL;
                    }
                    rd->pos_left += DBUF_CNT;
                    pthread_mutex_unlock(rd->mu_writ);
                    cntl=0;
                }
            } else {
                memcpy(br + (cntr * us),sptr,us);
                cntr ++;
                if(cntr == DBUF_CNT){
                    pthread_mutex_lock(rd->mu_writ);
                    rd->pos_right -= DBUF_CNT;
                    fseek(rd->fdst,rd->pos_right * us,SEEK_SET);
                    if(fwrite(br,us,DBUF_CNT,rd->fdst)<DBUF_CNT){
                        pthread_mutex_unlock(rd->mu_writ);
                        ERROR("写入数据不完整");
                        return NULL;
                    }
                    pthread_mutex_unlock(rd->mu_writ);
                    cntr=0;
                }
            }
            idx ++;
            if(idx == num){
                if(feof(rd->fsrc)){
                    break;
                }
                num=fread(buf,us,BUF_CNT,rd->fsrc);
                if(num==0){
                    break;
                }
                idx=0;
            }
        }
        pthread_mutex_lock(rd->mu_writ);
        // idx 一定是　0
        if(cntl){
            fseek(rd->fdst,rd->pos_left * us,SEEK_SET);
            if(fwrite(bl,us,cntl,rd->fdst)<cntl){
                pthread_mutex_unlock(rd->mu_writ);
                ERROR("写入数据不完整");
                return NULL;
            }
            rd->pos_left += cntl;
        }
        if(cntr){
            rd->pos_right -= cntr;
            fseek(rd->fdst,rd->pos_right * us,SEEK_SET);
            if(fwrite(br,us,cntr,rd->fdst)<cntr){
                pthread_mutex_unlock(rd->mu_writ);
                ERROR("写入数据不完整");
                return NULL;
            }
        }
        pthread_mutex_unlock(rd->mu_writ);

    } else {
        printf("task %2d done nothing\n",id);
    }
    free(buf);
    free(bl);
    free(br);
    return NULL;
}

long filesize(const char*path); // pathutil.c

/**
 * @brief 把记录分开2部分，第一部分的值小于分界位置的值，第二部分大于等于
 * @param sta 配置
 * @return 返回分界的位置,返回的值属于第二部分
 */
void apart32(struct STATUS *stu,const void *pl)
{
    FILE *fh;
    int64_t amount;
    pthread_t *pid;
    pthread_mutex_t mu_writ=PTHREAD_MUTEX_INITIALIZER;
    apart_data.ent=stu->payload;
    uint8_t pivot[apart_data.ent->unitsize];
    

    fh=fopen(stu->preform_dst,"w+");
    if(fh==NULL){
        ERROR(stu->preform_dst);
        ERROR_BY_ERRNO();
        return;
    }
        
    apart_data.fdst=fh;
    amount=filesize(stu->preform_src);
    if(amount<0){
        ERROR(stu->preform_dst);
        ERROR_BY_ERRNO();
        fclose(apart_data.fdst);
        return;
    }
    if(amount % apart_data.ent->unitsize){
        ERROR("文件大小不能对齐字节\n");
        fclose(apart_data.fdst);
        return;
    }
    amount = amount / apart_data.ent->unitsize;
    fh=fopen(stu->preform_src,"r");
    if(fh==NULL){
        ERROR(stu->preform_src);
        ERROR_BY_ERRNO();
        fclose(apart_data.fdst);
        return;
    }
    apart_data.fsrc      = fh;
    apart_data.pos_right = amount;
    apart_data.pos_left  = 0;
    apart_data.pivot=&pivot[0];
    if(fread(&pivot[0],apart_data.ent->unitsize,1,fh)<1){
        ERROR_BY_ERRNO();
        return;
    }
    
    printf("pivot value %s, amount:%ld\n",apart_data.ent->keystr(apart_data.pivot),amount);
    apart_data.mu_writ   = &mu_writ;
    pid=malloc(sizeof(pthread_t)*(cpunum-1));

    for(int ix=0;ix<cpunum-1;ix++){
        pthread_create(pid+ix,NULL,apart_task,(void *)(long)ix);
    }
    apart_task((void *)99L);
    for(int ix=0;ix<cpunum-1;ix++){
        pthread_join(pid[ix],NULL);
    }
    free(pid);
    if(has_error()){
        print_error_stack(stdout);
        return;
    }
    if(apart_data.pos_right - apart_data.pos_left != 1){
        printf("结果错误: left:%ld,right:%ld\n",apart_data.pos_left , apart_data.pos_right);
        return;
    }
    fseek(apart_data.fdst,apart_data.pos_left*apart_data.ent->unitsize,SEEK_SET);
    if(fwrite(apart_data.pivot,apart_data.ent->unitsize,1,apart_data.fdst)<1){
        ERROR_BY_ERRNO();
        return;
    }
    fclose(apart_data.fdst);
    fclose(apart_data.fsrc);

    stu->scope_cnt=4;
    stu->scope=malloc(32);
    stu->scope[0]=0;
    stu->scope[1]=apart_data.pos_left;
    stu->scope[2]=apart_data.pos_right;
    stu->scope[3]=amount;
    stu->step=2;
}
