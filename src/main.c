#include <locale.h>
int parse_cmd(int argc, char *argv[]);
int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    parse_cmd(argc, argv);
    return 0;
}