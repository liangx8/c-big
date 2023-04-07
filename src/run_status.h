#ifndef RUN_STATUS_H_J3J432K34
#define RUN_STATUS_H_J3J432K34
struct run_status{
/* 
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t ma = PTHREAD_COND_INITIALIZER;
    pthread_cond_t sa = PTHREAD_COND_INITIALIZER;
 */
    long size;
    long pt_left,pt_right,pt_cnt;
    void *cdt;
};
#endif