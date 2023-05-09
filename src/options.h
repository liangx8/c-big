#ifndef OPTIONS_H_KWBBW323
#define OPTIONS_H_KWBBW323
struct OPTION
{
    int action;
    int limit;
    unsigned int target;
    long offset;
};
#define SORT 0
#define TEST 1
#define LIST 2
#define FIND 3
#define GEN_TEST 4
#define UNIT_TEST 5
void parse(int argc, char *const argv[], struct OPTION *data);
#endif