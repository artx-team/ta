#include <stdlib.h>
#include <stdint.h>
#include <stdalign.h>
#include <string.h>

#include "ta.h"

#ifndef __ta_has_builtin
#   ifdef __has_builtin
#       define __ta_has_builtin(x) __has_builtin(x)
#   else
#       define __ta_has_builtin(x) (0)
#   endif
#endif

#ifndef __ta_likely
#   if defined(__GNUC__) || __ta_has_builtin(__builtin_expect)
#       define __ta_likely(x) (__builtin_expect(!!(x), 1))
#   else
#       define __ta_likely(x) (x)
#   endif
#endif

#ifndef __ta_unlikely
#   if defined(__GNUC__) || __ta_has_builtin(__builtin_expect)
#       define __ta_unlikely(x) (__builtin_expect(!!(x), 0))
#   else
#       define __ta_unlikely(x) (x)
#   endif
#endif

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
