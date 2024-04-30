#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <locale.h>
#include "entity.h"
#include "status.h"
#include "options.h"
#include "error_stack.h"
#include "timestamp.h"

//#include "test_item.h"

int search_qq (struct STATUS *,struct OPTION *);
int app_nameid(struct STATUS *,struct OPTION *);

struct APP_MAP{
    const char *name;
    int (*app)(struct STATUS *,struct OPTION *);
} ;
const struct APP_MAP amap[]={
    {"searchqq",search_qq},
    {"nameid",app_nameid},
    {0,0}
};

const char *config_file = "status.json";

int cpunum;

void sighandler(int signum);
void full_path(char *,const char *);

int main(int argc,char *const argv[])
{
    pid_t pid = getpid();
    struct STATUS *stu;
    struct OPTION opt;
    char tmstr[64];
    int64_t tm;
    char *fns,*fnd;
    setlocale(LC_ALL,"zh_CN.UTF-8");
    fns=malloc(256);
    fnd=malloc(256);
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
    //signal(SIGINT, sighandler);
    signal(SIGUSR2,sighandler);

    int ix=0;
    if(opt.app){
        while(amap[ix].app){
            if (strcmp(opt.app,amap[ix].name)==0){
                if (amap[ix].app(stu,&opt)){
                    print_error_stack(stdout);
                    return -1;
                }
                return 0;
            }
            ix ++;
        }
    }
    
    return search_qq(stu,&opt);

}

