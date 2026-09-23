#include <string.h>

#include "hash_utils.h"

int compar_int(const void *a, const void *b) {
	int ai = *(const int*)a;
	int bi = *(const int*)b;

	if (ai < bi) return -1;
	if (ai > bi) return 1;
	
	return 0;
}

int compar_str(const void *a, const void *b) {
	return (strcmp((const char*)a, (const char*)b));
}

#define FNV_OFFSET 0xcbf29ce484222325UL
#define FNV_PRIME  0x100000001b3UL

unsigned long hash_fnv1a_str(const void *key) {
	unsigned long hash = FNV_OFFSET;
	unsigned char *p = (unsigned char*)key;

	while (*p) {
		hash ^= *p;
		hash *= FNV_PRIME;
		p++;
	}
	return hash;
}

unsigned long hash_fnv1a_int(const void *key) {
	unsigned long hash = FNV_OFFSET;
	unsigned int *p = (unsigned int*)key;

	while (*p) {
		hash ^= *p;
		hash *= FNV_PRIME;
		p++;
	}
	return hash;
}
