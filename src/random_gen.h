#ifndef RANDOM_GEN_H_DKJJE33JJ3
#define RANDOM_GEN_H_DKJJE33JJ3
#ifdef linux
void random_init(void);
long random_long(void);
void random_close(void);
#else
#define random_init() do{}while(0)
#define random_close() do{}while(0)
long random_long(void);
#endif
#endif