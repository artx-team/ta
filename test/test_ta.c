#include <stdlib.h>
#include <stdint.h>

#include <CUnit/Basic.h>

#include "ta.h"

static void test_ta_xmalloc(void)
{
    for (size_t i = 0; i < 100; ++i) {
        void *ptr = ta_xmalloc(i);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        free(ptr);
    }
}

static void test_ta_xcalloc(void)
{
    for (size_t i = 0; i < 10; ++i) {
        for (size_t j = 0; j < 10; ++j) {
            void *ptr = ta_xcalloc(i, j);
            CU_ASSERT_PTR_NOT_NULL(ptr);
            free(ptr);
        }
    }
}

static void test_ta_xrealloc(void)
{
    void *ptr = ta_xrealloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    ptr = ta_xrealloc(ptr, 100);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    ptr = ta_xrealloc(ptr, 0);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    free(ptr);
}

static void test_ta_xzalloc(void)
{
    for (size_t i = 0; i < 100; ++i) {
        void *ptr = ta_xzalloc(i);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        free(ptr);
    }
}

static void test_ta_xstrdup(void)
{
    {
        char *str = ta_xstrdup("");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_STRING_EQUAL(str, "");
        free(str);
    }
    {
        char *str = ta_xstrdup("test");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_STRING_EQUAL(str, "test");
        free(str);
    }
}

static void test_ta_xstrndup(void)
{
    {
        char *str = ta_xstrndup("", 0);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_NSTRING_EQUAL(str, "", 0);
        free(str);
    }
    {
        char *str = ta_xstrndup("test", 0);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_NSTRING_EQUAL(str, "test", 0);
        free(str);
    }
    {
        char *str = ta_xstrndup("test", 2);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_NSTRING_EQUAL(str, "test", 2);
        free(str);
    }
    {
        char *str = ta_xstrndup("test", 5);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_NSTRING_EQUAL(str, "test", 5);
        free(str);
    }
}

static void test_ta_xmemdup(void)
{
    {
        char *str = ta_xmemdup("", 0);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_NSTRING_EQUAL(str, "", 0);
        free(str);
    }
    {
        char *str = ta_xmemdup("test", 0);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_NSTRING_EQUAL(str, "test", 0);
        free(str);
    }
    {
        char *str = ta_xmemdup("test", 2);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_NSTRING_EQUAL(str, "test", 2);
        free(str);
    }
    {
        char *str = ta_xmemdup("test", 5);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_STRING_EQUAL(str, "test");
        free(str);
    }
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

    tactx = ta_realloc(NULL, tactx, 100);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NOT_NULL(ta_get_child(tactx));

    ta_free_children(tactx);
    ta_free(tactx);
}

static void test_ta_get_size(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    CU_ASSERT_EQUAL(ta_get_size(tactx), 0);

    for (size_t i = 0; i < 5; ++i) {
        void *ptr = ta_alloc(tactx, i);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), i);
    }

    tactx = ta_realloc(NULL, tactx, 100);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NOT_NULL(ta_get_child(tactx));

    ta_free_children(tactx);
    ta_free(tactx);
}

static void test_ta_get_child(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    CU_ASSERT_PTR_NULL(ta_get_child(tactx));

    for (size_t i = 0; i < 5; ++i) {
        void *ptr = ta_alloc(tactx, i);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_PTR_EQUAL(ta_get_child(tactx), ptr);
    }

    for (size_t i = 0; i < 3; ++i) {
        void *ptr = ta_get_child(tactx);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        ta_free(ptr);
        CU_ASSERT_PTR_NOT_EQUAL(ta_get_child(tactx), ptr);
    }

    {
        void *ptr = ta_get_child(tactx);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), 1);
        ta_set_parent(ptr, NULL);
        ta_free(ptr);
    }

    {
        void *ptr = ta_get_child(tactx);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), 0);
        ta_set_parent(ptr, NULL);
        ta_free(ptr);
    }

    {
        void *ptr = ta_get_child(tactx);
        CU_ASSERT_PTR_NULL(ptr);
    }

    ta_free(tactx);
}

static void test_ta_get_next(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    CU_ASSERT_PTR_NULL(ta_get_next(tactx));

    for (size_t i = 0; i < 5; ++i) {
        void *tmp = ta_get_child(tactx);
        void *ptr = ta_alloc(tactx, i);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_PTR_EQUAL(ta_get_next(ptr), tmp);
    }

    for (size_t i = 0; i < 3; ++i) {
        void *tmp = ta_get_child(tactx);
        CU_ASSERT_PTR_NOT_NULL(tmp);
        void *ptr = ta_get_next(tmp);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        ta_free(tmp);
        CU_ASSERT_PTR_EQUAL(ta_get_child(tactx), ptr);
    }

    {
        void *tmp = ta_get_child(tactx);
        CU_ASSERT_PTR_NOT_NULL(tmp);
        void *ptr = ta_get_next(tmp);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(tmp), 1);
        CU_ASSERT_EQUAL(ta_get_size(ptr), 0);
        ta_free(tmp);
    }

    {
        void *tmp = ta_get_child(tactx);
        CU_ASSERT_PTR_NOT_NULL(tmp);
        void *ptr = ta_get_next(tmp);
        CU_ASSERT_PTR_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(tmp), tactx);
        CU_ASSERT_EQUAL(ta_get_size(tmp), 0);
        ta_free(tmp);
    }

    ta_free(tactx);
}

static void test_ta_get_prev(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    CU_ASSERT_PTR_NULL(ta_get_prev(tactx));

    for (size_t i = 0; i < 5; ++i) {
        void *tmp = ta_get_child(tactx);
        void *ptr = ta_alloc(tactx, i);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_PTR_EQUAL(ta_get_prev(ptr), NULL);
        if (tmp)
            CU_ASSERT_PTR_EQUAL(ta_get_prev(tmp), ptr);
    }

    for (size_t i = 0; i < 3; ++i) {
        void *ptr = ta_get_child(tactx);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_NULL(ta_get_prev(ptr));
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_PTR_EQUAL(ta_get_prev(ta_get_next(ptr)), ptr);
        ta_free(ptr);
    }

    ta_free(tactx);
}

static void test_ta_has_parent(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    CU_ASSERT_FALSE(ta_has_parent(tactx, NULL));
    CU_ASSERT_FALSE(ta_has_parent(tactx, tactx));

    void *ptr1 = ta_alloc(tactx, 0);
    CU_ASSERT_PTR_NOT_NULL(ptr1);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr1), tactx);
    CU_ASSERT_FALSE(ta_has_parent(ptr1, NULL));
    CU_ASSERT_FALSE(ta_has_parent(ptr1, ptr1));
    CU_ASSERT_TRUE(ta_has_parent(ptr1, tactx));
    CU_ASSERT_FALSE(ta_has_parent(tactx, ptr1));

    void *ptr2 = ta_alloc(tactx, 0);
    CU_ASSERT_PTR_NOT_NULL(ptr2);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr2), tactx);
    CU_ASSERT_FALSE(ta_has_parent(ptr2, NULL));
    CU_ASSERT_FALSE(ta_has_parent(ptr2, ptr2));
    CU_ASSERT_TRUE(ta_has_parent(ptr2, tactx));
    CU_ASSERT_FALSE(ta_has_parent(tactx, ptr2));
    CU_ASSERT_FALSE(ta_has_parent(ptr2, ptr1));
    CU_ASSERT_FALSE(ta_has_parent(ptr1, ptr2));

    void *ptr3 = ta_alloc(ptr1, 0);
    CU_ASSERT_PTR_NOT_NULL(ptr3);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr3), ptr1);
    CU_ASSERT_FALSE(ta_has_parent(ptr3, NULL));
    CU_ASSERT_FALSE(ta_has_parent(ptr3, ptr3));
    CU_ASSERT_TRUE(ta_has_parent(ptr3, tactx));
    CU_ASSERT_FALSE(ta_has_parent(tactx, ptr3));
    CU_ASSERT_TRUE(ta_has_parent(ptr3, ptr1));
    CU_ASSERT_FALSE(ta_has_parent(ptr1, ptr3));
    CU_ASSERT_FALSE(ta_has_parent(ptr3, ptr2));
    CU_ASSERT_FALSE(ta_has_parent(ptr2, ptr3));

    ta_free(ptr2);
    ta_free(tactx);
}

static void test_ta_has_child(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    CU_ASSERT_FALSE(ta_has_child(tactx, NULL));
    CU_ASSERT_FALSE(ta_has_child(tactx, tactx));

    void *ptr1 = ta_alloc(tactx, 0);
    CU_ASSERT_PTR_NOT_NULL(ptr1);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr1), tactx);
    CU_ASSERT_FALSE(ta_has_child(ptr1, NULL));
    CU_ASSERT_FALSE(ta_has_child(ptr1, ptr1));
    CU_ASSERT_FALSE(ta_has_child(ptr1, tactx));
    CU_ASSERT_TRUE(ta_has_child(tactx, ptr1));

    void *ptr2 = ta_alloc(tactx, 0);
    CU_ASSERT_PTR_NOT_NULL(ptr2);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr2), tactx);
    CU_ASSERT_FALSE(ta_has_child(ptr2, NULL));
    CU_ASSERT_FALSE(ta_has_child(ptr2, ptr2));
    CU_ASSERT_FALSE(ta_has_child(ptr2, tactx));
    CU_ASSERT_TRUE(ta_has_child(tactx, ptr2));
    CU_ASSERT_FALSE(ta_has_child(ptr2, ptr1));
    CU_ASSERT_FALSE(ta_has_child(ptr1, ptr2));

    void *ptr3 = ta_alloc(ptr1, 0);
    CU_ASSERT_PTR_NOT_NULL(ptr3);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr3), ptr1);
    CU_ASSERT_FALSE(ta_has_child(ptr3, NULL));
    CU_ASSERT_FALSE(ta_has_child(ptr3, ptr3));
    CU_ASSERT_FALSE(ta_has_child(ptr3, tactx));
    CU_ASSERT_TRUE(ta_has_child(tactx, ptr3));
    CU_ASSERT_FALSE(ta_has_child(ptr3, ptr1));
    CU_ASSERT_TRUE(ta_has_child(ptr1, ptr3));
    CU_ASSERT_FALSE(ta_has_child(ptr3, ptr2));
    CU_ASSERT_FALSE(ta_has_child(ptr2, ptr3));

    ta_free(ptr2);
    ta_free(tactx);
}

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
        void *ptr = ta_zalloc(NULL, i);
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

    void *ptr = NULL;

    ptr = ta_realloc(tactx, ptr, 10);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), 10);

    ptr = ta_realloc(tactx, ptr, 5);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), 5);

    ptr = ta_realloc(NULL, ptr, 5);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
    CU_ASSERT_EQUAL(ta_get_size(ptr), 5);

    ptr = ta_realloc(tactx, ptr, 1);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), 1);

    ptr = ta_realloc(tactx, ptr, 0);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), 0);

    ptr = ta_realloc(NULL, ptr, 0);
    CU_ASSERT_PTR_NOT_NULL(ptr);
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

    int *ptr = NULL;

    ptr = ta_realloc_array(tactx, ptr, sizeof(*ptr), 10);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 10);

    ptr = ta_realloc_array(tactx, ptr, sizeof(*ptr), 5);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 5);

    ptr = ta_realloc_array(NULL, ptr, sizeof(*ptr), 5);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
    CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 5);

    ptr = ta_realloc_array(tactx, ptr, sizeof(*ptr), 1);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(int) * 1);

    ptr = ta_realloc_array(tactx, ptr, sizeof(*ptr), 0);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    CU_ASSERT_EQUAL(ta_get_size(ptr), 0);

    ptr = ta_realloc_array(NULL, ptr, sizeof(*ptr), 0);
    CU_ASSERT_PTR_NOT_NULL(ptr);
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
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(str));
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

static void test_ta_strdup(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));

    const char str[] = "hello, world";
    {
        char *ptr = ta_strdup(tactx, str);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(str));
        CU_ASSERT_STRING_EQUAL(ptr, str);
        ta_free(ptr);
    }
    {
        char *ptr = ta_strdup(NULL, "");
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
        CU_ASSERT_EQUAL(ta_get_size(ptr), 1);
        ta_free(ptr);
    }
    ta_free(tactx);
}

static void test_ta_strdup_append(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    {
        char *str = ta_strdup(tactx, "hello");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello"));
        CU_ASSERT_STRING_EQUAL(str, "hello");

        str = ta_strdup_append(str, ", ");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, "));
        CU_ASSERT_STRING_EQUAL(str, "hello, ");

        str = ta_strdup_append(str, "world");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, world"));
        CU_ASSERT_STRING_EQUAL(str, "hello, world");
        ta_free(str);
    }
    {
        char *str = ta_strdup(tactx, "hello\0world");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello"));
        CU_ASSERT_STRING_EQUAL(str, "hello");

        str = ta_strdup_append(str, ", ");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, "));
        CU_ASSERT_STRING_EQUAL(str, "hello, ");

        str = ta_strdup_append(str, "world\0hello");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, world"));
        CU_ASSERT_STRING_EQUAL(str, "hello, world");
        ta_free(str);
    }
    ta_free(tactx);
}

static void test_ta_strdup_append_buffer(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    {
        char *str = ta_strdup(tactx, "hello");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello"));
        CU_ASSERT_STRING_EQUAL(str, "hello");

        str = ta_strdup_append_buffer(str, ", ");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, "));
        CU_ASSERT_STRING_EQUAL(str, "hello, ");

        str = ta_strdup_append_buffer(str, "world");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, world"));
        CU_ASSERT_STRING_EQUAL(str, "hello, world");
        ta_free(str);
    }
    {
        char *str = ta_memdup(tactx, "hello\0world", sizeof("hello\0world"));
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello\0world"));
        CU_ASSERT_TRUE(!memcmp(str, "hello\0world", ta_get_size(str)));

        str = ta_strdup_append_buffer(str, ", ");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello\0world, "));
        CU_ASSERT_TRUE(!memcmp(str, "hello\0world, ", ta_get_size(str)));

        str = ta_strdup_append_buffer(str, "world\0hello");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello\0world, world"));
        CU_ASSERT_TRUE(!memcmp(str, "hello\0world, world", ta_get_size(str)));
        ta_free(str);
    }
    ta_free(tactx);
}

static void test_ta_strndup(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));

    const char str[] = "hello, world";
    {
        char *ptr = ta_strndup(tactx, str, 5);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), 6);
        CU_ASSERT_NSTRING_EQUAL(ptr, str, 5);
        ta_free(ptr);
    }
    {
        char *ptr = ta_strndup(tactx, str, 1000);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(str));
        CU_ASSERT_NSTRING_EQUAL(ptr, str, sizeof(str));
        ta_free(ptr);
    }
    {
        char *ptr = ta_strndup(NULL, str, 0);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
        CU_ASSERT_EQUAL(ta_get_size(ptr), 1);
        ta_free(ptr);
    }
    ta_free(tactx);
}

static void test_ta_strndup_append(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    {
        char *str = ta_strndup(tactx, "hello, world", 5);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello"));
        CU_ASSERT_STRING_EQUAL(str, "hello");

        str = ta_strndup_append(str, ", world", 2);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, "));
        CU_ASSERT_STRING_EQUAL(str, "hello, ");

        str = ta_strndup_append(str, "world hello", 5);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, world"));
        CU_ASSERT_STRING_EQUAL(str, "hello, world");
        ta_free(str);
    }
    {
        char *str = ta_strndup(tactx, "hello\0world", 1000);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello"));
        CU_ASSERT_STRING_EQUAL(str, "hello");

        str = ta_strndup_append(str, ", world", 2);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, "));
        CU_ASSERT_STRING_EQUAL(str, "hello, ");

        str = ta_strndup_append(str, "world\0hello", 1000);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, world"));
        CU_ASSERT_STRING_EQUAL(str, "hello, world");
        ta_free(str);
    }
    ta_free(tactx);
}

static void test_ta_strndup_append_buffer(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    {
        char *str = ta_strndup(tactx, "hello, world", 5);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello"));
        CU_ASSERT_STRING_EQUAL(str, "hello");

        str = ta_strndup_append_buffer(str, ", world", 2);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, "));
        CU_ASSERT_STRING_EQUAL(str, "hello, ");

        str = ta_strndup_append_buffer(str, "world\0hello", 1000);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, world"));
        CU_ASSERT_STRING_EQUAL(str, "hello, world");
        ta_free(str);
    }
    {
        char *str = ta_memdup(tactx, "hello\0world", sizeof("hello\0world"));
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello\0world"));
        CU_ASSERT_TRUE(!memcmp(str, "hello\0world", ta_get_size(str)));

        str = ta_strndup_append_buffer(str, ", world", 2);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello\0world, "));
        CU_ASSERT_TRUE(!memcmp(str, "hello\0world, ", ta_get_size(str)));

        str = ta_strndup_append_buffer(str, "world\0hello", 1000);
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello\0world, world"));
        CU_ASSERT_TRUE(!memcmp(str, "hello\0world, world", ta_get_size(str)));
        ta_free(str);
    }
    ta_free(tactx);
}

static void test_ta_asprintf(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));

    const char str[] = "hello, world";
    {
        char *ptr = ta_asprintf(tactx, "%.*s", 5, str);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), 6);
        CU_ASSERT_NSTRING_EQUAL(ptr, str, 5);
        ta_free(ptr);
    }
    {
        char *ptr = ta_asprintf(tactx, "%s", str);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), sizeof(str));
        CU_ASSERT_NSTRING_EQUAL(ptr, str, sizeof(str));
        ta_free(ptr);
    }
    {
        char *ptr = ta_asprintf(NULL, "%s", "");
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_NULL(ta_get_parent(ptr));
        CU_ASSERT_EQUAL(ta_get_size(ptr), 1);
        ta_free(ptr);
    }
    ta_free(tactx);
}

static void test_ta_asprintf_append(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    {
        char *str = ta_asprintf(tactx, "%.*s", 5, "hello, world");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello"));
        CU_ASSERT_STRING_EQUAL(str, "hello");

        str = ta_asprintf_append(str, "%.*s", 2, ", world");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, "));
        CU_ASSERT_STRING_EQUAL(str, "hello, ");

        str = ta_asprintf_append(str, "%.*s", 5, "world hello");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, world"));
        CU_ASSERT_STRING_EQUAL(str, "hello, world");
        ta_free(str);
    }
    {
        char *str = ta_asprintf(tactx, "%s", "hello\0world");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello"));
        CU_ASSERT_STRING_EQUAL(str, "hello");

        str = ta_asprintf_append(str, "%.*s", 2, ", world");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, "));
        CU_ASSERT_STRING_EQUAL(str, "hello, ");

        str = ta_asprintf_append(str, "%s", "world\0hello");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, world"));
        CU_ASSERT_STRING_EQUAL(str, "hello, world");
        ta_free(str);
    }
    ta_free(tactx);
}

static void test_ta_asprintf_append_buffer(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));
    {
        char *str = ta_asprintf(tactx, "%.*s", 5, "hello, world");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello"));
        CU_ASSERT_STRING_EQUAL(str, "hello");

        str = ta_asprintf_append_buffer(str, "%.*s", 2, ", world");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, "));
        CU_ASSERT_STRING_EQUAL(str, "hello, ");

        str = ta_asprintf_append_buffer(str, "%s", "world\0hello");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello, world"));
        CU_ASSERT_STRING_EQUAL(str, "hello, world");
        ta_free(str);
    }
    {
        char *str = ta_memdup(tactx, "hello\0world", sizeof("hello\0world"));
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello\0world"));
        CU_ASSERT_TRUE(!memcmp(str, "hello\0world", ta_get_size(str)));

        str = ta_asprintf_append_buffer(str, "%.*s", 2, ", world");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello\0world, "));
        CU_ASSERT_TRUE(!memcmp(str, "hello\0world, ", ta_get_size(str)));

        str = ta_asprintf_append_buffer(str, "%s", "world\0hello");
        CU_ASSERT_PTR_NOT_NULL(str);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(str), tactx);
        CU_ASSERT_EQUAL(ta_get_size(str), sizeof("hello\0world, world"));
        CU_ASSERT_TRUE(!memcmp(str, "hello\0world, world", ta_get_size(str)));
        ta_free(str);
    }
    ta_free(tactx);
}

struct ctx {
    int *a;
};

static void ctx_destructor(void *ptr)
{
    struct ctx *ctx = (struct ctx *)ptr;
    *ctx->a = 2;
}

static void test_ta_destructor(void)
{
    int a = 1;
    struct ctx *ctx = ta_alloc(NULL, sizeof(struct ctx));
    CU_ASSERT_PTR_NULL(ta_get_destructor(ctx));
    ctx->a = &a;
    ta_set_destructor(ctx, ctx_destructor);
    CU_ASSERT_PTR_EQUAL(ta_get_destructor(ctx), ctx_destructor);
    ta_free(ctx);
    CU_ASSERT_EQUAL(a, 2);
}

static void test_ta_foreach(void)
{
    void *tactx = ta_alloc(NULL, 0);
    CU_ASSERT_PTR_NOT_NULL(tactx);
    CU_ASSERT_PTR_NULL(ta_get_parent(tactx));

    size_t j = 5;
    for (size_t i = 0; i < j; ++i) {
        void *ptr = ta_alloc(tactx, i);
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
    }

    void *ptr;
    TA_FOREACH(ptr, tactx) {
        CU_ASSERT_PTR_NOT_NULL(ptr);
        CU_ASSERT_PTR_EQUAL(ta_get_parent(ptr), tactx);
        CU_ASSERT_EQUAL(ta_get_size(ptr), --j);
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

    if (!CU_ADD_TEST(pSuite, test_ta_xmalloc) ||
        !CU_ADD_TEST(pSuite, test_ta_xcalloc) ||
        !CU_ADD_TEST(pSuite, test_ta_xrealloc) ||
        !CU_ADD_TEST(pSuite, test_ta_xzalloc) ||
        !CU_ADD_TEST(pSuite, test_ta_xstrdup) ||
        !CU_ADD_TEST(pSuite, test_ta_xstrndup) ||
        !CU_ADD_TEST(pSuite, test_ta_xmemdup) ||
        !CU_ADD_TEST(pSuite, test_ta_get_parent) ||
        !CU_ADD_TEST(pSuite, test_ta_get_size) ||
        !CU_ADD_TEST(pSuite, test_ta_get_child) ||
        !CU_ADD_TEST(pSuite, test_ta_get_next) ||
        !CU_ADD_TEST(pSuite, test_ta_get_prev) ||
        !CU_ADD_TEST(pSuite, test_ta_has_parent) ||
        !CU_ADD_TEST(pSuite, test_ta_has_child) ||
        !CU_ADD_TEST(pSuite, test_ta_alloc) ||
        !CU_ADD_TEST(pSuite, test_ta_zalloc) ||
        !CU_ADD_TEST(pSuite, test_ta_realloc) ||
        !CU_ADD_TEST(pSuite, test_ta_alloc_array) ||
        !CU_ADD_TEST(pSuite, test_ta_zalloc_array) ||
        !CU_ADD_TEST(pSuite, test_ta_realloc_array) ||
        !CU_ADD_TEST(pSuite, test_ta_assign) ||
        !CU_ADD_TEST(pSuite, test_ta_memdup) ||
        !CU_ADD_TEST(pSuite, test_ta_strdup) ||
        !CU_ADD_TEST(pSuite, test_ta_strdup_append) ||
        !CU_ADD_TEST(pSuite, test_ta_strdup_append_buffer) ||
        !CU_ADD_TEST(pSuite, test_ta_strndup) ||
        !CU_ADD_TEST(pSuite, test_ta_strndup_append) ||
        !CU_ADD_TEST(pSuite, test_ta_strndup_append_buffer) ||
        !CU_ADD_TEST(pSuite, test_ta_asprintf) ||
        !CU_ADD_TEST(pSuite, test_ta_asprintf_append) ||
        !CU_ADD_TEST(pSuite, test_ta_asprintf_append_buffer) ||
        !CU_ADD_TEST(pSuite, test_ta_destructor) ||
        !CU_ADD_TEST(pSuite, test_ta_foreach)) {
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
