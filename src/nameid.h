#ifndef NAMEID_H_JJ3J4KJ2LK34
#define NAMEID_H_JJ3J4KJ2LK34

struct ID_INDEX;
struct NAMEID;
struct NAMEID *new_nameid(const char *);
void nameid_close(struct NAMEID *);

int nameid_print1(struct NAMEID *,void *);
int nameid_verify(struct NAMEID *,void *);
long nameid_next(struct NAMEID *);



struct ID_INDEX *nameid_id_index(struct NAMEID *);
#endif