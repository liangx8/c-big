#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include "error_stack.h"
#include "status.h"

#define UNIT_SIZE 12
#define BUF_CNT 100000
extern int int_act; // define in main.c
int this_act;
int set_int(int *,int);
struct apt_data
{
    int64_t left, right, idx;
    char povit[12];
};

struct run_data{
    pthread_mutex_t *mutex;
    pthread_cond_t  *cond;
    struct apt_data data;
};
struct buf_ctrl{
    // 实际的大小
    int size;
    // 当前处理的位置
    int pos;
    char *data;
};

size_t fill_buf(FILE *fp,struct buf_ctrl *pb){
    size_t n=fread(pb->data,UNIT_SIZE,BUF_CNT,fp);
    pb->size=n;
    pb->pos=0;
    return n;
}
uint32_t getqq(struct buf_ctrl *buf){
    uint32_t *p=(uint32_t*)(buf->data+buf->pos * UNIT_SIZE);
    return *p;
}
int full_path(char *,const char *);
void *apt_print(void *);
/**
 * @brief 把记录分开2部分，第一部分的值小于分界位置的值，第二部分大于等于
 * @param sta 配置
 * @return 返回分界的位置,返回的值属于第二部分
 */
void apart32(struct STATUS *sta)
{
    struct run_data rd;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond   = PTHREAD_COND_INITIALIZER;
    uint64_t pid;
    size_t num;
    char *fn=malloc(512);
    //这部分没有运算过程，因此不考虑用多线程
    FILE *fsrc, *fdst;
    
    char povit[UNIT_SIZE];
    uint32_t povit_value;
    struct buf_ctrl buf;
//    char *povit = &rd.data.povit[0];
    int64_t left,right,cnt,size;
    buf.data=malloc(UNIT_SIZE*BUF_CNT);
    if(buf.data==NULL){
        ERROR_BY_ERRNO();
        return;
    }
    full_path(fn,sta->src);
    fsrc=fopen(fn,"r");
    if(fsrc==NULL){
        ERROR(fn);
        ERROR_BY_ERRNO();
        return;
    }
    num=fread(&povit[0],UNIT_SIZE,1,fsrc);
    if(num==0 && ferror(fsrc)){
        ERROR_BY_ERRNO();
        goto err1_return;
    }
    povit_value=*((uint32_t *)(&povit[0]));
    fseek(fsrc,0,SEEK_END);
    size=ftell(fsrc);
    size=size /UNIT_SIZE;
    if(sta->step1progress>=size){
        fclose(fsrc);
        free(buf.data);
        return;
    }
    if(sta->scope_cnt){
        if(sta->scope_cnt & 1){
            ERROR("内部错误:设置文件逻辑错误:scope必须成对出现");
            goto err1_return;
        }
        left=sta->scope[0];
        right=sta->scope[1];
        cnt=sta->step1progress;
        printf("从上次中断点继续 progress: %ld,left: %ld, right: %ld\n",cnt,left,right);
    } else {
        left=0;
        right=size;
        cnt=1;
    }

    rd.mutex=&mutex;
    rd.cond=&cond;
    if(pthread_create(&pid,NULL,apt_print,&rd)){
        ERROR_BY_ERRNO();
        goto err1_return;
    }
    full_path(fn,sta->dst);
    fdst=fopen(fn,"w+");
    if(fdst==NULL){
        ERROR(fn);
        ERROR_BY_ERRNO();
        goto err1_return;
    }
    free(fn);

    fseek(fsrc,cnt*UNIT_SIZE,SEEK_SET);
    buf.pos=0;
    buf.size=0;
    while(1){
        char *pb;
        if(buf.pos==buf.size){
            num=fill_buf(fsrc,&buf);
            if(num==0){
                if(left+1!=right){
                    // 不可能发生此情况。
                    ERROR("apart32():最后的数据不对，程序有bug");
                    printf("left:%ld,right:%ld,total:%ld\n",left,right,cnt);
                    goto err_return;
                }
                this_act=SORTING_DONE;
                fseek(fdst,left*UNIT_SIZE,SEEK_SET);
                if(fwrite(&povit[0],UNIT_SIZE,1,fdst)!=1){
                    ERROR_BY_ERRNO();
                    goto err_return;
                }
                sta->pivot_index=left;
                sta->pivot_value=povit_value;
                sta->step1progress=cnt;
                sta->scope_cnt=4;
                if(sta->scope){
                    free(sta->scope); // release the old one
                }
                sta->scope=malloc(sizeof(int64_t)*4);
                sta->scope[0]=0;
                sta->scope[1]=left;
                sta->scope[2]=left+1;
                sta->scope[3]=size;
                break;
            }
        }
        uint32_t qq=getqq(&buf);
        pb=&buf.data[buf.pos*UNIT_SIZE];
        cnt++;
        buf.pos++;
        if(qq<povit_value){
            // save to left
            fseek(fdst,left*UNIT_SIZE,SEEK_SET);
            if(fwrite(pb,UNIT_SIZE,1,fdst)!=1){
                ERROR_BY_ERRNO();
                goto err_return;
            }
            left++;
        } else {
            // save to right
            right--;
            fseek(fdst,right*UNIT_SIZE,SEEK_SET);
            if(fwrite(pb,UNIT_SIZE,1,fdst)!=1){
                ERROR_BY_ERRNO();
                goto err_return;
            }
        }
        if(int_act){
            this_act=set_int(&int_act,0);
        }
        switch(this_act){
            case SHOW_PROGRESS:
                if (pthread_mutex_lock(&mutex))
                {
                    ERROR("线程锁错误");
                    goto err_return;
                }
                pthread_cond_signal(&cond);
                rd.data.idx=cnt;
                rd.data.left=left;
                rd.data.right=right;
                memcpy(&rd.data.povit[0],&povit[0],UNIT_SIZE);
                if (pthread_mutex_unlock(&mutex))
                {
                    ERROR("线程解锁错误");
                    goto err_return;
                }
                break;
            case SORTING_BREAK:
            int_act=SORTING_BREAK;
            sta->step1progress=cnt;
            sta->scope_cnt=2;
            sta->scope=malloc(sizeof(int64_t)*2);
            sta->scope[0]=left;
            sta->scope[1]=right;

            goto break_while;
        }
    }
    break_while:
    pthread_cond_signal(&cond);
    pthread_join(pid,NULL);
    printf("pid:%ld joined\n",pid);
    fclose(fdst);
    fclose(fsrc);
    free(buf.data);
    return;


err_return:
    fclose(fdst);
err1_return:
    fclose(fsrc);
    free(buf.data);
    return;

}

void _apt_print(struct apt_data *data)
{
    uint32_t povit = *((uint32_t *)(data->povit));
    printf("povit:%10u,index:%ld,left: %ld,right: %ld\n", povit, data->idx,data->left,data->right);
}
/**
 * @brief 在另外一个线程显示apart32处理的进度.
 * @param mutex 必须与apart32共用，以保证rdata数据的完整性
 */
void *apt_print(void *obj)
{
    struct run_data *prd=obj;
    if (pthread_mutex_lock(prd->mutex))
    {
        ERROR("线程锁错误\n");
        return NULL;
    }
    while (1)
    {

        switch (this_act)
        {
        case SHOW_PROGRESS:
            _apt_print(&prd->data);
            set_int(&this_act,0);
            break;
        case SORTING_BREAK:
            printf("排序中断，保存信息中...\n");
        case SORTING_DONE:
            if(pthread_mutex_unlock(prd->mutex)){
                ERROR("线程解锁错误\n");
            }
            return NULL;
        default:
            if (pthread_cond_wait(prd->cond, prd->mutex))
            {
                ERROR("线程等待错误");
                return NULL;
            }
            break;
        }
    }
}

int s32_apart_exam(struct STATUS *sta)
{
    struct buf_ctrl buf;
    int retval=0;
    int64_t pos;
    FILE *fp;
    char *fn=malloc(256);
    buf.data=malloc(UNIT_SIZE*BUF_CNT);
    if(buf.data==NULL){
        ERROR_BY_ERRNO();
        return -1;
    }
    full_path(fn,sta->dst);
    fp=fopen(fn,"r");
    if(fp==NULL){
        ERROR(fn);
        ERROR_BY_ERRNO();
        return -1;
    }
    free(fn);
    buf.pos=0;
    buf.size=0;
    pos=0;

    while(1){
        if (buf.pos == buf.size){
            if (fill_buf(fp,&buf)==0){
                // 没有错误的完成了检查
                break;
            }
        }
        if(pos<sta->pivot_index){
            uint32_t qq=getqq(&buf);
            if (qq >= sta->pivot_value){
                retval=1;
                break;
            }
        } else {
            uint32_t qq=getqq(&buf);
            if (qq < sta->pivot_value){
                retval=1;
                break;
            }
        }
        buf.pos++;
        pos++;
    }
    fclose(fp);
    free(buf.data);
    return retval;
}