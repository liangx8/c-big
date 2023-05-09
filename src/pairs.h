#ifndef PAIRS_H_3K34JK33L
#define PAIRS_H_3K34JK33L
struct Pairs;
struct Pairs *pairs_create      (void);
struct Pairs *pairs_with_array  (const long *,int);
void pairs_free                 (struct Pairs *);
int pairs_is_empty              (struct Pairs *);
int pairs_get                   (struct Pairs *,long **);
int pairs_put                   (struct Pairs *,long,long);
int pairs_print                (struct Pairs *,void *out,int limit);

struct MutexPairs;
struct MutexPairs *mutex_create     (void);
struct MutexPairs *mutex_with_array (const long *,int);
void mutex_free                     (struct MutexPairs *);
int mutex_is_empty                  (struct MutexPairs *);
int mutex_get                       (struct MutexPairs *,long **);
int mutex_put                       (struct MutexPairs *,long,long);
int mutex_print                    (struct MutexPairs *,void *out,int limit);

#endif