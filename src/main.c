#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>
#include "abstract_db.h"
#include "error_stack.h"

#define DB_SIZE 20

struct ENV env;
struct ABSTRACT_DB* random_db(long size);
void mem_quick_sort(struct ABSTRACT_DB *db);
void mem_mult_quick_sort(struct ABSTRACT_DB *);
void test_run(const char *);
void generate_db(size_t size,void (*progress)(int));
struct ABSTRACT_DB* sample_db(void);
void pgr(int prst)
{
    wprintf(L"\r正在生成%3d%",prst);
    fflush(stdout);
}
int main(int argc, char **argv)
{
    setlocale(LC_ALL,"");
    env.cpunum=sysconf(_SC_NPROCESSORS_ONLN);
    env.pid=getpid();
    log_info(L"cpu number: %d\n",env.cpunum);
    log_info(L"Task \033[0;31;47m%d\033[0m\n",env.pid);
    if(argc > 1){
        if(strncmp("test",argv[1],5)){
            if(strncmp("gen",argv[1],4)){
                wprintf(L"不知道做什么\n");
                return -1;
            }
            generate_db(50000000,pgr);
            //generate_db(40000000,pgr);
            return 0;
        } else {
            test_run(NULL);
            return 0;
        }
    }
    error_init();
    struct ABSTRACT_DB *db=sample_db();
    //struct ABSTRACT_DB *db=random_db(DB_SIZE);
    mem_mult_quick_sort(db);
    for(int ix=0;ix<DB_SIZE;ix++){
        db->entity->print(db,ix);
    }
    if(has_error()){
        print_error_stack(stdout);
        error_release();
        return -1;
    }
    error_release();
    for(int ix=0;ix<DB_SIZE;ix++){
        db->entity->print(db,ix);
    }
    db->entity->close(db);

    log_info(L"program exit\n");
    return 0;
}