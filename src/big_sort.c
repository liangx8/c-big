#include <stdio.h>
#include <wchar.h>
#include <malloc.h>
#include <pthread.h>
#include "error_stack.h"
#include "memdb.h"
#include "sort_range.h"

struct sort_mgr{

    pthread_cond_t  *cond;
    pthread_mutex_t *mutex;
};
struct sort_mgr smr;
void signalhandler(int signum)
{
    CP_MSG(L"signal number %d\n",signum);
    pthread_cond_signal(smr.cond);
}

void *task(void *arg)
{
    return NULL;
}
/**
 * @brief sort hurge amount of data in memory
 * @param cfgname 包含有多少单元要排序，目标文件名
 * @param cmp compare function
 */
void bigsort(struct MEMDB *db)
{
    //db->raw
    //db->total
    pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    smr.cond=&cond;
    smr.mutex=&mutex;

    long l1,l2;
    rng_pop(db->scops,&l1,&l2);
    CP_MSG(L"%ld,%ld\n",l1,l2);
    pthread_cond_wait(&cond,&mutex);
    CP_MSG(L"continue%s","\n");

}