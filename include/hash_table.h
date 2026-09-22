#ifndef HASH_TABLE_H
#define HASH_TABLE_H

hash_table *ht_create(int (*compar)(const void*, const void*), unsigned long (*hash)(const void*));


#endif
