#include "parse_cmd.h"
#ifndef NULL
#define NULL ((void*)0)
#endif
int rename_action(int argc, char *argv[]);
const WB_CMD wb_cmds[] = {
    {"rename", rename_action},
    {NULL, NULL}
};