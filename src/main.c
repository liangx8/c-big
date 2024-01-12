#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <signal.h>
#include "entity.h"
#include "status.h"
#include "options.h"
#include "error_stack.h"
#include "timestamp.h"

//#include "test_item.h"

const char *config_file = "status.json";

int cpunum;


extern const struct ENTITY qq_entity;


void apart32(struct STATUS *);
void sort32(struct STATUS *);
void s32_apart_exam(struct STATUS *);
int sort_test(const char *);
void sighandler(int signum);
int list(const struct STATUS *, int64_t , int,int);
int gentestdata(const char *src, const char *dst, int64_t size);
void mem_sort_test(const char *);
void unit_test(void); // test_thread.c
void full_path(char *,const char *);
void seq_find(const char *fname,unsigned int val,int limit);
void test_signal(void);
void test_multi_write(void);
void test_qsort_partition(int64_t p1,int64_t p2);
void check_dup(const struct STATUS *);
int same_block(const unsigned char *src,const unsigned char *dst,int len);
void unit_run(const char *name);

int main(int argc,char *const argv[])
{
    pid_t pid = getpid();
    struct STATUS *stu;
    struct OPTION opt;
    char tmstr[64];
    int64_t tm, tm1;
    char *fns,*fnd;
    fns=malloc(256);
    fnd=malloc(256);

    // pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    // pthread_cond_t cond_print = PTHREAD_COND_INITIALIZER;
    //     pthread_cond_t ma = PTHREAD_COND_INITIALIZER;
    //     pthread_cond_t sa = PTHREAD_COND_INITIALIZER;
    // mrs.mutex = &mutex;
    // mrs.cond_signal = &cond_print;

    cpunum=sysconf(_SC_NPROCESSORS_ONLN);
    if (now(&tm) == -1)
    {
        print_error_stack(stdout);
    }
    timestamp_str(tmstr, tm);
    printf(
    "current timestamp:                    %ld\n\
time since Epoch 1970-01-01 00:00:00: %s\n\
pid:                                  \033[0;31;47m%d\033[0m\n\
cpu:                                  \033[1;35m%d\033[0m\n", tm, tmstr, pid,cpunum);
    stu = status_file_load_or_new(config_file);
    parse(argc, argv, &opt);
    stu->payload=&qq_entity;
    full_path(fnd,stu->dst);
    full_path(fns,stu->src);
    stu->preform_dst=fnd;
    stu->preform_src=fns;
    if(opt.srcname){
        stu->preform_dst=opt.srcname;
    }

    signal(SIGUSR1, sighandler);
    signal(SIGINT, sighandler);

    switch (opt.action)
    {
    case RESORT:
        stu->scope_cnt=0;
        free(stu->scope);
        stu->scope=NULL;
        stu->step=0;
        stu->step1time=0;
    case SORT:
        {
            if(stu->step <2 ){
                status_print(stu);
                if (now(&tm) == -1)
                {
                    print_error_stack(stdout);
                }
                apart32(stu);
                if(has_error()){
                    print_error_stack(stdout);
                    status_print(stu);
                    return 0;
                }
                if (now(&tm1) == -1)
                {
                    print_error_stack(stdout);
                    return -1;
                }
                stu->step1time += tm1-tm;
                if( status_save(stu, config_file)){
                    print_error_stack(stdout);
                    return -1;
                }
            } else {
                s32_apart_exam(stu);
            }
            // step1 完成
            // 如果返回不是0也不是-1，就是跳过
            status_print(stu);
#ifdef DNUM
            seq_find(stu->preform_dst,DNUM,10);
#endif
            /*
            if(stu->step ==1){
                printf("第一步被中断\n");
                return 0;
            }*/
            //sort32(stu);

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
            if(sort_test(stu->preform_dst)){
                print_error_stack(stdout);
            }
        }
        break;
    case LIST:
    {
        status_print(stu);
        if (list(stu, opt.offset, opt.limit,0))
        {
            print_error_stack(stdout);
        }
    }
        break;
    case UNIT_TEST:
        //test_mutex_lock();
        //test_pair();
        //test_multi_write();
        unit_run(opt.testname);
        //test_signal();
        //unit_test();
        break;
    case GEN_TEST:
    {
        // 用命令 -G 1000000
        if(opt.srcname){
            stu->preform_src=opt.srcname;
        } else {
            stu->preform_src="/home/com/big-data/qq-test.bin";
        }
        stu->preform_dst=fns;
        if (gentestdata(stu->preform_src, stu->preform_dst, opt.offset))
        {
            print_error_stack(stdout);
        }
    }
    break;
    case GEN_RAND_TEST:
        apart32(stu);
        status_save(stu,config_file);
    break;
    case FIND:
        if(opt.limit==0){
            opt.limit=20;
        }
        if(opt.srcname){
            stu->preform_dst=opt.srcname;
        }
        seq_find(stu->preform_dst,opt.offset,opt.limit);
    break;
    case TEST_PART:
        printf("begin:%ld,end: %ld\n",opt.offset,opt.limit);
        test_qsort_partition(opt.offset,opt.limit);
        return 0;
    case DUPLICATE:
        check_dup(stu);
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

