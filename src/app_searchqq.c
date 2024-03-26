#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "entity.h"
#include "status.h"
#include "options.h"
#include "error_stack.h"
#include "timestamp.h"


extern const struct ENTITY qq_entity;
extern const char *config_file;

void apart32(struct STATUS *);
void sort32(struct STATUS *);
int is_sorted(FILE *fh,long offset,long total,const struct ENTITY *ent);
int list(const struct STATUS *,int64_t, int64_t ,int);
int gentestdata(const char *src, const char *dst, int64_t size);
void mem_sort_test(const char *);
void seq_find(const struct STATUS *,unsigned int val,int limit);
void test_multi_write(void);
int same_block(const unsigned char *src,const unsigned char *dst,int len);
void unit_run(const char *name);
long filesize(const char *);
int makedata(const char *dst,int64_t size);
off_t bfind(const char *dst,unsigned long ival,const struct ENTITY *ent);


int search_qq(struct STATUS *stu,struct OPTION *opt)
{
    int64_t tm,tm1;
    char tmstr[64];
    stu->payload=&qq_entity;

    switch (opt->action)
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
            
            if(opt->limit < 0){
                total=filesize(stu->preform_dst)/qq_entity.unitsize;
            } else {
                total=opt->offset+opt->limit;
            }
            printf("检测数据%s(%ld,%ld)...\n",stu->preform_dst,opt->offset,total);
            FILE *fh=fopen(stu->preform_dst,"r");
            if(fh==NULL){
                ERROR_BY_ERRNO();
            } else {
                is_sorted(fh,opt->offset,total-opt->offset,stu->payload);
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
        if (list(stu,0, opt->offset, opt->limit))
        {
            print_error_stack(stdout);
        }
    }
        break;
    case UNIT_TEST:
        //test_mutex_lock();
        //test_pair();
        //test_multi_write();
        unit_run(opt->testname);
        break;
    case GEN_TEST:
    {
        // 用命令 -G 1000000
        const char *fns=stu->preform_src;
        if(opt->srcname){
            stu->preform_src=opt->srcname;
        } else {
            stu->preform_src="/home/com/big-data/qq-test.bin";
        }
        stu->preform_dst=fns;
        if (gentestdata(stu->preform_src, stu->preform_dst, opt->offset))
        {
            print_error_stack(stdout);
        }
    }
    break;
    case GEN_RAND_TEST:
        printf("target file %s\n",stu->preform_src);
        if(makedata(stu->preform_src,opt->offset)){
            print_error_stack(stderr);
        }
    break;
    case FIND:
        if(opt->limit==0){
            opt->limit=20;
        }
        if(opt->srcname){
            stu->preform_dst=opt->srcname;
        }
        
        long pos=bfind(stu->preform_dst,opt->offset,&qq_entity);
        if(pos < 0){
            print_error_stack(stderr);
        } else {
            printf("==============<%ld\n",pos);
            off_t start=pos-opt->limit/2;
            if (start<0)start=0;
            if (list(stu, pos,start, opt->limit))
            {
                print_error_stack(stdout);
            }

        }
    break;
    default:
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