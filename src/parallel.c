#include <pthread.h>
#include <malloc.h>
#include "error_stack.h"
#define TASK_MAX 32
struct PARALLEL
{
    unsigned long ids[TASK_MAX];
    int task_num;
};

struct PARALLEL *parall_create(void)
{
    struct PARALLEL *pa = malloc(sizeof(struct PARALLEL));
    pa->task_num = 0;
    return pa;
}
int parall_run(struct PARALLEL *pel, void *(*task)(void *), void *arg)
{
    int res;
    if (pel->task_num == TASK_MAX)
    {
        ERROR("task reached max number!");
        return -1;
    }
    res = pthread_create(&(pel->ids[pel->task_num]), NULL, task, arg);
    if (res)
    {
        ERROR_BY_ERRNO();
        return -1;
    }
    pel->task_num++;
    return 0;
}
int parall_wait(struct PARALLEL *pel,void *rets[],int *retnum)
{

    for (int ix = 0; ix < pel->task_num; ix++)
    {
        int res;
        if(pel->task_num<=*retnum){
            *retnum=pel->task_num;
        }
        if(rets && ix< *retnum){
            res = pthread_join(pel->ids[ix], &rets[ix]);
        } else {
            res = pthread_join(pel->ids[ix], NULL);
        }
        if (res)
        {
            ERROR_BY_ERRNO();
            return -1;
        }
    }
    free(pel);
    return 0;
}
void print_ids(struct PARALLEL *pel)
{
    for(int i=0;i<pel->task_num;i++){
        printf("[%lu]",pel->ids[i]);
    }
    printf("\n");
}