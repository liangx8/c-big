#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <locale.h>
#include "entity.h"
#include "options.h"
#include "error_stack.h"
#include "timestamp.h"

//#include "test_item.h"

int app_nameid(struct OPTION *);

struct APP_MAP{
    const char *name;
    int (*app)(struct OPTION *);
} ;
const struct APP_MAP amap[]={
    {"nameid",app_nameid},
    {0,0}
};


int cpunum;

void sighandler(int signum);
void full_path(char *,const char *);


void sighandler(int sig)
{}
int main(int argc,char *const argv[])
{
    pid_t pid = getpid();
    struct OPTION opt;
    char tmstr[64];
    int64_t tm;
    setlocale(LC_ALL,"zh_CN.UTF-8");
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

    parse(argc, argv, &opt);
    

    signal(SIGUSR1, sighandler);
    //signal(SIGINT, sighandler);
    signal(SIGUSR2,sighandler);

    int ix=0;
    if(opt.app){
        while(amap[ix].app){
            if (strcmp(opt.app,amap[ix].name)==0){
                if (amap[ix].app(&opt)){
                    print_error_stack(stdout);
                    return -1;
                }
                return 0;
            }
            ix ++;
        }
    }
    
    return 0;

}

