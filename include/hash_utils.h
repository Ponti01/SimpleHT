#ifndef HASH_UTILS_H
#define HASH_UTILS_H

int compar_str(const void *a, const void *b);
int compar_int(const void *a, const void *b);
unsigned long hash_fnv1a_int(const void *key);
unsigned long hash_fnv1a_str(const void *key);


#endif
