#include <stdlib.h>
#include <stdint.h>

#include <CUnit/Basic.h>

#include "ta.h"

static void test_ta_alloc(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    {
        void *ptr = ta_alloc(tactx, sizeof(int));
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int));
        ta_free(ptr);
    }
    {
        void *ptr = ta_alloc(tactx, sizeof(NULL));
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(NULL));
        ta_free(ptr);
    }
    for (size_t i = 0; i < 10; ++i) {
        void *ptr = ta_alloc(NULL, i);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
        CU_ASSERT_EQUAL(ta_get_size(ptr), i);
        ta_free(ptr);
    }
    ta_free(tactx);
}

static void test_ta_zalloc(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    {
        void *ptr = ta_zalloc(tactx, sizeof(int));
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int));
        CU_ASSERT_EQUAL(*(int *)ptr, 0);
        ta_free(ptr);
    }
    {
        void *ptr = ta_zalloc(tactx, sizeof(NULL));
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(NULL));
        CU_ASSERT_EQUAL(*(uintptr_t *)ptr, 0);
        ta_free(ptr);
    }
    for (size_t i = 0; i < 10; ++i) {
        void *ptr = ta_alloc(NULL, i);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
        CU_ASSERT_EQUAL(ta_get_size(ptr), i);
        for (size_t j = 0; j < i; ++j)
            CU_ASSERT_EQUAL(((uint8_t *)ptr)[j], 0);
        ta_free(ptr);
    }
    ta_free(tactx);
}

static void test_ta_realloc(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));

    void *ptr = NULL, *tmp;

    ptr = ta_realloc(tactx, ptr, 10);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), 10);

    tmp = ptr = ta_realloc(tactx, ptr, 5);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), 5);

    ptr = ta_realloc(NULL, ptr, 5);
    CU_ASSERT_PTR_EQUAL(ptr, tmp);
    CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
    CU_ASSERT_EQUAL(ta_get_size(ptr), 5);

    ptr = ta_realloc(tactx, ptr, 1);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), 1);

    tmp = ptr = ta_realloc(tactx, ptr, 0);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), 0);

    ptr = ta_realloc(NULL, ptr, 0);
    CU_ASSERT_PTR_EQUAL(ptr, tmp);
    CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
    CU_ASSERT_EQUAL(ta_get_size(ptr), 0);

    ptr = ta_realloc(tactx, ptr, 25);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), 25);

    ta_free(tactx);
}

static void test_ta_alloc_array(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    {
        int *ptr = ta_alloc_array(tactx, sizeof(*ptr), 10);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 10);
        ta_free(ptr);
    }
    {
        int *ptr = ta_alloc_array(tactx, sizeof(*ptr), 5);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 5);
        ta_free(ptr);
    }
    {
        int *ptr = ta_alloc_array(NULL, sizeof(*ptr), 1);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 1);
        ta_free(ptr);
    }
    {
        int *ptr = ta_alloc_array(tactx, sizeof(*ptr), 0);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), 0);
        ta_free(ptr);
    }
    {
        int *ptr = ta_alloc_array(NULL, sizeof(*ptr), 0);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
        CU_ASSERT_EQUAL(ta_get_size(ptr), 0);
        ta_free(ptr);
    }
    {
        int *ptr = ta_alloc_array(tactx, sizeof(*ptr), 25);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 25);
        ta_free(ptr);
    }
    ta_free(tactx);
}

static void test_ta_zalloc_array(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    {
        int *ptr = ta_zalloc_array(tactx, sizeof(*ptr), 10);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 10);
        for (size_t i = 0; i < 10; ++i)
            CU_ASSERT_EQUAL(ptr[i], 0);
        ta_free(ptr);
    }
    {
        int *ptr = ta_zalloc_array(tactx, sizeof(*ptr), 5);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 5);
        for (size_t i = 0; i < 5; ++i)
            CU_ASSERT_EQUAL(ptr[i], 0);
        ta_free(ptr);
    }
    {
        int *ptr = ta_zalloc_array(NULL, sizeof(*ptr), 1);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 1);
        CU_ASSERT_EQUAL(ptr[0], 0);
        ta_free(ptr);
    }
    {
        int *ptr = ta_zalloc_array(tactx, sizeof(*ptr), 0);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), 0);
        ta_free(ptr);
    }
    {
        int *ptr = ta_zalloc_array(NULL, sizeof(*ptr), 0);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
        CU_ASSERT_EQUAL(ta_get_size(ptr), 0);
        ta_free(ptr);
    }
    {
        int *ptr = ta_zalloc_array(tactx, sizeof(*ptr), 25);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 25);
        for (size_t i = 0; i < 25; ++i)
            CU_ASSERT_EQUAL(ptr[i], 0);
        ta_free(ptr);
    }
    ta_free(tactx);
}

static void test_ta_realloc_array(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));

    int *ptr = NULL, *tmp;

    ptr = ta_realloc_array(tactx, ptr, sizeof(*ptr), 10);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 10);

    tmp = ptr = ta_realloc_array(tactx, ptr, sizeof(*ptr), 5);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 5);

    ptr = ta_realloc_array(NULL, ptr, sizeof(*ptr), 5);
    CU_ASSERT_PTR_EQUAL(ptr, tmp);
    CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
    CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 5);

    ptr = ta_realloc_array(tactx, ptr, sizeof(*ptr), 1);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 1);

    tmp = ptr = ta_realloc_array(tactx, ptr, sizeof(*ptr), 0);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), 0);

    ptr = ta_realloc_array(NULL, ptr, sizeof(*ptr), 0);
    CU_ASSERT_PTR_EQUAL(ptr, tmp);
    CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
    CU_ASSERT_EQUAL(ta_get_size(ptr), 0);

    ptr = ta_realloc_array(tactx, ptr, sizeof(*ptr), 25);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 25);

    ta_free(tactx);
}

static void test_ta_assign(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    {
        void *ptr = ta_assign(tactx, NULL, sizeof(int));
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int));
        ta_free(ptr);
    }
    for (size_t i = 0; i < 10; ++i) {
        void *ptr = ta_assign(tactx, NULL, i);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), i);
        ta_free(ptr);
    }
    {
        void *ptr = malloc(sizeof(int));
        CU_ASSERT_PTR_NOT_NULL(ptr);
        ptr = ta_assign(NULL, ptr, sizeof(int));
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int));
        ta_free(ptr);
    }
    ta_free(tactx);
}

static void test_ta_memdup(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));

    const char str[] = "hello, world";
    {
        char *ptr = ta_memdup(tactx, str, sizeof(str));
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_STRING_EQUAL(ptr, str);
        ta_free(ptr);
    }
    {
        char *ptr = ta_memdup(NULL, str, 0);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
        CU_ASSERT_EQUAL(ta_get_size(ptr), 0);
        ta_free(ptr);
    }
    ta_free(tactx);
}

static void test_ta_get_parent(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));

    for (size_t i = 0; i < 5; ++i) {
        void *ptr = ta_get_parent(ta_alloc(tactx, i));
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ptr, tactx);
    }

    ta_free(tactx);
}

int main(void)
{
    if (CU_initialize_registry() != CUE_SUCCESS)
        return (int)CU_get_error();

    CU_pSuite pSuite = CU_add_suite("ta", NULL, NULL);
    if (!pSuite)
        goto error;

    if (!CU_ADD_TEST(pSuite, test_ta_alloc) ||
        !CU_ADD_TEST(pSuite, test_ta_zalloc) ||
        !CU_ADD_TEST(pSuite, test_ta_realloc) ||
        !CU_ADD_TEST(pSuite, test_ta_alloc_array) ||
        !CU_ADD_TEST(pSuite, test_ta_zalloc_array) ||
        !CU_ADD_TEST(pSuite, test_ta_realloc_array) ||
        !CU_ADD_TEST(pSuite, test_ta_assign) ||
        !CU_ADD_TEST(pSuite, test_ta_memdup) ||
        !CU_ADD_TEST(pSuite, test_ta_get_parent)) {
        goto error;
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int num = CU_get_number_of_tests_failed();
    CU_cleanup_registry();
    if (CU_get_error() == CUE_SUCCESS) {
        return (int)num;
    } else {
        printf("CUnit Error: %s\n", CU_get_error_msg());
        return (int)CU_get_error();
    }

error:
    CU_cleanup_registry();
    return (int)CU_get_error();
}
