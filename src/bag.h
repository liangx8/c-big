#ifndef BAG_H_KE3KJJ32
#define BAG_H_KE3KJJ32

struct BAG;
struct BAG *bag_create(void);
void bag_free(struct BAG*);

int bag_put(struct BAG*,void *);
void * bag_get(struct BAG*);
int bag_size(struct BAG*);
#endif