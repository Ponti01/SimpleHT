#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

#define INIT_CAP 8


hash_table *ht_create(int (*compar)(const void*, const void*), unsigned long (*hash)(const void*)) {
	if (compar == NULL || hash == NULL)
		return NULL;

	hash_table *ht = malloc(sizeof(hash_table));
	if (ht == NULL)
		return NULL;

	ht->buckets = calloc(INIT_CAP, sizeof(*ht->buckets));
	if (ht->buckets == NULL) {
		free(ht);
		return NULL;
	}
	ht->size = 0;
	ht->cap = INIT_CAP;
	ht->compar = compar;
	ht->hash = hash;

	return ht;
}

int ht_insert(hash_table *ht, const void *key, const void *value) {
	if (ht == NULL || key == NULL || value == NULL)
		return -1;

	unsigned long hash = ht->hash(key);
	size_t idx = (hash ^ (hash >> 32)) % ht->cap;

	ht_entry *curr = ht->buckets[idx];
	while (curr != NULL) {
		if (ht->compar(curr->key, key) == 0) {
			curr->value = value;
			return 1;
		}
		curr = curr->next;
	}

	ht_entry *new_entry = malloc(sizeof(ht_entry));
	if (new_entry == NULL) return -1;
	
	new_entry->key = (void*)key;
	new_entry->value = value;
	new_entry->hash = hash;
	new_entry->next = ht->buckets[idx];

	ht->buckets[idx] = new_entry;
	ht->size++;

	return 0;
}

void *ht_get(hash_table *ht, const void *key) {
	if (ht == NULL || key == NULL)
		return NULL;

	unsigned long hash = ht->hash(key);
	size_t idx = (hash ^ (hash >> 32)) % ht->cap;

	ht_entry *curr = ht->buckets[idx];
	while (curr != NULL) {
		if (ht->compar(curr->key, key) == 0) {
			return (void*)curr->value;
		}
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
	free(ht);;
}

