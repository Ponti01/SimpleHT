#include "hash_table.h"

#include <stdlib.h>
#include <assert.h>

#define DEFAULT_CAP ((size_t)8)

#define MAX_CAP (SIZE_MAX / sizeof(ht_entry*))

typedef struct entry {
	const void *key;
	const void *value;
	uint64_t hash;
	struct entry *next;
} ht_entry;

struct hash_table {
	ht_entry **buckets;
	size_t size;
	size_t cap;
	int (*compar)(const void*, const void*);
	uint64_t (*hash)(const void*);
};

static size_t next_power_of_two(size_t n) {
	if (n > MAX_CAP)
		return 0;

	size_t new_cap = 1;
	while (new_cap < n) {
		if (new_cap > MAX_CAP / 2)
			return 0;

		new_cap *= 2;
	}
	return new_cap;	
}

static size_t get_bucket_idx(uint64_t h, size_t cap) {
	h ^= h >> 33;
	h *= UINT64_C(0xff51afd7ed558ccd);
	h ^= h >> 33;
	h *= UINT64_C(0xc4ceb9fe1a85ec53);
	h ^= h >> 33;
	return (size_t)(h & (cap - 1));
}

static void try_grow(hash_table *ht) {
	if (ht->cap > MAX_CAP / 2)
		return;

	const size_t new_cap = ht->cap * 2;
	ht_entry **new_buckets = calloc(new_cap, sizeof(ht_entry*));
	if (new_buckets == NULL)
		return;

	for (size_t i = 0; i < ht->cap; i++) {
		ht_entry *e = ht->buckets[i];
		while (e != NULL) {
			ht_entry *next = e->next;
			const size_t idx = get_bucket_idx(e->hash, new_cap);
			e->next = new_buckets[idx];
			new_buckets[idx] = e;
			e = next;
		}
	}

	free(ht->buckets);
	ht->buckets = new_buckets;
	ht->cap = new_cap;
}

hash_table *ht_create(int (*compar)(const void*, const void*), uint64_t (*hash)(const void*), size_t init_cap) {
	if (compar == NULL || hash == NULL)
		return NULL;

	size_t new_cap = (init_cap == 0) ? DEFAULT_CAP : next_power_of_two(init_cap);
	if (new_cap == 0)
		return NULL;

	hash_table *ht = malloc(sizeof(*ht));
	if (ht == NULL)
		return NULL;

	ht->buckets = calloc(new_cap, sizeof(*ht->buckets));
	if (ht->buckets == NULL) {
		free(ht);
		return NULL;
	}
	ht->size = 0;
	ht->cap = new_cap;
	ht->compar = compar;
	ht->hash = hash;

	return ht;

}


int ht_insert(hash_table *ht, const void *key, const void *value, const void **old_value) {
	if (ht == NULL || key == NULL || value == NULL)
		return -1;

	const uint64_t hash = ht->hash(key);
	const size_t idx = get_bucket_idx(hash, ht->cap);

	ht_entry *e = ht->buckets[idx];
	while (e != NULL) {
		if (e->hash == hash && ht->compar(e->key, key) == 0) {
			if (old_value != NULL)
				*old_value = e->value;
			e->value = value;
			return 1;
		}
		e = e->next;
	}

	if (ht->size == SIZE_MAX)
		return -1;

	ht_entry *new_entry = malloc(sizeof(*new_entry));
	if (new_entry == NULL)
		return -1;
	
	new_entry->key = key;
	new_entry->value = value;
	new_entry->hash = hash;
	new_entry->next = ht->buckets[idx];

	ht->buckets[idx] = new_entry;
	ht->size++;

	if (ht->size >= ht->cap - (ht->cap / 4))
		try_grow(ht);

	return 0;
}

void *ht_get(hash_table *ht, const void *key) {
	if (ht == NULL || key == NULL)
		return NULL;

	const uint64_t hash = ht->hash(key);
	const size_t idx = get_bucket_idx(hash, ht->cap);

	ht_entry *e = ht->buckets[idx];
	while (e != NULL) {
		if (e->hash == hash && ht->compar(e->key, key) == 0) {
			return (void*)e->value;
		}
		e = e->next;
	}

	return NULL;
}

size_t ht_get_size(hash_table *ht) {
	return (ht == NULL) ? 0 : ht->size;
}

size_t ht_get_cap(hash_table *ht) {
	return (ht == NULL) ? 0 : ht->cap;
}

void ht_foreach(hash_table *ht, ht_visit_fn visit, void *p) {
	if (ht == NULL || visit == NULL)
		return;

	for (size_t i = 0; i < ht->cap; i++) {
		ht_entry *e = ht->buckets[i];
		while (e != NULL) {
			ht_entry *next = e->next;
			visit(e->key, e->value, p);
			e = next;
		}
	}
}

void *ht_remove(hash_table *ht, const void *key, const void **removed_key) {
	if (ht == NULL || key == NULL)
		return NULL;

	if (removed_key != NULL)
			*removed_key = NULL;

	const uint64_t hash = ht->hash(key);
	const size_t idx = get_bucket_idx(hash, ht->cap);

	ht_entry *curr = ht->buckets[idx];
	ht_entry *prev = NULL;
	while (curr != NULL) {
		if (ht->compar(curr->key, key) == 0) {
			if (prev == NULL) 
				ht->buckets[idx] = curr->next;
			else
				prev->next = curr->next;
			
			if (removed_key != NULL)
				*removed_key = curr->key;

			void *val = (void*)curr->value;
			free(curr);
			ht->size--;

			return val;
		}
		prev = curr;
		curr = curr->next;
	}
	return NULL;
}

void ht_destroy(hash_table *ht) {
	if (ht == NULL)
		return;

	for (size_t i = 0; i < ht->cap; i++) {
		ht_entry *e = ht->buckets[i];
		while (e != NULL) {
			ht_entry *next = e->next;
			free(e);
			e = next;
		}
	}
	
	free(ht->buckets);
	free(ht);
}

