#ifndef OPTIONS_H_KWBBW323
#define OPTIONS_H_KWBBW323
struct OPTION
{
    long offset;
    const char *srcname;
    const char *testname;
    int action;
    long limit;
};
#define SORT         0
#define TEST         1
#define LIST         2
#define FIND         3
#define GEN_TEST     4
#define UNIT_TEST    5
#define RESORT       6
#define GEN_RAND_TEST 7
#define TEST_PART    8
#define DUPLICATE    9
void parse(int argc, char *const argv[], struct OPTION *data);
void usage(const char*);
#endif