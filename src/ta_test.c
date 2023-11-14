#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "ta.h"

#define TEST(func) static void func(const char *__unit)

// GCOVR_EXCL_START
static void assert_expr(const char *unit, bool result,
                        const char *const expression,
                        const char *const file, const int line)
{
    if (__ta_likely(result))
        return;

    fprintf(stderr, "%s:%d: unit test '%s' assertion failed: (%s)\n",
            file, line, unit, expression);
    fflush(stderr);
    abort();
}
// GCOVR_EXCL_STOP

#define assert_true(x) assert_expr(__unit, !!(x), #x, __FILE__, __LINE__)
#define assert_false(x) assert_true(!(x))

#define assert_null(x) assert_true((x) == NULL)
#define assert_not_null(x) assert_true((x) != NULL)

#define assert_equal(a, b) assert_true((a) == (b))
#define assert_not_equal(a, b) assert_true((a) != (b))

#define assert_str_equal(a, b) assert_true(strcmp(a, b) == 0)
#define assert_str_not_equal(a, b) assert_true(strcmp(a, b) != 0)

#define assert_strn_equal(a, b, n) assert_true(strncmp(a, b, n) == 0)
#define assert_strn_not_equal(a, b, n) assert_true(strncmp(a, b, n) != 0)

#define assert_mem_equal(a, b, n) assert_true(memcmp(a, b, n) == 0)
#define assert_mem_not_equal(a, b, n) assert_true(memcmp(a, b, n) != 0)

TEST(test_ta_xmalloc)
{
    for (size_t i = 0; i < 100; ++i) {
        void *ptr = ta_xmalloc(i);
        assert_not_null(ptr);
        free(ptr);
    }
}

TEST(test_ta_xcalloc)
{
    for (size_t i = 0; i < 10; ++i) {
        for (size_t j = 0; j < 10; ++j) {
            void *ptr = ta_xcalloc(i, j);
            assert_not_null(ptr);
            free(ptr);
        }
    }
}

TEST(test_ta_xrealloc)
{
    void *ptr = ta_xrealloc(NULL, 0);
    assert_not_null(ptr);
    ptr = ta_xrealloc(ptr, 100);
    assert_not_null(ptr);
    ptr = ta_xrealloc(ptr, 0);
    assert_not_null(ptr);
    free(ptr);
}

TEST(test_ta_xzalloc)
{
    for (size_t i = 0; i < 100; ++i) {
        void *ptr = ta_xzalloc(i);
        assert_not_null(ptr);
        free(ptr);
    }
}

#ifndef _WIN32
TEST(test_ta_xmemalign)
{
    for (size_t i = 0; i < 100; ++i) {
        void *ptr = ta_xmemalign(128, i);
        assert_not_null(ptr);
        assert_equal((uintptr_t)ptr % 128, 0);
        free(ptr);
    }
}
#endif

TEST(test_ta_xstrdup)
{
    {
        char *str = ta_xstrdup("");
        assert_not_null(str);
        assert_str_equal(str, "");
        free(str);
    }
    {
        char *str = ta_xstrdup("test");
        assert_not_null(str);
        assert_str_equal(str, "test");
        free(str);
    }
}

TEST(test_ta_xstrndup)
{
    {
        char *str = ta_xstrndup("", 0);
        assert_not_null(str);
        assert_strn_equal(str, "", 0);
        free(str);
    }
    {
        char *str = ta_xstrndup("test", 0);
        assert_not_null(str);
        assert_strn_equal(str, "test", 0);
        free(str);
    }
    {
        char *str = ta_xstrndup("test", 2);
        assert_not_null(str);
        assert_strn_equal(str, "test", 2);
        free(str);
    }
    {
        char *str = ta_xstrndup("test", 5);
        assert_not_null(str);
        assert_strn_equal(str, "test", 5);
        free(str);
    }
}

TEST(test_ta_xmemdup)
{
    {
        char *str = (char *)ta_xmemdup("", 0);
        assert_not_null(str);
        assert_strn_equal(str, "", 0);
        free(str);
    }
    {
        char *str = (char *)ta_xmemdup("test", 0);
        assert_not_null(str);
        assert_strn_equal(str, "test", 0);
        free(str);
    }
    {
        char *str = (char *)ta_xmemdup("test", 2);
        assert_not_null(str);
        assert_strn_equal(str, "test", 2);
        free(str);
    }
    {
        char *str = (char *)ta_xmemdup("test", 5);
        assert_not_null(str);
        assert_str_equal(str, "test");
        free(str);
    }
}

TEST(test_ta_xasprintf)
{
    const char str[] = "hello, world";
    {
        char *ptr = ta_xasprintf("%.*s", 5, str);
        assert_not_null(ptr);
        assert_equal(strlen(ptr), 5);
        assert_strn_equal(ptr, str, 5);
        free(ptr);
    }
    {
        char *ptr = ta_xasprintf("%s", str);
        assert_not_null(ptr);
        assert_str_equal(ptr, str);
        free(ptr);
    }
    {
        char *ptr = ta_xasprintf("%s", "");
        assert_not_null(ptr);
        assert_equal(strlen(ptr), 0);
        free(ptr);
    }
}

TEST(test_ta_set_parent)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));

    void *arr[10];
    for (size_t i = 0; i < 10; ++i) {
        void *ptr = arr[i] = ta_alloc(tactx, i);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), i);
        assert_equal(ta_set_parent(ptr, tactx), ptr);
        assert_equal(ta_get_parent(ptr), tactx);
    }

    ta_set_parent(arr[8], arr[9]);
    ta_set_parent(arr[2], arr[1]);
    ta_set_parent(arr[9], tactx);
    ta_set_parent(arr[0], tactx);
    ta_set_parent(arr[3], NULL);
    ta_set_parent(arr[7], NULL);
    ta_free(arr[3]);
    ta_free(arr[7]);
    ta_free(tactx);
}

TEST(test_ta_get_parent)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));

    for (size_t i = 0; i < 5; ++i) {
        void *ptr = ta_get_parent(ta_alloc(tactx, i));
        assert_not_null(ptr);
        assert_equal(ptr, tactx);
    }

    tactx = ta_realloc(NULL, tactx, 100);
    assert_not_null(tactx);
    assert_not_null(ta_get_child(tactx));

    ta_free_children(tactx);
    ta_free(tactx);
}

TEST(test_ta_get_size)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    assert_equal(ta_get_size(tactx), 0);

    for (size_t i = 0; i < 5; ++i) {
        void *ptr = ta_alloc(tactx, i);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), i);
    }

    tactx = ta_realloc(NULL, tactx, 100);
    assert_not_null(tactx);
    assert_not_null(ta_get_child(tactx));

    ta_free_children(tactx);
    ta_free(tactx);
}

TEST(test_ta_get_child)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    assert_null(ta_get_child(tactx));

    for (size_t i = 0; i < 5; ++i) {
        void *ptr = ta_alloc(tactx, i);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_child(tactx), ptr);
    }

    for (size_t i = 0; i < 3; ++i) {
        void *ptr = ta_get_child(tactx);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        ta_free(ptr);
        assert_not_equal(ta_get_child(tactx), ptr);
    }

    {
        void *ptr = ta_get_child(tactx);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), 1);
        ta_set_parent(ptr, NULL);
        ta_free(ptr);
    }

    {
        void *ptr = ta_get_child(tactx);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), 0);
        ta_set_parent(ptr, NULL);
        ta_free(ptr);
    }

    {
        void *ptr = ta_get_child(tactx);
        assert_null(ptr);
    }

    ta_free(tactx);
}

TEST(test_ta_get_next)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    assert_null(ta_get_next(tactx));

    for (size_t i = 0; i < 5; ++i) {
        void *tmp = ta_get_child(tactx);
        void *ptr = ta_alloc(tactx, i);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_next(ptr), tmp);
    }

    for (size_t i = 0; i < 3; ++i) {
        void *tmp = ta_get_child(tactx);
        assert_not_null(tmp);
        void *ptr = ta_get_next(tmp);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        ta_free(tmp);
        assert_equal(ta_get_child(tactx), ptr);
    }

    {
        void *tmp = ta_get_child(tactx);
        assert_not_null(tmp);
        void *ptr = ta_get_next(tmp);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(tmp), 1);
        assert_equal(ta_get_size(ptr), 0);
        ta_free(tmp);
    }

    {
        void *tmp = ta_get_child(tactx);
        assert_not_null(tmp);
        void *ptr = ta_get_next(tmp);
        assert_null(ptr);
        assert_equal(ta_get_parent(tmp), tactx);
        assert_equal(ta_get_size(tmp), 0);
        ta_free(tmp);
    }

    ta_free(tactx);
}

TEST(test_ta_get_prev)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    assert_null(ta_get_prev(tactx));

    for (size_t i = 0; i < 5; ++i) {
        void *tmp = ta_get_child(tactx);
        void *ptr = ta_alloc(tactx, i);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_prev(ptr), NULL);
        if (tmp)
            assert_equal(ta_get_prev(tmp), ptr);
    }

    for (size_t i = 0; i < 3; ++i) {
        void *ptr = ta_get_child(tactx);
        assert_not_null(ptr);
        assert_null(ta_get_prev(ptr));
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_prev(ta_get_next(ptr)), ptr);
        ta_free(ptr);
    }

    ta_free(tactx);
}

TEST(test_ta_has_parent)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    assert_false(ta_has_parent(tactx, NULL));
    assert_false(ta_has_parent(tactx, tactx));

    void *ptr1 = ta_alloc(tactx, 0);
    assert_not_null(ptr1);
    assert_equal(ta_get_parent(ptr1), tactx);
    assert_false(ta_has_parent(ptr1, NULL));
    assert_false(ta_has_parent(ptr1, ptr1));
    assert_true(ta_has_parent(ptr1, tactx));
    assert_false(ta_has_parent(tactx, ptr1));

    void *ptr2 = ta_alloc(tactx, 0);
    assert_not_null(ptr2);
    assert_equal(ta_get_parent(ptr2), tactx);
    assert_false(ta_has_parent(ptr2, NULL));
    assert_false(ta_has_parent(ptr2, ptr2));
    assert_true(ta_has_parent(ptr2, tactx));
    assert_false(ta_has_parent(tactx, ptr2));
    assert_false(ta_has_parent(ptr2, ptr1));
    assert_false(ta_has_parent(ptr1, ptr2));

    void *ptr3 = ta_alloc(ptr1, 0);
    assert_not_null(ptr3);
    assert_equal(ta_get_parent(ptr3), ptr1);
    assert_false(ta_has_parent(ptr3, NULL));
    assert_false(ta_has_parent(ptr3, ptr3));
    assert_true(ta_has_parent(ptr3, tactx));
    assert_false(ta_has_parent(tactx, ptr3));
    assert_true(ta_has_parent(ptr3, ptr1));
    assert_false(ta_has_parent(ptr1, ptr3));
    assert_false(ta_has_parent(ptr3, ptr2));
    assert_false(ta_has_parent(ptr2, ptr3));

    ta_free(ptr2);
    ta_free(tactx);
}

TEST(test_ta_has_child)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    assert_false(ta_has_child(tactx, NULL));
    assert_false(ta_has_child(tactx, tactx));

    void *ptr1 = ta_alloc(tactx, 0);
    assert_not_null(ptr1);
    assert_equal(ta_get_parent(ptr1), tactx);
    assert_false(ta_has_child(ptr1, NULL));
    assert_false(ta_has_child(ptr1, ptr1));
    assert_false(ta_has_child(ptr1, tactx));
    assert_true(ta_has_child(tactx, ptr1));

    void *ptr2 = ta_alloc(tactx, 0);
    assert_not_null(ptr2);
    assert_equal(ta_get_parent(ptr2), tactx);
    assert_false(ta_has_child(ptr2, NULL));
    assert_false(ta_has_child(ptr2, ptr2));
    assert_false(ta_has_child(ptr2, tactx));
    assert_true(ta_has_child(tactx, ptr2));
    assert_false(ta_has_child(ptr2, ptr1));
    assert_false(ta_has_child(ptr1, ptr2));

    void *ptr3 = ta_alloc(ptr1, 0);
    assert_not_null(ptr3);
    assert_equal(ta_get_parent(ptr3), ptr1);
    assert_false(ta_has_child(ptr3, NULL));
    assert_false(ta_has_child(ptr3, ptr3));
    assert_false(ta_has_child(ptr3, tactx));
    assert_true(ta_has_child(tactx, ptr3));
    assert_false(ta_has_child(ptr3, ptr1));
    assert_true(ta_has_child(ptr1, ptr3));
    assert_false(ta_has_child(ptr3, ptr2));
    assert_false(ta_has_child(ptr2, ptr3));

    ta_free(ptr2);
    ta_free(tactx);
}

TEST(test_ta_move_children)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    ta_move_children(tactx, NULL);
    {
        void *src = ta_alloc(tactx, 0);

        void *ptr1 = ta_alloc(src, 1);
        void *ptr2 = ta_alloc(src, 2);
        void *ptr3 = ta_alloc(src, 3);

        ta_move_children(src, NULL);

        assert_null(ta_get_child(src));

        assert_false(ta_has_child(src, ptr1));
        assert_false(ta_has_child(src, ptr2));
        assert_false(ta_has_child(src, ptr3));

        assert_null(ta_get_parent(ptr1));
        assert_null(ta_get_parent(ptr2));
        assert_null(ta_get_parent(ptr3));

        ta_free(ptr1);
        ta_free(ptr2);
        ta_free(ptr3);
    }
    {
        void *src = ta_alloc(tactx, 0);
        void *dst = ta_alloc(tactx, 0);

        void *ptr1 = ta_alloc(src, 1);
        void *ptr2 = ta_alloc(src, 2);
        void *ptr3 = ta_alloc(src, 3);

        ta_move_children(src, dst);

        assert_null(ta_get_child(src));
        assert_not_null(ta_get_child(dst));

        assert_false(ta_has_child(src, ptr1));
        assert_false(ta_has_child(src, ptr2));
        assert_false(ta_has_child(src, ptr3));

        assert_equal(ta_get_parent(ptr1), dst);
        assert_equal(ta_get_parent(ptr2), dst);
        assert_equal(ta_get_parent(ptr3), dst);

        assert_true(ta_has_child(dst, ptr1));
        assert_true(ta_has_child(dst, ptr2));
        assert_true(ta_has_child(dst, ptr3));
    }
    {
        void *src = ta_alloc(tactx, 0);
        void *dst = ta_alloc(tactx, 0);

        void *ptr1 = ta_alloc(src, 1);
        void *ptr2 = ta_alloc(src, 2);
        void *ptr3 = ta_alloc(src, 3);

        void *ptr4 = ta_alloc(dst, 4);
        void *ptr5 = ta_alloc(dst, 5);
        void *ptr6 = ta_alloc(dst, 6);

        ta_move_children(src, dst);

        assert_null(ta_get_child(src));
        assert_not_null(ta_get_child(dst));

        assert_false(ta_has_child(src, ptr1));
        assert_false(ta_has_child(src, ptr2));
        assert_false(ta_has_child(src, ptr3));

        assert_equal(ta_get_parent(ptr1), dst);
        assert_equal(ta_get_parent(ptr2), dst);
        assert_equal(ta_get_parent(ptr3), dst);
        assert_equal(ta_get_parent(ptr4), dst);
        assert_equal(ta_get_parent(ptr5), dst);
        assert_equal(ta_get_parent(ptr6), dst);

        assert_true(ta_has_child(dst, ptr1));
        assert_true(ta_has_child(dst, ptr2));
        assert_true(ta_has_child(dst, ptr3));
        assert_true(ta_has_child(dst, ptr4));
        assert_true(ta_has_child(dst, ptr5));
        assert_true(ta_has_child(dst, ptr6));

        assert_equal(ta_get_next(ptr4), ptr3);
        assert_equal(ta_get_prev(ptr3), ptr4);
        assert_null(ta_get_next(ptr1));
        assert_null(ta_get_prev(ptr6));
    }
    ta_free(tactx);
}

TEST(test_ta_alloc)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    {
        void *ptr = ta_alloc(tactx, sizeof(int));
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(int));
        ta_free(ptr);
    }
    {
        void *ptr = ta_alloc(tactx, sizeof(NULL));
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(NULL));
        ta_free(ptr);
    }
    for (size_t i = 0; i < 10; ++i) {
        void *ptr = ta_alloc(NULL, i);
        assert_not_null(ptr);
        assert_null(ta_get_parent(ptr));
        assert_equal(ta_get_size(ptr), i);
        ta_free(ptr);
    }
    ta_free(tactx);
}

TEST(test_ta_zalloc)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    {
        void *ptr = ta_zalloc(tactx, sizeof(int));
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(int));
        assert_equal(*(int *)ptr, 0);
        ta_free(ptr);
    }
    {
        void *ptr = ta_zalloc(tactx, sizeof(NULL));
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(NULL));
        assert_equal(*(uintptr_t *)ptr, 0);
        ta_free(ptr);
    }
    for (size_t i = 0; i < 10; ++i) {
        void *ptr = ta_zalloc(NULL, i);
        assert_not_null(ptr);
        assert_null(ta_get_parent(ptr));
        assert_equal(ta_get_size(ptr), i);
        for (size_t j = 0; j < i; ++j)
            assert_equal(((uint8_t *)ptr)[j], 0);
        ta_free(ptr);
    }
    ta_free(tactx);
}

TEST(test_ta_realloc)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));

    void *ptr = NULL;

    ptr = ta_realloc(tactx, ptr, 10);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), 10);

    ptr = ta_realloc(tactx, ptr, 5000);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), 5000);

    ptr = ta_realloc(NULL, ptr, 5);
    assert_not_null(ptr);
    assert_null(ta_get_parent(ptr));
    assert_equal(ta_get_size(ptr), 5);

    ptr = ta_realloc(tactx, ptr, 1);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), 1);

    ptr = ta_realloc(tactx, ptr, 0);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), 0);

    ptr = ta_realloc(NULL, ptr, 0);
    assert_not_null(ptr);
    assert_null(ta_get_parent(ptr));
    assert_equal(ta_get_size(ptr), 0);

    ptr = ta_realloc(tactx, ptr, 25);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), 25);

    void *arr[10];
    for (size_t i = 0; i < 10; ++i) {
        ptr = arr[i] = ta_realloc(tactx, NULL, i);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), i);
    }

    ptr = ta_realloc(tactx, arr[3], 30);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), 30);

    ptr = ta_realloc(tactx, arr[6], 60);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), 60);

    ptr = ta_realloc(tactx, arr[0], 10);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), 10);

    ptr = ta_realloc(tactx, arr[9], 90);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), 90);

    ptr = ta_realloc(tactx, arr[4], 40);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), 40);

    ptr = ta_realloc(tactx, arr[5], 50);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), 50);

    ta_free(tactx);
}

TEST(test_ta_memdup)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));

    const char str[] = "hello, world";
    {
        char *ptr = (char *)ta_memdup(tactx, str, sizeof(str));
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(str));
        assert_str_equal(ptr, str);
        ta_free(ptr);
    }
    {
        char *ptr = (char *)ta_memdup(NULL, str, 0);
        assert_not_null(ptr);
        assert_null(ta_get_parent(ptr));
        assert_equal(ta_get_size(ptr), 0);
        ta_free(ptr);
    }
    ta_free(tactx);
}

TEST(test_ta_assign)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    {
        void *ptr = ta_assign(tactx, NULL, sizeof(int));
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(int));
        ta_free(ptr);
    }
    for (size_t i = 0; i < 10; ++i) {
        void *ptr = ta_assign(tactx, NULL, i);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), i);
        ta_free(ptr);
    }
    for (size_t i = 0; i < 10; ++i) {
        void *ptr = malloc(i * 2);
        assert_not_null(ptr);
        ptr = ta_assign(tactx, ptr, i * 2);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), i * 2);
        ta_free(ptr);
    }
    for (size_t i = 0; i < 10; ++i) {
        void *ptr = malloc(i * 2);
        assert_not_null(ptr);
        ptr = ta_assign(tactx, ptr, i);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), i);
        ta_free(ptr);
    }
    {
        void *ptr = malloc(sizeof(int));
        assert_not_null(ptr);
        ptr = ta_assign(NULL, ptr, sizeof(int));
        assert_not_null(ptr);
        assert_null(ta_get_parent(ptr));
        assert_equal(ta_get_size(ptr), sizeof(int));
        ta_free(ptr);
    }
    ta_free(tactx);
}

TEST(test_ta_alloc_array)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    {
        int *ptr = (int *)ta_alloc_array(tactx, sizeof(*ptr), 10);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(int) * 10);
        ta_free(ptr);
    }
    {
        int *ptr = (int *)ta_alloc_array(tactx, sizeof(*ptr), 5);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(int) * 5);
        ta_free(ptr);
    }
    {
        int *ptr = (int *)ta_alloc_array(NULL, sizeof(*ptr), 1);
        assert_not_null(ptr);
        assert_null(ta_get_parent(ptr));
        assert_equal(ta_get_size(ptr), sizeof(int) * 1);
        ta_free(ptr);
    }
    {
        int *ptr = (int *)ta_alloc_array(tactx, sizeof(*ptr), 0);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), 0);
        ta_free(ptr);
    }
    {
        int *ptr = (int *)ta_alloc_array(NULL, sizeof(*ptr), 0);
        assert_not_null(ptr);
        assert_null(ta_get_parent(ptr));
        assert_equal(ta_get_size(ptr), 0);
        ta_free(ptr);
    }
    {
        int *ptr = (int *)ta_alloc_array(tactx, sizeof(*ptr), 25);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(int) * 25);
        ta_free(ptr);
    }
    ta_free(tactx);
}

TEST(test_ta_zalloc_array)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    {
        int *ptr = (int *)ta_zalloc_array(tactx, sizeof(*ptr), 10);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(int) * 10);
        for (size_t i = 0; i < 10; ++i)
            assert_equal(ptr[i], 0);
        ta_free(ptr);
    }
    {
        int *ptr = (int *)ta_zalloc_array(tactx, sizeof(*ptr), 5);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(int) * 5);
        for (size_t i = 0; i < 5; ++i)
            assert_equal(ptr[i], 0);
        ta_free(ptr);
    }
    {
        int *ptr = (int *)ta_zalloc_array(NULL, sizeof(*ptr), 1);
        assert_not_null(ptr);
        assert_null(ta_get_parent(ptr));
        assert_equal(ta_get_size(ptr), sizeof(int) * 1);
        assert_equal(ptr[0], 0);
        ta_free(ptr);
    }
    {
        int *ptr = (int *)ta_zalloc_array(tactx, sizeof(*ptr), 0);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), 0);
        ta_free(ptr);
    }
    {
        int *ptr = (int *)ta_zalloc_array(NULL, sizeof(*ptr), 0);
        assert_not_null(ptr);
        assert_null(ta_get_parent(ptr));
        assert_equal(ta_get_size(ptr), 0);
        ta_free(ptr);
    }
    {
        int *ptr = (int *)ta_zalloc_array(tactx, sizeof(*ptr), 25);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(int) * 25);
        for (size_t i = 0; i < 25; ++i)
            assert_equal(ptr[i], 0);
        ta_free(ptr);
    }
    ta_free(tactx);
}

TEST(test_ta_realloc_array)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));

    int *ptr = NULL;

    ptr = (int *)ta_realloc_array(tactx, ptr, sizeof(*ptr), 10);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), sizeof(int) * 10);

    ptr = (int *)ta_realloc_array(tactx, ptr, sizeof(*ptr), 5);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), sizeof(int) * 5);

    ptr = (int *)ta_realloc_array(NULL, ptr, sizeof(*ptr), 5);
    assert_not_null(ptr);
    assert_null(ta_get_parent(ptr));
    assert_equal(ta_get_size(ptr), sizeof(int) * 5);

    ptr = (int *)ta_realloc_array(tactx, ptr, sizeof(*ptr), 1);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), sizeof(int) * 1);

    ptr = (int *)ta_realloc_array(tactx, ptr, sizeof(*ptr), 0);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), 0);

    ptr = (int *)ta_realloc_array(NULL, ptr, sizeof(*ptr), 0);
    assert_not_null(ptr);
    assert_null(ta_get_parent(ptr));
    assert_equal(ta_get_size(ptr), 0);

    ptr = (int *)ta_realloc_array(tactx, ptr, sizeof(*ptr), 25);
    assert_not_null(ptr);
    assert_equal(ta_get_parent(ptr), tactx);
    assert_equal(ta_get_size(ptr), sizeof(int) * 25);

    ta_free(tactx);
}

TEST(test_ta_memdup_array)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));

    const char str[] = "hello, world";
    {
        char *ptr = (char *)ta_memdup_array(tactx, str, 1, sizeof(str));
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(str));
        assert_str_equal(ptr, str);
        ta_free(ptr);
    }
    {
        char *ptr = (char *)ta_memdup_array(NULL, str, 1, 0);
        assert_not_null(ptr);
        assert_null(ta_get_parent(ptr));
        assert_equal(ta_get_size(ptr), 0);
        ta_free(ptr);
    }
    ta_free(tactx);
}

TEST(test_ta_assign_array)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    {
        void *ptr = ta_assign_array(tactx, NULL, sizeof(int), 5);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(int) * 5);
        ta_free(ptr);
    }
    for (size_t i = 0; i < 10; ++i) {
        void *ptr = ta_assign_array(tactx, NULL, 5, i);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), i * 5);
        ta_free(ptr);
    }
    for (size_t i = 0; i < 10; ++i) {
        void *ptr = malloc(i * 2);
        assert_not_null(ptr);
        ptr = ta_assign_array(tactx, ptr, 2, i);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), i * 2);
        ta_free(ptr);
    }
    for (size_t i = 0; i < 10; ++i) {
        void *ptr = malloc(i * 2);
        assert_not_null(ptr);
        ptr = ta_assign_array(tactx, ptr, 1, i);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), i);
        ta_free(ptr);
    }
    {
        void *ptr = malloc(sizeof(int));
        assert_not_null(ptr);
        ptr = ta_assign_array(NULL, ptr, sizeof(int), 1);
        assert_not_null(ptr);
        assert_null(ta_get_parent(ptr));
        assert_equal(ta_get_size(ptr), sizeof(int));
        ta_free(ptr);
    }
    ta_free(tactx);
}

TEST(test_ta_free)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));

    void *arr[10];
    for (size_t i = 0; i < 10; ++i) {
        void *ptr = arr[i] = ta_alloc(tactx, i);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), i);
    }

    ta_free(arr[3]);
    ta_free(arr[6]);
    ta_free(arr[0]);
    ta_free(arr[9]);
    ta_free(arr[4]);
    ta_free(arr[5]);
    ta_free(tactx);
}

TEST(test_ta_strdup)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));

    const char str[] = "hello, world";
    {
        char *ptr = ta_strdup(tactx, str);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(str));
        assert_str_equal(ptr, str);
        ta_free(ptr);
    }
    {
        char *ptr = ta_strdup(NULL, "");
        assert_not_null(ptr);
        assert_null(ta_get_parent(ptr));
        assert_equal(ta_get_size(ptr), 1);
        ta_free(ptr);
    }
    ta_free(tactx);
}

TEST(test_ta_strdup_append)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    {
        char *str = ta_strdup(tactx, "hello");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello"));
        assert_str_equal(str, "hello");

        str = ta_strdup_append(str, ", ");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, "));
        assert_str_equal(str, "hello, ");

        str = ta_strdup_append(str, "world");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, world"));
        assert_str_equal(str, "hello, world");
        ta_free(str);
    }
    {
        char *str = ta_strdup(tactx, "hello\0world");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello"));
        assert_str_equal(str, "hello");

        str = ta_strdup_append(str, ", ");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, "));
        assert_str_equal(str, "hello, ");

        str = ta_strdup_append(str, "world\0hello");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, world"));
        assert_str_equal(str, "hello, world");
        ta_free(str);
    }
    ta_free(tactx);
}

TEST(test_ta_strdup_append_buffer)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    {
        char *str = ta_strdup(tactx, "hello");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello"));
        assert_str_equal(str, "hello");

        str = ta_strdup_append_buffer(str, ", ");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, "));
        assert_str_equal(str, "hello, ");

        str = ta_strdup_append_buffer(str, "world");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, world"));
        assert_str_equal(str, "hello, world");
        ta_free(str);
    }
    {
        char *str = (char *)ta_memdup(tactx, "hello\0world", sizeof("hello\0world"));
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello\0world"));
        assert_true(!memcmp(str, "hello\0world", ta_get_size(str)));

        str = ta_strdup_append_buffer(str, ", ");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello\0world, "));
        assert_true(!memcmp(str, "hello\0world, ", ta_get_size(str)));

        str = ta_strdup_append_buffer(str, "world\0hello");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello\0world, world"));
        assert_true(!memcmp(str, "hello\0world, world", ta_get_size(str)));
        ta_free(str);
    }
    ta_free(tactx);
}

TEST(test_ta_strndup)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));

    const char str[] = "hello, world";
    {
        char *ptr = ta_strndup(tactx, str, 5);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), 6);
        assert_strn_equal(ptr, str, 5);
        ta_free(ptr);
    }
    {
        char *ptr = ta_strndup(tactx, str, 1000);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(str));
        assert_strn_equal(ptr, str, sizeof(str));
        ta_free(ptr);
    }
    {
        char *ptr = ta_strndup(NULL, str, 0);
        assert_not_null(ptr);
        assert_null(ta_get_parent(ptr));
        assert_equal(ta_get_size(ptr), 1);
        ta_free(ptr);
    }
    ta_free(tactx);
}

TEST(test_ta_strndup_append)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    {
        char *str = ta_strndup(tactx, "hello, world", 5);
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello"));
        assert_str_equal(str, "hello");

        str = ta_strndup_append(str, ", world", 2);
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, "));
        assert_str_equal(str, "hello, ");

        str = ta_strndup_append(str, "world hello", 5);
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, world"));
        assert_str_equal(str, "hello, world");
        ta_free(str);
    }
    {
        char *str = ta_strndup(tactx, "hello\0world", 1000);
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello"));
        assert_str_equal(str, "hello");

        str = ta_strndup_append(str, ", world", 2);
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, "));
        assert_str_equal(str, "hello, ");

        str = ta_strndup_append(str, "world\0hello", 1000);
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, world"));
        assert_str_equal(str, "hello, world");
        ta_free(str);
    }
    ta_free(tactx);
}

TEST(test_ta_strndup_append_buffer)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    {
        char *str = ta_strndup(tactx, "hello, world", 5);
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello"));
        assert_str_equal(str, "hello");

        str = ta_strndup_append_buffer(str, ", world", 2);
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, "));
        assert_str_equal(str, "hello, ");

        str = ta_strndup_append_buffer(str, "world\0hello", 1000);
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, world"));
        assert_str_equal(str, "hello, world");
        ta_free(str);
    }
    {
        char *str = (char *)ta_memdup(tactx, "hello\0world", sizeof("hello\0world"));
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello\0world"));
        assert_true(!memcmp(str, "hello\0world", ta_get_size(str)));

        str = ta_strndup_append_buffer(str, ", world", 2);
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello\0world, "));
        assert_true(!memcmp(str, "hello\0world, ", ta_get_size(str)));

        str = ta_strndup_append_buffer(str, "world\0hello", 1000);
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello\0world, world"));
        assert_true(!memcmp(str, "hello\0world, world", ta_get_size(str)));
        ta_free(str);
    }
    ta_free(tactx);
}

TEST(test_ta_asprintf)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));

    const char str[] = "hello, world";
    {
        char *ptr = ta_asprintf(tactx, "%.*s", 5, str);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), 6);
        assert_strn_equal(ptr, str, 5);
        ta_free(ptr);
    }
    {
        char *ptr = ta_asprintf(tactx, "%s", str);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), sizeof(str));
        assert_strn_equal(ptr, str, sizeof(str));
        ta_free(ptr);
    }
    {
        char *ptr = ta_asprintf(NULL, "%s", "");
        assert_not_null(ptr);
        assert_null(ta_get_parent(ptr));
        assert_equal(ta_get_size(ptr), 1);
        ta_free(ptr);
    }
    ta_free(tactx);
}

TEST(test_ta_asprintf_append)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    {
        char *str = ta_asprintf(tactx, "%.*s", 5, "hello, world");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello"));
        assert_str_equal(str, "hello");

        str = ta_asprintf_append(str, "%.*s", 2, ", world");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, "));
        assert_str_equal(str, "hello, ");

        str = ta_asprintf_append(str, "%.*s", 5, "world hello");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, world"));
        assert_str_equal(str, "hello, world");
        ta_free(str);
    }
    {
        char *str = ta_asprintf(tactx, "%s", "hello\0world");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello"));
        assert_str_equal(str, "hello");

        str = ta_asprintf_append(str, "%.*s", 2, ", world");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, "));
        assert_str_equal(str, "hello, ");

        str = ta_asprintf_append(str, "%s", "world\0hello");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, world"));
        assert_str_equal(str, "hello, world");
        ta_free(str);
    }
    ta_free(tactx);
}

TEST(test_ta_asprintf_append_buffer)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));
    {
        char *str = ta_asprintf(tactx, "%.*s", 5, "hello, world");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello"));
        assert_str_equal(str, "hello");

        str = ta_asprintf_append_buffer(str, "%.*s", 2, ", world");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, "));
        assert_str_equal(str, "hello, ");

        str = ta_asprintf_append_buffer(str, "%s", "world\0hello");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello, world"));
        assert_str_equal(str, "hello, world");
        ta_free(str);
    }
    {
        char *str = (char *)ta_memdup(tactx, "hello\0world", sizeof("hello\0world"));
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello\0world"));
        assert_true(!memcmp(str, "hello\0world", ta_get_size(str)));

        str = ta_asprintf_append_buffer(str, "%.*s", 2, ", world");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello\0world, "));
        assert_true(!memcmp(str, "hello\0world, ", ta_get_size(str)));

        str = ta_asprintf_append_buffer(str, "%s", "world\0hello");
        assert_not_null(str);
        assert_equal(ta_get_parent(str), tactx);
        assert_equal(ta_get_size(str), sizeof("hello\0world, world"));
        assert_true(!memcmp(str, "hello\0world, world", ta_get_size(str)));
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

TEST(test_ta_destructor)
{
    int a = 1;
    struct ctx *ctx = (struct ctx *)ta_alloc(NULL, sizeof(struct ctx));
    assert_null(ta_get_destructor(ctx));
    ctx->a = &a;
    ta_set_destructor(ctx, ctx_destructor);
    assert_equal(ta_get_destructor(ctx), ctx_destructor);
    ta_free(ctx);
    assert_equal(a, 2);
}

TEST(test_ta_foreach)
{
    void *tactx = ta_alloc(NULL, 0);
    assert_not_null(tactx);
    assert_null(ta_get_parent(tactx));

    size_t j = 5;
    for (size_t i = 0; i < j; ++i) {
        void *ptr = ta_alloc(tactx, i);
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
    }

    void *ptr;
    TA_FOREACH(ptr, tactx) {
        assert_not_null(ptr);
        assert_equal(ta_get_parent(ptr), tactx);
        assert_equal(ta_get_size(ptr), --j);
    }

    ta_free(tactx);
}

int main(void)
{
    struct {
        const char *name;
        void (*func)(const char *__unit);
    } tests[] = {
        { "ta_xmalloc", test_ta_xmalloc },
        { "ta_xcalloc", test_ta_xcalloc },
        { "ta_xrealloc", test_ta_xrealloc },
        { "ta_xzalloc", test_ta_xzalloc },
#ifndef _WIN32
        { "ta_xmemalign", test_ta_xmemalign },
#endif
        { "ta_xstrdup", test_ta_xstrdup },
        { "ta_xstrndup", test_ta_xstrndup },
        { "ta_xmemdup", test_ta_xmemdup },
        { "ta_xasprintf", test_ta_xasprintf },
        { "ta_set_parent", test_ta_set_parent },
        { "ta_get_parent", test_ta_get_parent },
        { "ta_get_size", test_ta_get_size },
        { "ta_get_child", test_ta_get_child },
        { "ta_get_next", test_ta_get_next },
        { "ta_get_prev", test_ta_get_prev },
        { "ta_has_parent", test_ta_has_parent },
        { "ta_has_child", test_ta_has_child },
        { "ta_move_children", test_ta_move_children },
        { "ta_alloc", test_ta_alloc },
        { "ta_zalloc", test_ta_zalloc },
        { "ta_realloc", test_ta_realloc },
        { "ta_memdup", test_ta_memdup },
        { "ta_assign", test_ta_assign },
        { "ta_alloc_array", test_ta_alloc_array },
        { "ta_zalloc_array", test_ta_zalloc_array },
        { "ta_realloc_array", test_ta_realloc_array },
        { "ta_memdup_array", test_ta_memdup_array },
        { "ta_assign_array", test_ta_assign_array },
        { "ta_free", test_ta_free },
        { "ta_strdup", test_ta_strdup },
        { "ta_strdup_append", test_ta_strdup_append },
        { "ta_strdup_append_buffer", test_ta_strdup_append_buffer },
        { "ta_strndup", test_ta_strndup },
        { "ta_strndup_append", test_ta_strndup_append },
        { "ta_strndup_append_buffer", test_ta_strndup_append_buffer },
        { "ta_asprintf", test_ta_asprintf },
        { "ta_asprintf_append", test_ta_asprintf_append },
        { "ta_asprintf_append_buffer", test_ta_asprintf_append_buffer },
        { "ta_destructor", test_ta_destructor },
        { "ta_foreach", test_ta_foreach },
    };

    for (size_t i = 0, n = sizeof(tests) / sizeof(tests[0]); i < n; ++i) {
        printf(">>> Testing (%zu of %zu) %s...\n", i + 1, n, tests[i].name);
        tests[i].func(tests[i].name);
    }

    return EXIT_SUCCESS;
}
