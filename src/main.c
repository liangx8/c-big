#include <stdio.h>
#include <wchar.h>
#include <locale.h>

int test_main(void);
int main(int argc, char **argv)
{
    setlocale(LC_ALL,"");
    test_main();
    return 0;
}