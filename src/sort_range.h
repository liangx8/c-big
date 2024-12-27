#pragma once

struct RANGES;

struct RANGES *rng_new(void);
void rng_push(struct RANGES *rgs,long l1,long l2);
int rng_pop(struct RANGES *rgs,long *l1,long *l2);
int rng_release(struct RANGES *rgs);
int rng_print(struct RANGES *rgs,int limit);