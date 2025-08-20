#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include "abstract_db.h"
#include "log.h"

#define DB_SIZE 20

struct ENV env;
struct ABSTRACT_DB* random_db(long size);
void mem_quick_sort(struct ABSTRACT_DB *db);
void mem_mult_quick_sort(struct ABSTRACT_DB *);
void test_run(const char *);
struct ABSTRACT_DB* sample_db(uint64_t);

int main(int argc, char **argv)
{
    setlocale(LC_ALL,"");
    env.cpunum=sysconf(_SC_NPROCESSORS_ONLN);
    env.pid=getpid();
    log_info(L"cpu number: %d\n",env.cpunum);
    log_info(L"Task \033[0;31;47m%d\033[0m\n",env.pid);
    if(argc > 1){
        if(strncmp("test",argv[1],5)){
            wprintf(L"不知道做什么\n");
            return -1;
        } else {
            test_run(NULL);
            return 0;
        }
    }
    struct ABSTRACT_DB *db=sample_db(6000000);
    //struct ABSTRACT_DB *db=random_db(DB_SIZE);
    mem_mult_quick_sort(db);
    for(int ix=0;ix<DB_SIZE;ix++){
        db->entity->print(db,ix);
    }
    if(db->entity->close(db)){
        log_info(L"关闭数据库错误\n");
        return -1;
    }

    log_info(L"program exit\n");
    return 0;
}