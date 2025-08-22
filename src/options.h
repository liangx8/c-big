#ifndef OPTIONS_H_KDKFEW345NK
#define OPTIONS_H_KDKFEW345NK
struct OPTIONS{
    long action;
    unsigned long num;
    const char *str;
};
int parse_options(struct OPTIONS *,int, char **);
#define ACTION_RANDOM_DB 'r'
#define ACTION_MULT_SORT_DB 'm'
#define ACTION_TEST 't'
#define ACTION_SEARCH 's'
#define ACTION_SQLITE 'S'
#endif
