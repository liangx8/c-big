#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <signal.h>
#include "status.h"
#include "options.h"
#include "error_stack.h"
#include "timestamp.h"

const char *config_file = "status.json";
const char *test_src = "big/qq-test.bin";
const char *origin = "/home/com/big-data/qq-phone.bin";

int cpunum;

void apart32(struct STATUS *);
void sort32(struct STATUS *);
int s32_apart_exam(struct STATUS *);
int sort_test(const char *);
void sighandler(int signum);
int list(const char *fname, int64_t offset, int limit);
int gentestdata(const char *src, const char *dst, int64_t size);
void mem_sort_test(const char *);
void unit_test(void); // test_thread.c
void full_path(char *,const char *);
int main(int argc,char *const argv[])
{
    pid_t pid = getpid();
    struct STATUS *stu;
    struct OPTION opt;
    char tmstr[64];
    int64_t tm, tm1;
    // pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    // pthread_cond_t cond_print = PTHREAD_COND_INITIALIZER;
    //     pthread_cond_t ma = PTHREAD_COND_INITIALIZER;
    //     pthread_cond_t sa = PTHREAD_COND_INITIALIZER;
    // mrs.mutex = &mutex;
    // mrs.cond_signal = &cond_print;

    if (now(&tm) == -1)
    {
        print_error_stack(stdout);
    }
    cpunum=sysconf(_SC_NPROCESSORS_ONLN);
    timestamp_str(tmstr, tm);
    printf(
    "current timestamp:                    %ld\n\
time since Epoch 1970-01-01 00:00:00: %s\n\
pid:                                  \033[0;31;47m%d\033[0m\n\
cpu:                                  \033[1;35m%d\033[0m\n", tm, tmstr, pid,cpunum);
    stu = status_file_load_or_new(config_file);
    parse(argc, argv, &opt);

    signal(SIGUSR1, sighandler);
    signal(SIGINT, sighandler);

    switch (opt.action)
    {
    case RESORT:
        stu->scope_cnt=0;
        free(stu->scope);
        stu->scope=NULL;
        stu->step1progress=0;
    case SORT:
        {
            apart32(stu);
            if(has_error()){
                print_error_stack(stdout);
                return 0;
            }
            // step1 完成
            // 如果返回不是0也不是-1，就是跳过
            if (now(&tm1) == -1)
            {
                print_error_stack(stdout);
                return -1;
            }
            stu->step1time +=tm1-tm;
            if( status_save(stu, config_file)){
                print_error_stack(stdout);
                return -1;
            }
            status_print(stu);
            timestamp_str(tmstr, stu->step1time);
            printf("第一步用时:%s\n", tmstr);
            sort32(stu);

        }
        break;
    case TEST:
        // test_thread();
        // status_print(stu);
        // if(status_save(&stu,config_file)){
        //     print_error_stack(stdout);
        // }
        /*
        {
            int64_t scope[]={1,2,3,4,5,6,7,8,9,10};
            stu->scope=scope;
            stu->scope_cnt=10;
            status_save(stu,config_file);

        }*/
        {

            int res=s32_apart_exam(stu);
            if(res<0){
                print_error_stack(stdout);
                return -1;
            }
            if(res){
                printf("%s 没有被正确的分区\n",stu->dst);
                return -1;
            }
            printf("apart test OK!\n");
            char *fn=malloc(255);
            full_path(fn,stu->dst);
            if(sort_test(fn)){
                print_error_stack(stdout);
                return -1;
            }
            free(fn);
            status_print(stu);
        }
        break;
    case LIST:
    {
        char *fn;
        fn = malloc(255);
        full_path(fn,stu->dst);

        if (list(fn, opt.offset, opt.limit))
        {
            print_error_stack(stdout);
        }
        free(fn);
    }
        break;
    case UNIT_TEST:
        //test_mutex_lock();
        //test_pair();
{
        char *fn;
        fn = malloc(255);
        full_path(fn,stu->dst);

        mem_sort_test(fn);
        free(fn);
}
        //unit_test();
        
        break;
    case GEN_TEST:
    {
        // 用命令 -G 1000000
        char *fn;
        fn = malloc(255);
        full_path(fn,test_src);
        if (gentestdata(origin, fn, opt.offset))
        {
            print_error_stack(stdout);
        }
        free(fn);
    }
    break;
    default:
        usage(argv[0]);
        return 0;
    }
    if (now(&tm1) == -1)
    {
        print_error_stack(stdout);
    }
    timestamp_str(tmstr, tm1 - tm);
    printf("run:%s\n", tmstr);
    status_free(stu);
    return 0;
}
int int_act;
void sighandler(int signum)
{
    switch (signum)
    {
    case SIGUSR1:
        int_act = SHOW_PROGRESS;
        break;
    case SIGINT:
        int_act = SORTING_BREAK;
    }
}
/**
 * @brief 为了防止程序优化而设计的设值函数
 * @param target 指针
 * @param value 新值
 * @return 旧值
*/
int set_int(int *target,int value)
{
    int retval=*target;
    *target=value;
    return retval;
}