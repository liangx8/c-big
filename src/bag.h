#ifndef BAG_H_KE3KJJ32
#define BAG_H_KE3KJJ32

typedef void * BAG;
BAG bag_create(void);
void bag_free(BAG);

int bag_put(BAG,void *);
void * bag_get(BAG);
int bag_size(BAG);
#endif