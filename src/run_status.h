#ifndef RUN_STATUS_H_J3J432K34
#define RUN_STATUS_H_J3J432K34
struct apart_status{
/* 
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t ma = PTHREAD_COND_INITIALIZER;
    pthread_cond_t sa = PTHREAD_COND_INITIALIZER;
 */
    long size;
    long pt_left,pt_right,pt_cnt;
    int pid;
    int runing;
};
struct sort_status{
};


#define RUNING_BREAK 0
#define RUNING_PRINT 1
#define RUNING_KEEP  2
#endif