#include <signal.h>

struct {
    int tri;
    int sig;
} sigdata;

void sighdl(int sig){
    sigdata.tri=1;
    sigdata.sig=sig;
}
void signal_init(void)
{
    signal(SIGUSR1,sighdl);
    signal(SIGUSR2,sighdl);
    signal(SIGINT,sighdl);
}
int new_signal(int *sig){
    if(sigdata.tri){
        sigdata.tri=0;
        *sig=sigdata.sig;
        return -1;
    }
    return 0;
}