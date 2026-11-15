// #include <unistd.h>
// #include <stdio.h>
#include "parse_cmd.h"
#include <string.h>
#include <libgen.h>

#include <wchar.h>
int parse_cmd(int argc, char *argv[]) {
    char *cmd_name = argv[0];

    for(int i = 0; wb_cmds[i].name != NULL; i++) {
        if(strcmp(basename(cmd_name), wb_cmds[i].name) == 0) {
            return wb_cmds[i].action(argc, argv);
        }
    }
    if(argc == 1) {
        wprintf(L"Unknown command: %s\n", cmd_name);
        return -1;
    }
    cmd_name=argv[1];
    for(int i = 0; wb_cmds[i].name != NULL; i++) {
        if(strcmp(cmd_name, wb_cmds[i].name) == 0) {
            return wb_cmds[i].action(argc-1, &argv[1]);
        }
    }
    wprintf(L"Unknown command: %s\n", cmd_name);
    return -1;
}