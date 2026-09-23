#ifndef HASH_TABLE_H
#define HASH_TABLE_H

typedef struct _hash_table {
	void **buckets;
	size_t size;
	size_t cap;
	int (*compar)(const void*, const void*);
	unsigned long (*hash)(const void*);
} hash_table;

typedef struct _entry {
	const void *key;
	const void *value;
	unsigned long hash;
	struct _entry *next;
} ht_entry;

hash_table *ht_create(int (*compar)(const void*, const void*), unsigned long (*hash)(const void*));
int ht_insert(hash_table *ht, const void *key, const void *value);
void *ht_get(hash_table *ht, const void *key);

void ht_destroy(hash_table *ht);

#endif
