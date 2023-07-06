#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

#include "error_stack.h"
#include "status.h"

#include "test_item.h"


#define UNIT_SIZE 12
#define BUF_CNT 102400

#define DEBUG_LOG(x) printf("%s(%3d):",__FILE__,__LINE__); x


#ifdef DNUM
FILE *dout;

const char* JSON_LEFT_FORM="{\"id\":%1$d,\"type\":\"left\",\"value\":%2$ld},";
const char* JSON_RIGHT_FORM="{\"id\":%1$d,\"type\":\"right\",\"value\":%2$ld},";
#endif

extern int int_act; // define in main.c
extern int cpunum;

struct run_data{
    FILE *fdst,*fsrc;
    pthread_mutex_t *mu_read;
    pthread_mutex_t *mu_writ;
    int64_t pos_src,pos_left,pos_right;
    uint8_t pivot[UNIT_SIZE];
} apart_data;
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
    #ifdef DNUM
    int show_dnum=0;
    #endif
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
#ifdef DNUM
            int64_t pos_dnum=ad->pos_left;
#endif
            fseek(ad->fdst,ad->pos_left*UNIT_SIZE,SEEK_SET);
            int64_t size=fwrite(p1,UNIT_SIZE,BUF_CNT,ad->fdst);
            ad->pos_left += BUF_CNT;
            pthread_mutex_unlock(ad->mu_writ);
            p1_idx=0;
            if(size < BUF_CNT){
                ERROR_BY_ERRNO();
                return NULL;
            }
#ifdef DNUM
            if(show_dnum){
                //DEBUG_LOG(printf("id:%2d, pos_left: %10ld\n",id,pos_dnum));
                show_dnum=0;
            }
            fprintf(dout,JSON_LEFT_FORM,id,pos_dnum);
#endif
        }
        if(p2_idx==BUF_CNT){
            pthread_mutex_lock(ad->mu_writ);
            ad->pos_right -= BUF_CNT;
#ifdef DNUM
            int64_t pos_dnum= ad->pos_right;
#endif
            fseek(ad->fdst,ad->pos_right*UNIT_SIZE,SEEK_SET);
            int64_t size=fwrite(p1,UNIT_SIZE,BUF_CNT,ad->fdst);
            pthread_mutex_unlock(ad->mu_writ);
            p2_idx=0;
#ifdef DNUM
            fprintf(dout,JSON_RIGHT_FORM,id,pos_dnum);
#endif
            if(size < BUF_CNT){
                ERROR_BY_ERRNO();
                return NULL;
            }
        }
        i32=(uint32_t*)(src + src_idx * UNIT_SIZE);
        src_idx ++;
        if(*i32 < pvalue){
#ifdef DNUM            
            if(DNUM == *i32){
                //DEBUG_LOG(printf("id:%2d,   p1_idx: %10d\n",id,p1_idx));
                show_dnum=1;
            }
#endif
            memcpy(p1+p1_idx*UNIT_SIZE,src+src_idx * UNIT_SIZE,UNIT_SIZE);
            p1_idx++;
        } else {
#ifdef DNUM
            if(DNUM == *i32){
                DEBUG_LOG(printf("id:%2d,  p2_idx: %10d\n",id,p2_idx));
                show_dnum=1;
            }
#endif
            memcpy(p2+p2_idx*UNIT_SIZE,src+src_idx * UNIT_SIZE,UNIT_SIZE);
            p2_idx++;
        }
        if(int_act==SHOW_PROGRESS){
            int act;
            pthread_mutex_lock(ad->mu_read);
            if(int_act==SHOW_PROGRESS){
                int_act=0;
                act=1;
            } else {
                act=0;
            }
            pthread_mutex_unlock(ad->mu_read);
            if(act){
                printf("id:%2d,pivot:%u,left:%9ld,right:%9ld,src-index:%9ld\n",id,pvalue,ad->pos_left,ad->pos_right,ad->pos_src);
            }
        }
    }
    // 源文件中最后一块数据
    //printf("id:%2d,last_size:%ld\n",id,last_size);
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
#ifdef DNUM
            int64_t pos_dnum;
#endif
            p1_idx=0;
            pthread_mutex_lock(ad->mu_writ);
#ifdef DNUM
            pos_dnum = ad->pos_left;
#endif
            fseek(ad->fdst,ad->pos_left*UNIT_SIZE,SEEK_SET);
            int64_t size=fwrite(p1,UNIT_SIZE,BUF_CNT,ad->fdst);
            ad->pos_left += BUF_CNT;
            pthread_mutex_unlock(ad->mu_writ);
            if(size < BUF_CNT){
                ERROR_BY_ERRNO();
                return NULL;
            }
#ifdef DNUM
            fprintf(dout,JSON_LEFT_FORM,id,pos_dnum);
#endif
        }
        if(p2_idx==BUF_CNT){
#ifdef DNUM
            int64_t pos_dnum;
#endif
            p2_idx=0;
            pthread_mutex_lock(ad->mu_writ);
            ad->pos_right -= BUF_CNT;
#ifdef DNUM
            pos_dnum = ad->pos_right;
#endif
            fseek(ad->fdst,ad->pos_right*UNIT_SIZE,SEEK_SET);
            int64_t size=fwrite(p1,UNIT_SIZE,BUF_CNT,ad->fdst);
            pthread_mutex_unlock(ad->mu_writ);
            if(size < BUF_CNT){
                ERROR_BY_ERRNO();
                return NULL;
            }
#ifdef DNUM
            fprintf(dout,JSON_RIGHT_FORM,id,pos_dnum);
#endif
        }
    }
    // 最后把剩下的p1,p2都写到文件中
    if(p1_idx){
#ifdef DNUM
            int64_t pos_dnum;
#endif
        pthread_mutex_lock(ad->mu_writ);
#ifdef DNUM
            pos_dnum = ad->pos_left;
#endif
        fseek(ad->fdst,ad->pos_left*UNIT_SIZE,SEEK_SET);
        int64_t size=fwrite(p1,UNIT_SIZE,p1_idx,ad->fdst);
        if(size < p1_idx){
            //出错了。
            ERROR_BY_ERRNO();
        } else {
            ad->pos_left += p1_idx;
        }
        pthread_mutex_unlock(ad->mu_writ);
#ifdef DNUM
            fprintf(dout,JSON_LEFT_FORM,id,pos_dnum);
#endif
        //printf("id:%2d,pos_left :%10ld, p1_idx:%10d\n",id,ad->pos_left,p1_idx);
        if(has_error()){
            return NULL;
        }
    }
    if(p2_idx){
#ifdef DNUM
        int64_t pos_dnum;
#endif
        pthread_mutex_lock(ad->mu_writ);
        ad->pos_right -= p2_idx;
#ifdef DNUM
        pos_dnum = ad->pos_right;
#endif
        fseek(ad->fdst,ad->pos_right * UNIT_SIZE,SEEK_SET);
        int64_t size=fwrite(p2,UNIT_SIZE,p2_idx,ad->fdst);
        pthread_mutex_unlock(ad->mu_writ);
        //printf("id:%2d,pos_right:%10ld, p2_idx:%10d\n",id,ad->pos_right,p2_idx);
        if(size < p2_idx){
            //出错了。
            ERROR_BY_ERRNO();
        }
#ifdef DNUM
            fprintf(dout,JSON_RIGHT_FORM,id,pos_dnum);
#endif
    }
    free(src);
    free(p1);
    free(p2);
    return NULL;
}


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

#ifdef DNUM
    dout=fopen("tmp.json","w+");
#endif
    fh=fopen(stu->preform_dst,"w+");
    if(fh==NULL){
        ERROR(stu->preform_dst);
        ERROR_BY_ERRNO();
        return;
    }
        
    apart_data.fdst=fh;
    fh=fopen(stu->preform_src,"r");
    if(fh==NULL){
        fclose(apart_data.fdst);
        ERROR_BY_ERRNO();
        return;
    }
    fseek(fh,0,SEEK_END);
    amount=ftell(fh);
    //assert((amount % UNIT_SIZE) ==0 );
    if(amount % UNIT_SIZE){
        ERROR("文件大小不能对齐１２字节\n");
        fclose(fh);
        fclose(apart_data.fdst);
        return;
    }
    amount = amount /UNIT_SIZE;
    apart_data.fsrc      = fh;
    if(stu->step==1){
        //assert(stu->scope_cnt == 3);
        // step 1检测scope_cnt无意义，固定是３个记录
        apart_data.pos_src   = stu->scope[0];
        apart_data.pos_left  = stu->scope[1];
        apart_data.pos_right = stu->scope[2];

    } else {
        apart_data.pos_right = amount;
        apart_data.pos_left  = 0;
        apart_data.pos_src   = 1;

    }
    rewind(fh);
    if(fread(&apart_data.pivot[0],UNIT_SIZE,1,fh)<1){
        ERROR_BY_ERRNO();
        return;
    }
    //fseek(fh,apart_data.pos_src * UNIT_SIZE,SEEK_SET);
    apart_data.mu_read   = &mu_read;
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
#ifdef DNUM
    fclose(dout);
#endif
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
