#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include "status.h"
#include "options.h"
#include "error_stack.h"
#include "timestamp.h"

const char *config_file = "status.json";


int bigsort(struct STATUS *);

int list(const char *fname, int64_t offset, int limit);

void test_thread(void);
int status_save(struct STATUS *stu,const char *fname);
int main(int argc, char **argv)
{
    pid_t pid = getpid();
    struct STATUS stu;
    struct OPTION opt;
    char tmstr[30];
    int64_t tm,tm1;
    if (now(&tm) == -1){
        print_error_stack(stdout);
    }
    
    timestamp_str(tmstr,tm);
    printf("current timestamp %ld\ntime since Epoch 1970-01-01 00:00:00:%s\npid:%d\n",tm,tmstr, pid);
    if(config_parse(&stu, config_file)){
        print_error_stack(stdout);
        return -1;
    }
    parse(argc, argv, &opt);
    printf("value:%lu\n", sizeof(int));
    switch (opt.action)
    {
    case SORT:
        if (bigsort(&stu))
        {
            print_error_stack(stdout);
        }
        break;
    case TEST:
        //test_thread();
        if(status_save(&stu,config_file)){
            print_error_stack(stdout);
        }
        break;
    case LIST:
        if (list(stu.src, opt.offset, opt.limit))
        {
            print_error_stack(stdout);
        }
        break;
    default:
        printf("unexpect return!\n");
    }
    if (now(&tm1) == -1){
        print_error_stack(stdout);
    }
    timestamp_str(tmstr,tm1-tm);
    printf("run:%s\n",tmstr);

    return 0;
}
