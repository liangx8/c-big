#ifndef BAG_H_3K34JK33L
#define BAG_H_3K34JK33L
struct Bag;
struct Bag *bag_create      (void);
struct Bag *bag_with_array  (const long *,int);
void bag_free               (struct Bag *);
int bag_get                 (struct Bag *,long *);
int bag_put2                (struct Bag *,long,long);
int bag_print               (struct Bag *,void *out,int limit);
int bag_put                 (struct Bag *,long);
int bag_exist               (struct Bag *,long);

/*
struct MutexPairs;
struct MutexPairs *mutex_create     (void);
struct MutexPairs *mutex_with_array (const long *,int);
void mutex_free                     (struct MutexPairs *);
int mutex_get                       (struct MutexPairs *,long **);
int mutex_put                       (struct MutexPairs *,long,long);
int mutex_print                     (struct MutexPairs *,void *out,int limit);
*/
#define EMPTY -2
#endif