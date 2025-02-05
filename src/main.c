#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>
#include <signal.h>
#include "memdb.h"
#include "error_stack.h"

struct ENV env;

void signalhandler(int signum);
//int test_main(void);

void sort_onethread(struct MEMDB *db);
struct MEMDB* loaddb(const char *fname);
struct MEMDB* random_db(long size);
void bigsort(struct MEMDB *db);
int main(int argc, char **argv)
{
    struct MEMDB *db;
    error_init();
    setlocale(LC_ALL,"");
    env.cpunum=sysconf(_SC_NPROCESSORS_ONLN);
    env.pid=getpid();
    wprintf(L"cpu number: %d\n       PID: \033[0;31;47m%d\033[0m\n",env.cpunum,env.pid);
    if(argc>1){
        if(argv[1][0]=='t'){
            wprintf(L"测试\n");
            db=random_db(15);
            sort_onethread(db);
            goto fin;
        }
        db=random_db(1000000);

    } else {
        db=loaddb("/home/com/big-data/chinaid/nameid.bin");
    }
    if(has_error()){
        print_error_stack(stdout);
        error_release();
        return -1;
    }
    signal(SIGUSR1,signalhandler);
    signal(SIGUSR2,signalhandler);
    signal(SIGINT,signalhandler);
    bigsort(db);
    for(long lx=0;lx<30;lx++)
        db->entity->print(db->payload,lx);
fin:
    db->entity->close(db->payload);
    error_release();
    wprintf(L"program exit\n");
    return 0;
}