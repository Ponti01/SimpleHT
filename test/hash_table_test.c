#include "unity.h"
#include "hash_table.h"
#include "hash_utils.h"

static int dummy_comp(const void *a, const void *b) {
	return *(const int*)a - *(const int*)b;
}

static unsigned long dummy_hash(const void *key) {
	return (unsigned long)(*(const int*)key);
}

static hash_table *ht_int;
static hash_table *ht_str;

void setUp(void) {
	ht_int = ht_create(compar_int, hash_fnv1a_int);
	ht_str = ht_create(compar_str, hash_fnv1a_str);
}

void tearDown(void) {
	ht_destroy(ht_int);
	ht_destroy(ht_str);
}

void test_ht_create_valid_table(void) {
	hash_table *ht = ht_create(dummy_comp, dummy_hash);

	TEST_ASSERT_NOT_NULL(ht);
	TEST_ASSERT_NOT_NULL(ht->buckets);
	TEST_ASSERT_EQUAL_INT(0, ht->size);
	TEST_ASSERT_EQUAL_INT(8, ht->cap);
	TEST_ASSERT_EQUAL_PTR(dummy_comp, ht->compar);
	TEST_ASSERT_EQUAL_PTR(dummy_hash, ht->hash);
}

void test_ht_create_buckets_are_zeroed(void) {
	hash_table *ht = ht_create(dummy_comp, dummy_hash);

	for (int i = 0; i < 8; i++) {
		TEST_ASSERT_NULL(ht->buckets[i]);
	}
}

void test_ht_create_null_functions(void) {

	TEST_ASSERT_NULL(ht_create(NULL, dummy_hash));
	TEST_ASSERT_NULL(ht_create(dummy_comp, NULL));
}

void test_ht_insert_one_entry(void) {
	int val = 123;

	int ret = ht_insert(ht_str, "marco", &val);

	TEST_ASSERT_EQUAL_INT(ret, 0);
	TEST_ASSERT_EQUAL_INT(ht_str->size, 1);
}

void test_ht_get_one_entry(void) {
	int val = 123;

	ht_insert(ht_str, "marco", &val);

	int *res = (int*)ht_get(ht_str, "marco");

	TEST_ASSERT_EQUAL_INT(val, *res);
}

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_ht_create_valid_table);
	RUN_TEST(test_ht_create_buckets_are_zeroed);
	RUN_TEST(test_ht_create_null_functions);
	RUN_TEST(test_ht_insert_one_entry);
	RUN_TEST(test_ht_get_one_entry);

	return UNITY_END();
}
