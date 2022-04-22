#include <stdlib.h>
#include <stdint.h>
#include <stdalign.h>
#include <string.h>

#include "ta.h"

#ifndef __ta_packed
#   if defined(__GNUC__) || __ta_has_attribute(__packed__)
#       define __ta_packed __attribute__((__packed__))
#   else
#       define __ta_packed
#   endif
#endif

#ifndef __ta_aligned
#   if defined(__GNUC__) || __ta_has_attribute(__aligned__)
#       define __ta_aligned(x) __attribute__((__aligned__(x)))
#   else
#       define __ta_aligned(x)
#   endif
#endif

#ifndef __ta_inline
#   if defined(__GNUC__) || __ta_has_attribute(__always_inline__)
#       define __ta_inline inline __attribute__((__always_inline__))
#   else
#       define __ta_inline inline
#   endif
#endif

struct ta_header {
    uint32_t magic;
    struct ta_header *list;
    struct ta_header *prev;
    struct ta_header *next;
    size_t size;
    ta_destructor destructor;
} __ta_packed __ta_aligned(alignof(max_align_t));

#ifndef TA_MAGIC
#define TA_MAGIC 0x8FBEA918ul
#endif

#define TA_HDR_SIZE sizeof(struct ta_header)
#define TA_MAX_SIZE ((size_t)PTRDIFF_MAX - TA_HDR_SIZE)

#define TA_HDR_FROM_PTR(ptr) ((struct ta_header *)((uint8_t *)(ptr) - TA_HDR_SIZE))
#define TA_PTR_FROM_HDR(hdr) ((void *)((uint8_t *)(hdr) + TA_HDR_SIZE))

static __ta_inline
struct ta_header *ta_header_from_ptr(const void *ptr)
{
    if (__ta_unlikely((uintptr_t)ptr <= TA_HDR_SIZE))
        abort();

    struct ta_header *h = TA_HDR_FROM_PTR(ptr);
    if (__ta_unlikely(h->magic != TA_MAGIC))
        abort();

    return h;
}

static __ta_inline
void *ta_header_init(struct ta_header *restrict h, size_t size, void *restrict tactx)
{
    *h = (struct ta_header) {
        .magic  = TA_MAGIC,
        .size   = size,
    };

    if (tactx) {
        struct ta_header *h_parent = ta_header_from_ptr(tactx);
        if (h_parent->list) {
            h->next = h_parent->list;
            h->next->prev = h;
        }
        h->prev = h_parent;
        h->prev->list = h;
    }

    return TA_PTR_FROM_HDR(h);
}

static void ta_header_free(struct ta_header *h)
{
    if (h->destructor) {
        h->destructor(TA_PTR_FROM_HDR(h));
        h->destructor = NULL;
    }

    while (h->list)
        ta_header_free(h->list);

    if (h->prev) {
        if (h->prev->list == h) {
            h->prev->list = h->next;
        } else {
            h->prev->next = h->next;
        }
        if (h->next)
            h->next->prev = h->prev;
    }

    h->magic = 0;
    free(h);
}

void *ta_xmalloc(size_t size)
{
    if (__ta_unlikely(!size))
        size = 1;
    void *ptr = malloc(size);
    if (__ta_unlikely(!ptr))
        abort();
    return ptr;
}

void *ta_xcalloc(size_t n, size_t size)
{
    if (__ta_unlikely(!n || !size))
        n = size = 1;
    void *ptr = calloc(n, size);
    if (__ta_unlikely(!ptr))
        abort();
    return ptr;
}

void *ta_xrealloc(void *ptr, size_t size)
{
    if (__ta_unlikely(!size))
        size = 1;
    ptr = ptr ? realloc(ptr, size) : malloc(size);
    if (__ta_unlikely(!ptr))
        abort();
    return ptr;
}

void *ta_xzalloc(size_t size)
{
    if (__ta_unlikely(!size))
        size = 1;
    void *ptr = calloc(1, size);
    if (__ta_unlikely(!ptr))
        abort();
    return ptr;
}

char *ta_xstrdup(const char *str)
{
    if (__ta_unlikely(!str))
        abort();
    char *ptr = strdup(str);
    if (__ta_unlikely(!ptr))
        abort();
    return ptr;
}

char *ta_xstrndup(const char *str, size_t n)
{
    if (__ta_unlikely(!str))
        abort();
    char *ptr = strndup(str, n);
    if (__ta_unlikely(!ptr))
        abort();
    return ptr;
}

void *ta_xmemdup(const void *mem, size_t n)
{
    if (__ta_unlikely(!mem))
        abort();
    if (__ta_unlikely(!n)) {
        void *ptr = malloc(1);
        if (__ta_unlikely(!ptr))
            abort();
        return ptr;
    }
    void *ptr = malloc(n);
    if (__ta_unlikely(!ptr))
        abort();
    return memcpy(ptr, mem, n);
}

void *ta_alloc(void *tactx, size_t size)
{
    if (__ta_unlikely(size > TA_MAX_SIZE))
        abort();

    struct ta_header *h = malloc(TA_HDR_SIZE + size);
    if (__ta_unlikely(!h))
        abort();

    return ta_header_init(h, size, tactx);
}

void *ta_zalloc(void *tactx, size_t size)
{
    if (__ta_unlikely(size > TA_MAX_SIZE))
        abort();

    struct ta_header *h = calloc(1, TA_HDR_SIZE + size);
    if (__ta_unlikely(!h))
        abort();

    return ta_header_init(h, size, tactx);
}

void *ta_realloc(void *restrict tactx, void *restrict ptr, size_t size)
{
    if (!ptr)
        return ta_alloc(tactx, size);

    if (__ta_unlikely(size > TA_MAX_SIZE))
        abort();

    struct ta_header *h = ta_header_from_ptr(ptr);
    struct ta_header *h_old = h;

    h = realloc(h, TA_HDR_SIZE + size);
    if (__ta_unlikely(!h))
        abort();

    h->size = size;

    if (h != h_old) {
        if (h->list)
            h->list->prev = h;
        if (h->next)
            h->next->prev = h;
        if (h->prev) {
            if (h->prev->list == h_old) {
                h->prev->list = h;
            } else {
                h->prev->next = h;
            }
        }
    }

    return ta_set_parent(TA_PTR_FROM_HDR(h), tactx);
}

void *ta_alloc_array(void *restrict tactx, size_t size, size_t count)
{
    if (__ta_unlikely(!size || count > TA_MAX_SIZE / size))
        abort();

    return ta_alloc(tactx, size * count);
}

void *ta_zalloc_array(void *restrict tactx, size_t size, size_t count)
{
    if (__ta_unlikely(!size || count > TA_MAX_SIZE / size))
        abort();

    return ta_zalloc(tactx, size * count);
}

void *ta_realloc_array(void *restrict tactx, void *restrict ptr, size_t size, size_t count)
{
    if (__ta_unlikely(!size || count > TA_MAX_SIZE / size))
        abort();

    return ta_realloc(tactx, ptr, size * count);
}

void *ta_assign(void *restrict tactx, void *restrict ptr, size_t size)
{
    if (!ptr)
        return ta_alloc(tactx, size);

    if (__ta_unlikely(size > TA_MAX_SIZE))
        abort();

    struct ta_header *h = realloc(ptr, TA_HDR_SIZE + size);
    if (__ta_unlikely(!h))
        abort();

    if (__ta_likely(size))
        memmove(TA_PTR_FROM_HDR(h), h, size);

    return ta_header_init(h, size, tactx);
}

void *ta_memdup(void *restrict tactx, const void *restrict ptr, size_t size)
{
    if (__ta_unlikely(!ptr))
        abort();

    if (__ta_unlikely(size > TA_MAX_SIZE))
        abort();

    struct ta_header *h = malloc(TA_HDR_SIZE + size);
    if (__ta_unlikely(!h))
        abort();

    if (__ta_likely(size))
        memcpy(TA_PTR_FROM_HDR(h), ptr, size);

    return ta_header_init(h, size, tactx);
}

void *ta_strdup(void *restrict tactx, const char *restrict str)
{
    if (__ta_unlikely(!str))
        abort();

    size_t size = strlen(str) + 1;
    if (__ta_unlikely(size > TA_MAX_SIZE))
        abort();

    struct ta_header *h = malloc(TA_HDR_SIZE + size);
    if (__ta_unlikely(!h))
        abort();

    memcpy(TA_PTR_FROM_HDR(h), str, size);
    return ta_header_init(h, size, tactx);
}

void *ta_strndup(void *restrict tactx, const char *restrict str, size_t n)
{
    if (__ta_unlikely(!str))
        abort();

    n = strnlen(str, n);
    if (__ta_unlikely(n >= TA_MAX_SIZE))
        abort();

    struct ta_header *h = malloc(TA_HDR_SIZE + n + 1);
    if (__ta_unlikely(!h))
        abort();

    char *ptr = TA_PTR_FROM_HDR(h);
    if (__ta_likely(n))
        memcpy(ptr, str, n);

    ptr[n] = '\0';
    return ta_header_init(h, n + 1, tactx);
}

void ta_free(void *ptr)
{
    if (__ta_likely(ptr)) {
        struct ta_header *h = ta_header_from_ptr(ptr);
        ta_header_free(h);
    }
}

void ta_free_children(void *ptr)
{
    struct ta_header *h = ta_header_from_ptr(ptr);
    while (h->list)
        ta_header_free(h->list);
}

void ta_set_destructor(void *restrict ptr, ta_destructor destructor)
{
    struct ta_header *h = ta_header_from_ptr(ptr);
    h->destructor = destructor;
}

ta_destructor ta_get_destructor(void *ptr)
{
    struct ta_header *h = ta_header_from_ptr(ptr);
    return h->destructor;
}

void *ta_set_parent(void *restrict ptr, void *restrict tactx)
{
    struct ta_header *h = ta_header_from_ptr(ptr);
    struct ta_header *h_parent = tactx ? ta_header_from_ptr(tactx) : NULL;

    if (h->prev) {
        if (h->prev->list == h) {
            if (h->prev == h_parent)
                return TA_PTR_FROM_HDR(h);
            h->prev->list = h->next;
        } else {
            h->prev->next = h->next;
        }
        if (h->next)
            h->next->prev = h->prev;
    }

    if (h_parent) {
        if (h_parent->list) {
            h->next = h_parent->list;
            h->next->prev = h;
        } else {
            h->next = NULL;
        }
        h->prev = h_parent;
        h->prev->list = h;
    } else {
        h->prev = h->next = NULL;
    }

    return TA_PTR_FROM_HDR(h);
}

void *ta_get_parent(void *ptr)
{
    struct ta_header *h = ta_header_from_ptr(ptr);
    while (h->prev && h->prev->list != h)
        h = h->prev;
    return h->prev ? TA_PTR_FROM_HDR(h->prev) : NULL;
}

size_t ta_get_size(void *ptr)
{
    struct ta_header *h = ta_header_from_ptr(ptr);
    return h->size;
}
