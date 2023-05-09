#ifndef PARALLEL_H_J23J4H3K24JHHJ33
#define PARALLEL_H_J23J4H3K24JHHJ33
struct PARALLEL;
struct PARALLEL *parall_create(void);
int parall_run(struct PARALLEL *pal, void *(*task)(void *), void *arg);
int parall_wait(struct PARALLEL *pal,void **,int *);
void print_ids(struct PARALLEL *);

#endif