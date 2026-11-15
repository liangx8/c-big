#ifndef PARSE_CMD_H
#define PARSE_CMD_H
typedef int (*wb_action)(int argc, char *argv[]);
typedef struct {
    const char *name;
    wb_action action;
} WB_CMD;
extern const WB_CMD wb_cmds[];
#endif // PARSE_CMD_H