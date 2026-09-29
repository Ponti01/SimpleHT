#ifndef HASH_UTILS_H
#define HASH_UTILS_H

#include <stdint.h>

int compar_str(const void *a, const void *b);
int compar_int(const void *a, const void *b);
uint64_t hash_fnv1a_int(const void *key);
uint64_t hash_fnv1a_str(const void *key);

#endif
