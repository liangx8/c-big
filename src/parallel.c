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
int parall_wait(struct PARALLEL *pel, void **retval)
{

    for (int ix = 0; ix < pel->task_num; ix++)
    {
        int res;
        res = pthread_join(pel->ids[ix], retval);
        if (res)
        {
            ERROR_BY_ERRNO();
            return -1;
        }
        if (retval != NULL)
            retval++;
    }
    free(pel);
    return 0;
}