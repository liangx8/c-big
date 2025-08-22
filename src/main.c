#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include "options.h"
#include "abstract_db.h"
#include "log.h"

#define DB_SIZE 20

struct ENV env;
struct ABSTRACT_DB* random_db(long size);
void mem_quick_sort(struct ABSTRACT_DB *db);
void mem_mult_quick_sort(struct ABSTRACT_DB *);
void test_run(const char *);
struct ABSTRACT_DB* sample_db(uint64_t);
void sm_search(uint64_t val);
void sqlite_exec(int);
void sqlite_prepare(int);
int main(int argc, char **argv)
{
    setlocale(LC_ALL,"");
    env.cpunum=sysconf(_SC_NPROCESSORS_ONLN);
    env.pid=getpid();
    log_info(L"cpu number: %d\n",env.cpunum);
    log_info(L"Task \033[0;31;47m%d\033[0m\n",env.pid);
    struct OPTIONS opts;
    if(parse_options(&opts,argc,argv)){
        return -1;
    }
    switch(opts.action){
        case ACTION_TEST:
        if(strncmp("all",opts.str,4)){
            test_run(opts.str);
        } else {
            test_run(NULL);
        }
        break;
        case ACTION_RANDOM_DB:
        {
            struct ABSTRACT_DB *db=random_db(opts.num);
            mem_quick_sort(db);
            for(int ix=0;ix<opts.num;ix++){
                db->entity->print(db,ix,ix);
            }
            db->entity->close(db);
        }
        break;
        case ACTION_MULT_SORT_DB:
        {
            struct ABSTRACT_DB *db=sample_db(opts.num);
            mem_mult_quick_sort(db);
            for(int ix=0;ix<DB_SIZE;ix++){
                db->entity->print(db,ix,ix);
            }
            if(db->entity->close(db)){
                log_info(L"关闭数据库错误\n");
                return -1;
            }
        }
        case ACTION_SEARCH:
            log_info(L"0x%016lx,%lu\n",opts.num,opts.num);
            sm_search(opts.num);
            break;
        case ACTION_SQLITE:
            //sqlite_exec(opts.num);
            sqlite_prepare(opts.num);
    }

    log_info(L"program exit\n");
    return 0;
}