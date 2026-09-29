#include "hash_utils.h"

#include <string.h>
#include <assert.h>

#define FNV1A_OFFSET UINT64_C(0xcbf29ce484222325)
#define FNV1A_PRIME  UINT64_C(0x100000001b3)

int compar_int(const void *a, const void *b) {
	assert(a != NULL);
	assert(b != NULL);

	const int ai = *(const int*)a;
	const int bi = *(const int*)b;

	if (ai < bi) return -1;
	if (ai > bi) return 1;
	
	return 0;
}

int compar_str(const void *a, const void *b) {
	assert(a != NULL);
	assert(b != NULL);

	return (strcmp(a, b));
}

uint64_t hash_fnv1a_str(const void *key) {
	assert(key != NULL);

	uint64_t hash = FNV1A_OFFSET;
	const unsigned char *p = key;

	while (*p != '\0') {
		hash ^= *p++;
		hash *= FNV1A_PRIME;
	}
	return hash;
}

uint64_t hash_fnv1a_int(const void *key) {
	assert(key != NULL);

	uint64_t hash = FNV1A_OFFSET;
	uint64_t val = (uint64_t)*(const int*)key;

	for (size_t i = 0; i < 8; i++) {
		hash ^= val & UINT64_C(0xff);
		hash *= FNV1A_PRIME;
		val >>= 8;
	}
	return hash;
}
