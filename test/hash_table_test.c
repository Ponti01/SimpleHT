#include <stdint.h>
#include "unity.h"
#include "hash_table.h"
#include "hash_utils.h"

static int dummy_comp(const void *a, const void *b) {
	return *(const int*)a - *(const int*)b;
}

static uint64_t dummy_hash(const void *key) {
	return (uint64_t)(*(const int*)key);
}

static hash_table *ht_int;
static hash_table *ht_str;

void setUp(void) {
	ht_int = ht_create(compar_int, hash_fnv1a_int, 0);
	ht_str = ht_create(compar_str, hash_fnv1a_str, 0);
}

void tearDown(void) {
	ht_destroy(ht_int);
	ht_destroy(ht_str);
}

void test_ht_create_default_cap_when_zero(void) {
	hash_table *ht = ht_create(dummy_comp, dummy_hash, 0);

	TEST_ASSERT_NOT_NULL(ht);
	TEST_ASSERT_EQUAL_INT(0, ht_get_size(ht));
	TEST_ASSERT_EQUAL_INT(8, ht_get_cap(ht));

	ht_destroy(ht);
}

void test_ht_create_custom_cap_rounds_up(void) {
	hash_table *ht = ht_create(dummy_comp, dummy_hash, 10);

	TEST_ASSERT_NOT_NULL(ht);
	TEST_ASSERT_EQUAL_INT(16, ht_get_cap(ht));

	ht_destroy(ht);
}

void test_ht_create_null_functions(void) {
	TEST_ASSERT_NULL(ht_create(NULL, dummy_hash, 0));
	TEST_ASSERT_NULL(ht_create(dummy_comp, NULL, 0));
	TEST_ASSERT_NULL(ht_create(NULL, NULL, 0));
}

void test_ht_get_one_entry(void) {
	int val = 123;

	ht_insert(ht_str, "marco", &val, NULL);

	int *res = (int*)ht_get(ht_str, "marco");

	TEST_ASSERT_NOT_NULL(res);
	TEST_ASSERT_EQUAL_INT(val, *res);
}

void test_ht_get_missing_key(void) {
	int val = 1;
	ht_insert(ht_str, "marco", &val, NULL);

	TEST_ASSERT_NULL(ht_get(ht_str, "non_esiste"));
}

void test_ht_get_null_args(void) {
	int val = 1;
	ht_insert(ht_str, "marco", &val, NULL);

	TEST_ASSERT_NULL(ht_get(NULL, "marco"));
	TEST_ASSERT_NULL(ht_get(ht_str, NULL));
}

void test_ht_get_size_null_table(void) {
	TEST_ASSERT_EQUAL_INT(0, ht_get_size(NULL));
}

void test_ht_insert_one_entry(void) {
	int val = 123;

	int ret = ht_insert(ht_str, "marco", &val, NULL);

	TEST_ASSERT_EQUAL_INT(ret, 0);
	TEST_ASSERT_EQUAL_INT(ht_get_size(ht_str), 1);
}

void test_ht_insert_overwrite_return(void) {
	int key = 1, v1 = 2, v2 = 3;

	int res1 = ht_insert(ht_int, &key, &v1, NULL);
	int res2 = ht_insert(ht_int, &key, &v2, NULL);

	TEST_ASSERT_EQUAL_INT(0, res1);
	TEST_ASSERT_EQUAL_INT(1, res2);
}

void test_ht_insert_null_entry(void) {
	int key = 1, val = 2;
	int res = ht_insert(NULL, &key, &val, NULL);

	TEST_ASSERT_EQUAL_INT(-1, res);
}

void test_ht_insert_null_key(void) {
	int val = 1;
	int res = ht_insert(ht_int, NULL, &val, NULL);

	TEST_ASSERT_EQUAL_INT(-1, res);
}

void test_ht_insert_null_value(void) {
	int key = 1;
	int res = ht_insert(ht_int, &key, NULL, NULL);

	TEST_ASSERT_EQUAL_INT(-1, res);
}

void test_insert_entries(void) {
	int a = 1, b = 2, c = 3;

	ht_insert(ht_str, "marco", &a, NULL);
	ht_insert(ht_str, "paolo", &b, NULL);
	ht_insert(ht_str, "mario", &c, NULL);

	TEST_ASSERT_EQUAL_INT(3, ht_get_size(ht_str));
	TEST_ASSERT_EQUAL_INT(1, *(int*)ht_get(ht_str, "marco"));
	TEST_ASSERT_EQUAL_INT(2, *(int*)ht_get(ht_str, "paolo"));
	TEST_ASSERT_EQUAL_INT(3, *(int*)ht_get(ht_str, "mario"));
}

void test_ht_insert_overwrite_value(void) {
	int a = 1, b = 2;

	ht_insert(ht_str, "mario", &a, NULL);
	ht_insert(ht_str, "mario", &b, NULL);

	TEST_ASSERT_EQUAL_INT(1, ht_get_size(ht_str));
	TEST_ASSERT_EQUAL_INT(2, *(int*)ht_get(ht_str, "mario"));
}

void test_ht_insert_overwrite_old_value(void) {
	int k = 1, v1 = 2, v2 = 3;
	const void *old = NULL;

	ht_insert(ht_int, &k, &v1, &old);
	ht_insert(ht_int, &k, &v2, &old);

	TEST_ASSERT_EQUAL_INT(2, *(int*)old);

	int v3 = 4;
	int res = ht_insert(ht_int, &k, &v3, NULL);

	TEST_ASSERT_EQUAL_INT(1, res);
	TEST_ASSERT_EQUAL_INT(4, *(int*)ht_get(ht_int, &k));
}

void test_table_resize(void) {
	int keys[7];
	int vals[7];

	for (int i = 0; i < 7; i++) {
		keys[i] = i;
		vals[i] = i * 10;
		ht_insert(ht_int, &keys[i], &vals[i], NULL);
	}

	TEST_ASSERT_EQUAL_INT(16, ht_get_cap(ht_int));
	TEST_ASSERT_EQUAL_INT(7, ht_get_size(ht_int));

	for (int i = 0; i < 7; i++) {
		int *res = (int*)ht_get(ht_int, &keys[i]);
		TEST_ASSERT_NOT_NULL(res);
		TEST_ASSERT_EQUAL_INT(i * 10, *res);
	}
}

void test_table_resize_exact_threshold(void) {
	int keys[6];
	int vals[6];

	for (int i = 0; i < 5; i++) {
		keys[i] = i;
		vals[i] = i;
		ht_insert(ht_int, &keys[i], &vals[i], NULL);
	}
	TEST_ASSERT_EQUAL_INT(8, ht_get_cap(ht_int));
	keys[5] = 5; vals[5] = 5;
	ht_insert(ht_int, &keys[5], &vals[5], NULL);

	TEST_ASSERT_EQUAL_INT(16, ht_get_cap(ht_int));
	TEST_ASSERT_EQUAL_INT(6, ht_get_size(ht_int));
}

void test_ht_remove_one_el(void) {
	int a = 1, b = 2;
	const void *removed;
	ht_insert(ht_str, "mario", &a, NULL);
	ht_insert(ht_str, "paolo", &b, NULL);

	int *res = (int*)ht_remove(ht_str, "mario", &removed);

	TEST_ASSERT_EQUAL_INT(1, ht_get_size(ht_str));
	TEST_ASSERT_EQUAL_INT(1, *res);
	TEST_ASSERT_EQUAL_STRING("mario", (const char*)removed);
}

void test_ht_remove_ret_null(void) {
	int key = 1, a = 2;
	const void *removed;

	ht_insert(ht_int, &key, &a, NULL);

	int k = 5;
	int *res = (int*)ht_remove(ht_int, &k, &removed);

	TEST_ASSERT_NULL(res);
	TEST_ASSERT_EQUAL_INT(1, ht_get_size(ht_int));
	TEST_ASSERT_NULL(removed);
}

void test_ht_remove_null_removed_key(void) {
	int key = 1, a = 2;

	ht_insert(ht_int, &key, &a, NULL);

	int *res = (int*)ht_remove(ht_int, &key, NULL);

	TEST_ASSERT_EQUAL_INT(0, ht_get_size(ht_int));
	TEST_ASSERT_EQUAL_INT(2, *res);
}

void test_ht_remove_null_args(void) {
	int key = 1, a = 2;
	ht_insert(ht_int, &key, &a, NULL);

	TEST_ASSERT_NULL(ht_remove(NULL, &key, NULL));
	TEST_ASSERT_NULL(ht_remove(ht_int, NULL, NULL));
	TEST_ASSERT_EQUAL_INT(1, ht_get_size(ht_int));
}

void test_ht_remove_then_reinsert(void) {
	int key = 1, a = 2, b = 3;

	ht_insert(ht_int, &key, &a, NULL);
	ht_remove(ht_int, &key, NULL);

	TEST_ASSERT_EQUAL_INT(0, ht_get_size(ht_int));
	TEST_ASSERT_NULL(ht_get(ht_int, &key));

	int res = ht_insert(ht_int, &key, &b, NULL);

	TEST_ASSERT_EQUAL_INT(0, res);
	TEST_ASSERT_EQUAL_INT(1, ht_get_size(ht_int));
	TEST_ASSERT_EQUAL_INT(3, *(int*)ht_get(ht_int, &key));
}

void test_ht_destroy_null_is_noop(void) {
	ht_destroy(NULL);
	TEST_PASS();
}

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_ht_create_default_cap_when_zero);
	RUN_TEST(test_ht_create_custom_cap_rounds_up);
	RUN_TEST(test_ht_create_null_functions);

	RUN_TEST(test_ht_get_one_entry);
	RUN_TEST(test_ht_get_missing_key);
	RUN_TEST(test_ht_get_null_args);
	RUN_TEST(test_ht_get_size_null_table);

	RUN_TEST(test_ht_insert_one_entry);
	RUN_TEST(test_ht_insert_overwrite_return);
	RUN_TEST(test_ht_insert_null_entry);
	RUN_TEST(test_ht_insert_null_key);
	RUN_TEST(test_ht_insert_null_value);
	RUN_TEST(test_insert_entries);
	RUN_TEST(test_ht_insert_overwrite_value);
	RUN_TEST(test_ht_insert_overwrite_old_value);

	RUN_TEST(test_table_resize);
	RUN_TEST(test_table_resize_exact_threshold);

	RUN_TEST(test_ht_remove_one_el);
	RUN_TEST(test_ht_remove_ret_null);
	RUN_TEST(test_ht_remove_null_removed_key);
	RUN_TEST(test_ht_remove_null_args);
	RUN_TEST(test_ht_remove_then_reinsert);

	RUN_TEST(test_ht_destroy_null_is_noop);

	return UNITY_END();
}
