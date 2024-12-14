#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include "error_stack.h"


int test_main(void);
void bigsort(const char *cfgname);

int main(int argc, char **argv)
{
    error_init();
    setlocale(LC_ALL,"");
    bigsort("/home/com/big-data/chinaid/nameid.bin");
    if(has_error()){
        print_error_stack(stdout);
    }
    error_release();
    return 0;
}