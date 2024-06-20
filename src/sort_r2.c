/*备忘，思路比较混乱。先放下，理顺在继续
-A nameid -o 0 -l 1 这个功能已经实现。
后面考虑如何能最大的榨取ＣＰＵ的能力
*/

#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "entity.h"
#include "werror.h"
#include "app_path.h"
#include "breakinfo.h"
#include "bag.h"
#include "action_const.h"

#define MEM_USE_MAX 8000000000L
struct sort_status{
    char *data;
    const struct ENTITY *ent;
    pthread_mutex_t *mut;
    pthread_cond_t *cond;
    struct Bag *jobs;
    long *working;
    volatile int run;

} sorts;



extern int cpunum;
extern int action;
int is_order(char *data,size_t total, const struct ENTITY *ent)
{
    const int us=ent->unitsize;
    for(long lx=0;lx<total-1;lx++){
        char *ptrl=data + lx * us;
        char *ptrr=data + (lx + 1) * us;
        if(ent->lt(ptrr,ptrl)){
            long start=lx-5;
            if (start < 0){
                start=0;
            }
            for (;start < lx+5;start++){
                char *pint=data+start * us;
                ent->print(start,pint,NULL);
                wprintf(L"\n");
            }
            return 0;
        }
    }
    return 1;
}

long apart2(long left,long right)
{
    const int US=sorts.ent->unitsize;
    char pivot[US];
    char tmp[US];
    char *pvt=&pivot[0];
    

    memcpy(pvt,sorts.data+((right-1)*US),US);
    long store_idx=left;
    while(1){
        char *store=sorts.data + (store_idx * US);
        if(store_idx==(right-1)){
            return store_idx;
        }
        if(sorts.ent->lt(store,pvt)){
            store_idx++;
        } else {
            break;
        }
    }
    // store_idx指向第一个大于等于pivot的值
    for(long lx=store_idx+1;lx<right-1;lx++){
        char *olx=sorts.data + (lx * US);
        if(sorts.ent->lt(olx,pvt)){
            char *store=sorts.data+(store_idx*US);
            memcpy(&tmp[0],olx,US);
            memcpy(olx,store,US);
            memcpy(store,&tmp[0],US);
            store_idx++;
        }
    }
    char *endptr=sorts.data+(right-1)*US;
    memcpy(endptr,sorts.data+store_idx * US,US);
    memcpy(sorts.data+store_idx*US,pvt,US);
    return store_idx;
}


int others_done(int id)
{
    for(int ix=0;ix<cpunum;ix++){
        if(ix==id) continue;
        if(sorts.working[ix*2]){
            return 0;
        }
        if(sorts.working[ix*2+1]){
            return 0;
        }
    }
    if(bag_num(sorts.jobs)){
        return 0;
    }
    return -1;
    
}
void *task(void *obj)
{
    long id=(int)(long)obj;
    struct sort_status *ss=&sorts;
    long *left=ss->working+id * 2;
    long *righ=left + 1;
    const wchar_t *exit_msg;
    pthread_mutex_lock(ss->mut);
    while(1){
        if(!ss->run){
            exit_msg=L"用户中断，当前任务%2d退出\n";
            break;
        }
        if(bag_get(ss->jobs,left)){
            *left=0;
            *righ=0;
            // 这里需要　pthread_cond_wait(),等待其他进程通知有内容或者，工作已经结束
            pthread_cond_wait(ss->cond,ss->mut);
            if(bag_get(ss->jobs,left)){
                // 被唤醒后，工作池中没有发现有任务。即认为所有任务已经结束。
                exit_msg=L"任务%2d 被唤醒退出\n";
                break;
            }
        }
        pthread_mutex_unlock(ss->mut);

        while(1){
            long lv=*left;
            long rv=*righ;
            int newJob=0;
            long mid=apart2(lv,rv);
            // 检查mid和left,righ的关系。判断是否放回到工作池中
            if(mid - lv >1){
                *left=lv;
                *righ=mid;
                newJob=1;
                if(!ss->run){
                    // FIXME:如果用户中断，保存结果，然后退出
                }
            }
            if(rv-mid >2){
                if(newJob){
                    pthread_mutex_lock(ss->mut);
                    if(bag_put2(ss->jobs,mid+1,rv)){
                        exit_msg=L"内部错误，当前任务%2d退出\n";
                        pthread_cond_broadcast(ss->cond);
                        *left=0;
                        *righ=0;
                        kill(getpid(),SIGINT);
                        goto outter_break;
                    }
                    pthread_mutex_unlock(ss->mut);
                    pthread_cond_signal(ss->cond);
                    //wprintf(L"\033[0;36mtask%2d: 生成任务\033[0m[%8ld,%8ld]\n",id,mid+1,rv);
                } else {
                    *left=mid+1;
                    *righ=rv;
                }
                newJob++;
            }

            if(newJob){
                // 结果有活干
            }else {
                pthread_mutex_lock(ss->mut);
                // 1 检查其他的任务是否都停止
                // 2 检查jobs(工作池)中是否还有任务
                if(others_done(id)){
                    exit_msg=L"所有工作完成，当前任务%2d退出\n";
                    pthread_cond_broadcast(ss->cond);
                    *left=0;
                    *righ=0;
                    kill(getpid(),SIGINT);
                    goto outter_break;
                }
                break;
            }
        }
    }
outter_break:
    pthread_mutex_unlock(ss->mut);
    wprintf(exit_msg,id);
    //fclose(ss->fdst);
    return NULL;
}
int sort_big_mem(const struct ENTITY *ent,char *data,struct break_info *bi,long size)
{
    pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
    pthread_t pid[cpunum];
    long working[cpunum*2];
    bzero(&working[0],cpunum*2 * sizeof(long));
    struct Bag *bag;
    if(bi){
        bag=bag_with_array(bi->scope,bi->scope_cnt);
    } else {
        bag=bag_with_array(NULL,0);
    }
    sorts.ent=ent;
    sorts.jobs=bag;
    sorts.mut=&mut;
    sorts.cond=&cond;
    sorts.run=1;
    sorts.working=&working[0];
    sorts.data=data;
    for(int ix=0;ix<cpunum;ix++){
        pthread_create(&pid[ix],NULL,task,(void *)(long)ix);
    }
    while(1){
        pause();
        switch(action){
            case ACTION_INT:
                sorts.run= 0;
                goto outer_break;
            case ACTION_PROGRESS1:
            case ACTION_PROGRESS2:
            pthread_mutex_lock(&mut);
            bag_print(bag,stdout,10);
            pthread_mutex_unlock(&mut);
            for(int ix=0;ix<cpunum;ix++){
                long *pl=&working[ix*2];
                if(*pl  || *(pl+1)){
                    CP_MSG(L"任务:%2d [%8ld,%8ld]\n",ix,*pl,*(pl+1));
                }
            }
            default:
        }
    }
outer_break:
    for(int ix=0;ix<cpunum;ix++){
        pthread_join(pid[ix],NULL);
    }
    if(is_order(data,size,ent)){
        wprintf(L"OK\n");
    } else {
        wprintf(L"\033[0;35m没有正确排序\033[0m\n");
        return -1;
    }
    bag_free(bag);
    return 0;
}
long filesize(const char *);
int sort_part(const struct ENTITY *ent,struct APP_PATH *ap)
{
    const int us=ent->unitsize;
    struct break_info *bi;
    long total=filesize(ap->preform_dst);
    if(total > MEM_USE_MAX){
        WERROR(L"file size is biger than %ld(%ld) ",MEM_USE_MAX,total);
        return -1;
    }
    if(total % us){
        WERROR(L"data of index file '%s' is not align!",ap->preform_dst);
        return -1;
    }
    bi=break_info_load(ap->preform_break_info);
    if(bi == NULL){
        return -1;
    }

    // 索引的第一条记录是标识不能被移动
    for(int ix=0;ix<bi->scope_cnt;ix++){
        bi->scope[ix]=bi->scope[ix]-1;
    }
    break_info_print(bi);
    total=total - us;
    FILE *fdst=fopen(ap->preform_dst,"r");
    
    if(fdst==NULL){
        break_info_free(bi);
        WERR_NO_MSG(errno,L"打开文件错误%s",ap->preform_dst);
        return -1;
    }
    char *data=malloc(total);
    if(data==NULL){
        wprintf(L"无法处理的错误\n");
        exit(1);
    }
    fseek(fdst,us,SEEK_SET);
    size_t act= fread(data,1,total,fdst);
    fclose(fdst);
    if(act != total){
        free(data);
        break_info_free(bi);
        WERROR(L"不能完整读到数据%ld\n",act);
        return -1;
    }
    wprintf(L"完整读取排序数据\n");
    CP_MSG(L"总共:%ld\n",total/us);
    if(sort_big_mem(ent,data,bi,total/us)){
        werror_print();
    }
    free(data);
    break_info_free(bi);
    return 0;
}