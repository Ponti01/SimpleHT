#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>
#include <stdint.h>

/* Opaque hash table type, keys and values are owned by the caller. */
typedef struct hash_table hash_table;

/* Callback function invoked for each stored key/value pair by ht_foreach(). */
typedef void (*ht_visit_fn)(const void *key, const void *val, void *p);

/**
 * Create an empty hash table using the provided comparison and hash functions.
 * Return NULL if those functions are invalid and on allocation failure.
 * Use init_cap to specify the initial cap which will be rounded
 * to the next power of two, if called with init_cap == 0 it will be set at 8.
 * The table stores pointers without freeing users data.
 */
hash_table *ht_create(int (*compar)(const void*, const void*), uint64_t (*hash)(const void*), size_t init_cap);

/**
 * Insert a key/value pair, or replace the value of an already stored key.
 * Return NULL if ht, key or value are NULL, also their pointed-to data is not copied.
 * If old_value is non-NULL, the previous value pointer is written to *old_value.
 * Return 0 for a new entry, 1 for replacement, or -1 for invalid arguments or entry allocation failure.
 * No key or value is freed.
 * New entries may trigger automatic growth.
 * Failure to grow does not undo
 * the insertion and is not reported as an insertion error.
 */
int ht_insert(hash_table *ht, const void *key, const void *value, const void **old_value);

/**
 * Return the stored value for the specified key, or NULL if no entry matches or ht/key is NULL.
 * The returned pointer is borrowed, not a copy.
 * Although the return type is void *, data originally supplied as const
 * must not be modified through this pointer.
 */
void *ht_get(hash_table *ht, const void *key);

/* Return the number of entries or 0 if ht is NULL. */
size_t ht_get_size(hash_table *ht);

/* Return the current cap (number of buckets) or 0 is ht is NULL. */
size_t ht_get_cap(hash_table *ht);

/** 
 * Visit every entry once in an unspecified order, passing p to visit.
 * Do nothing if ht or visit is NULL, p itself may be NULL.
 * This function neither copies nor frees the visited keys and values
 */
void ht_foreach(hash_table *ht, ht_visit_fn visit, void *p);

/**
 * Remove an equivalent key and return its value pointer, or return NULL if
 * no entry matches or ht/key is NULL.
 * Only the internal entry is freed.
 * If removed_key is non-NULL, store the original key pointer on success,
 * or NULL when valid ht/key arguments are supplied but no entry matches.
 * The caller remains responsible for the removed key and value.
 * Although the return type is void *, originally const data must not be modified.
 */
void *ht_remove(hash_table *ht, const void *key, const void **removed_key);

/**
 * Free all internal entries, the bucket array, and the table itself.
 * Keys and values are not freed, the caller must manage their lifetime.
 * Do nothing if ht is NULL.
 */
void ht_destroy(hash_table *ht);

#endif
