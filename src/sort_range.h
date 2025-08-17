#pragma once
#include <stdint.h>

struct RANGES;

struct RANGES *rng_new(void);
int rng_push(struct RANGES *rgs,uint64_t l1,uint64_t l2);
int rng_pop(struct RANGES *rgs,uint64_t *l1,uint64_t *l2);
void rng_release(struct RANGES *rgs);
void rng_print(struct RANGES *rgs,int limit);