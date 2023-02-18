#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "error_stack.h"
#include "parallel.h"
#define TASK_NUM 5

struct run_data
{
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    long uid;
    int *pcnt, *busy;
};

int run;

void *task(void *pl)
{
    struct run_data *rd = pl;
    int res;
    while (run)
    {
        sleep(1+rd->uid);
        res = pthread_mutex_lock(rd->mutex);
        if (res)
        {
            printf("mutex error:%s %d", __FILE__, __LINE__);
        }
        if (*rd->busy == 0)
        {
            printf("task %ld: %d\n", rd->uid, *rd->pcnt);
            (*rd->pcnt)++;
            *rd->busy = 1;

            res = pthread_cond_signal(rd->cond);
            if (res)
            {
                printf("mutex error:%s %d", __FILE__, __LINE__);
            }
        } else {
            // 需要 pthread_mutex_wait(),可避免轮询rd->busy以释放CPU
        }
        res = pthread_mutex_unlock(rd->mutex);
        if (res)
        {
            printf("mutex error:%s %d", __FILE__, __LINE__);
        }
    }
    return NULL;
}
void sig_usr1_handler(int sig)
{
    printf("USR1 信号\n");
    run = 0;
}
void test_thread(void)
{
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    struct run_data rd[TASK_NUM];
    struct PARALLEL *pel=parall_create();
    int res;
    int cnt = 0;
    int busy;
    run = 1;
    signal(SIGUSR1, sig_usr1_handler);

    res = pthread_mutex_lock(&mutex);
    if (res)
    {
        print_error_stack(stdout);
        return;
    }

    for (int ix = 0; ix < TASK_NUM; ix++)
    {
        rd[ix].mutex = &mutex;
        rd[ix].cond = &cond;
        rd[ix].uid = ix;
        rd[ix].pcnt = &cnt;
        rd[ix].busy = &busy;
        res = parall_run(pel,task,&rd[ix]);
        if (res)
        {
            print_error_stack(stdout);
            return;
        }
    }
    
    printf("master start:\n");
    while (run)
    {
        busy = 0;
        printf("master: %d\n", cnt);
        cnt++;
        res = pthread_cond_wait(&cond, &mutex);
        if (res)
        {
            printf("mutex error:%s %d", __FILE__, __LINE__);
        }
        if (cnt > 100000)
        {
            run = 0;
        }
    }
    res = pthread_mutex_unlock(&mutex);
    if (res)
    {
        printf("mutex error:%s %d", __FILE__, __LINE__);
    }
    parall_wait(pel,NULL);
}
