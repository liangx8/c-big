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
int is_sorted(FILE *fh,long offset,long total,const struct ENTITY *ent);
void sighandler(int signum);
int list(const struct STATUS *, int64_t , int,int);
int gentestdata(const char *src, const char *dst, int64_t size);
void mem_sort_test(const char *);
void unit_test(void); // test_thread.c
void full_path(char *,const char *);
void seq_find(const struct STATUS *,unsigned int val,int limit);
void test_signal(void);
void test_multi_write(void);
int same_block(const unsigned char *src,const unsigned char *dst,int len);
void unit_run(const char *name);
long filesize(const char *);
int makedata(const char *dst,int64_t size);
off_t bfind(const char *dst,unsigned long ival,const struct ENTITY *ent);


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
    if(stu->src[0]=='/'){
        stu->preform_src=stu->src;
    } else {
        full_path(fns,stu->src);
        stu->preform_src=fns;
    }
    if(stu->dst[0]=='/'){
        stu->preform_dst=stu->dst;
    }else {
        full_path(fnd,stu->dst);
        stu->preform_dst=fnd;
    }
    if(opt.srcname){
        stu->preform_dst=opt.srcname;
    }

    signal(SIGUSR1, sighandler);
    signal(SIGINT, sighandler);
    signal(SIGUSR2,sighandler);

    switch (opt.action)
    {
    case RESORT:
        stu->scope_cnt=0;
        free(stu->scope);
        stu->scope=NULL;
        stu->step=0;
    case SORT:
        {
            if(stu->step <2 ){
                stu->step1time=0;
                stu->step2time=0;
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
                //s32_apart_exam(stu);
                stu->step=2;
                status_print(stu);
                if( status_save(stu, config_file)){
                    print_error_stack(stdout);
                    return -1;
                }
            }
            if(stu->scope_cnt){
                if (now(&tm) == -1)
                {
                    print_error_stack(stdout);
                }
                sort32(stu);
                if (now(&tm1) == -1)
                {
                    print_error_stack(stdout);
                    return -1;
                }
                stu->step2time += tm1-tm;
                if( status_save(stu, config_file)){
                    print_error_stack(stdout);
                    return -1;
                }
                status_print(stu);
            } else {
                timestamp_str(tmstr,stu->step1time+stu->step2time);
                printf("已经排序，用时 %s\n",tmstr);
            }

            /*
            if(stu->step ==1){
                printf("第一步被中断\n");
                return 0;
            }*/
            //sort32(stu);

        }
        break;
    case IS_SORTED:
//int is_sorted(FILE *fh,long offset,long total,const struct ENTITY *ent);
        {
            long total;
            
            if(opt.limit < 0){
                total=filesize(stu->preform_dst)/qq_entity.unitsize;
            } else {
                total=opt.offset+opt.limit;
            }
            printf("检测数据%s(%ld,%ld)...\n",stu->preform_dst,opt.offset,total);
            FILE *fh=fopen(stu->preform_dst,"r");
            if(fh==NULL){
                ERROR_BY_ERRNO();
            } else {
                is_sorted(fh,opt.offset,total-opt.offset,stu->payload);
                fclose(fh);
            }
            if(has_error()){
                print_error_stack(stderr);
            } else {
                printf("数据库已经排序\n");
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
        printf("target file %s\n",stu->preform_src);
        if(makedata(stu->preform_src,opt.offset)){
            print_error_stack(stderr);
        }
    break;
    case FIND:
        if(opt.limit==0){
            opt.limit=20;
        }
        if(opt.srcname){
            stu->preform_dst=opt.srcname;
        }
        
        long pos=bfind(stu->preform_dst,opt.offset,&qq_entity);
        if(pos < 0){
            print_error_stack(stderr);
        } else {
            if (list(stu, pos-opt.limit/2, opt.limit,pos))
            {
                print_error_stack(stdout);
            }

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

