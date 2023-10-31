#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "ta.h"

#ifndef __ta_inline
#   if __ta_has_attribute(__always_inline__)
#       define __ta_inline inline __attribute__((__always_inline__))
#   else
#       define __ta_inline inline
#   endif
#endif

#ifndef TA_MAGIC
#   if defined(__OPTIMIZE__) || defined(NDEBUG)
#       define TA_MAGIC 0
#   else
#       define TA_MAGIC 0x8FBEA918UL
#   endif
#endif

struct ta_header {
#if TA_MAGIC
    uintptr_t magic;
#endif
    struct ta_header *list;
    struct ta_header *prev;
    struct ta_header *next;
    size_t size;
    ta_destructor destructor;
};

#define TA_HDR_SIZE sizeof(struct ta_header)
#define TA_MAX_SIZE ((size_t)PTRDIFF_MAX - TA_HDR_SIZE)

#define TA_HDR_FROM_PTR(ptr) ((struct ta_header *)((uint8_t *)(ptr) - TA_HDR_SIZE))
#define TA_PTR_FROM_HDR(hdr) ((void *)((uint8_t *)(hdr) + TA_HDR_SIZE))

static __ta_inline __ta_nodiscard __ta_returns_nonnull
struct ta_header *ta_header_from_ptr(const void *ptr)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely((uintptr_t)ptr <= TA_HDR_SIZE))
        abort();
    // GCOVR_EXCL_STOP

    struct ta_header *h = TA_HDR_FROM_PTR(ptr);

#if TA_MAGIC
    // GCOVR_EXCL_START
    if (__ta_unlikely(h->magic != TA_MAGIC))
        abort();
    // GCOVR_EXCL_STOP
#endif

    return h;
}

static __ta_inline __ta_nodiscard __ta_returns_nonnull
void *ta_header_init(struct ta_header *restrict h, size_t size, void *restrict tactx)
{
    *h = (struct ta_header) {
#if TA_MAGIC
        .magic  = TA_MAGIC,
#endif
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
        ta_header_free(h->list); // NOLINT(clang-analyzer-unix.Malloc)

    if (h->prev) {
        if (h->prev->list == h) {
            h->prev->list = h->next;
        } else {
            h->prev->next = h->next;
        }
        if (h->next)
            h->next->prev = h->prev;
    }

#if TA_MAGIC
    h->magic = 0;
#endif
    free(h);
}

static __ta_inline __ta_nodiscard __ta_returns_nonnull
void *ta_header_realloc(struct ta_header *h, size_t size)
{
    struct ta_header *h_old = h;

    h = (struct ta_header *)realloc(h, TA_HDR_SIZE + size);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!h))
        abort();
    // GCOVR_EXCL_STOP

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

    return TA_PTR_FROM_HDR(h);
}

static __ta_inline __ta_nodiscard __ta_returns_nonnull
char *ta_header_append(struct ta_header *restrict h, size_t at,
                       const char *restrict append, size_t len)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(h->size < at))
        abort();

    if (__ta_unlikely(len >= TA_MAX_SIZE || at >= TA_MAX_SIZE - len))
        abort();
    // GCOVR_EXCL_STOP

    char *str = h->size <= at + len
                ? (char *)ta_header_realloc(h, at + len + 1)
                : (char *)TA_PTR_FROM_HDR(h);

    if (__ta_likely(len))
        memcpy(str + at, append, len);

    str[at + len] = '\0';
    return str;
}

static __ta_nodiscard __ta_returns_nonnull __ta_printf(3, 0)
char *ta_header_printf(struct ta_header *restrict h, size_t at,
                       const char *restrict format, va_list ap)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(h->size < at))
        abort();
    // GCOVR_EXCL_STOP

    va_list copy;
    va_copy(copy, ap);
    int len = vsnprintf(NULL, 0, format, copy);
    va_end(copy);

    // GCOVR_EXCL_START
    if (__ta_unlikely(len < 0))
        abort();

    if (__ta_unlikely((size_t)len >= TA_MAX_SIZE || at >= TA_MAX_SIZE - (size_t)len))
        abort();
    // GCOVR_EXCL_STOP

    char *str = h->size <= at + (size_t)len
                ? (char *)ta_header_realloc(h, at + (size_t)len + 1)
                : (char *)TA_PTR_FROM_HDR(h);

    int res = vsnprintf(str + at, (size_t)len + 1, format, ap);

    // GCOVR_EXCL_START
    if (__ta_unlikely(res != len))
        abort();
    // GCOVR_EXCL_STOP

    return str;
}

static __ta_inline
void ta_header_set_parent(struct ta_header *restrict h,
                          struct ta_header *restrict h_parent)
{
    if (h->prev) {
        if (h->prev->list == h) {
            if (h->prev == h_parent)
                return;
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
}

void *ta_xmalloc(size_t size)
{
    if (__ta_unlikely(!size))
        size = 1;

    void *ptr = malloc(size);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!ptr))
        abort();
    // GCOVR_EXCL_STOP

    return ptr;
}

void *ta_xcalloc(size_t n, size_t size)
{
    if (__ta_unlikely(!n || !size))
        n = size = 1;

    void *ptr = calloc(n, size);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!ptr))
        abort();
    // GCOVR_EXCL_STOP

    return ptr;
}

void *ta_xrealloc(void *ptr, size_t size)
{
    if (__ta_unlikely(!size))
        size = 1;

    ptr = ptr ? realloc(ptr, size) : malloc(size);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!ptr))
        abort();
    // GCOVR_EXCL_STOP

    return ptr;
}

void *ta_xzalloc(size_t size)
{
    if (__ta_unlikely(!size))
        size = 1;

    void *ptr = calloc(1, size);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!ptr))
        abort();
    // GCOVR_EXCL_STOP

    return ptr;
}

#ifndef _WIN32
void *ta_xmemalign(size_t alignment, size_t n)
{
    void *ptr = NULL;
    int rc = posix_memalign(&ptr, alignment, n);

    // GCOVR_EXCL_START
    if (__ta_unlikely(rc != 0))
        abort();
    // GCOVR_EXCL_STOP

    return ptr;
}
#endif

char *ta_xstrdup(const char *str)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(!str))
        abort();
    // GCOVR_EXCL_STOP

    size_t n = strlen(str) + 1;
    void *ptr = malloc(n);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!ptr))
        abort();
    // GCOVR_EXCL_STOP

    return (char *)memcpy(ptr, str, n);
}

char *ta_xstrndup(const char *str, size_t n)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(!str))
        abort();
    // GCOVR_EXCL_STOP

    n = strnlen(str, n);
    char *ptr = (char *)malloc(n + 1);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!ptr))
        abort();
    // GCOVR_EXCL_STOP

    if (__ta_likely(n))
        memcpy(ptr, str, n);

    ptr[n] = '\0';
    return ptr;
}

void *ta_xmemdup(const void *mem, size_t n)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(!mem))
        abort();
    // GCOVR_EXCL_STOP

    if (__ta_unlikely(!n)) {
        void *ptr = malloc(1);

        // GCOVR_EXCL_START
        if (__ta_unlikely(!ptr))
            abort();
        // GCOVR_EXCL_STOP

        return ptr;
    }

    void *ptr = malloc(n);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!ptr))
        abort();
    // GCOVR_EXCL_STOP

    return memcpy(ptr, mem, n);
}

char *ta_xasprintf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *str = ta_xvasprintf(format, ap);
    va_end(ap);
    return str;
}

char *ta_xvasprintf(const char *format, va_list ap)
{
    char *str;
    int len = vasprintf(&str, format, ap);

    // GCOVR_EXCL_START
    if (__ta_unlikely(len < 0))
        abort();
    // GCOVR_EXCL_STOP

    return str;
}

void *ta_alloc(void *tactx, size_t size)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(size > TA_MAX_SIZE))
        abort();
    // GCOVR_EXCL_STOP

    struct ta_header *h = (struct ta_header *)malloc(TA_HDR_SIZE + size);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!h))
        abort();
    // GCOVR_EXCL_STOP

    return ta_header_init(h, size, tactx);
}

void *ta_zalloc(void *tactx, size_t size)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(size > TA_MAX_SIZE))
        abort();
    // GCOVR_EXCL_STOP

    struct ta_header *h = (struct ta_header *)calloc(1, TA_HDR_SIZE + size);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!h))
        abort();
    // GCOVR_EXCL_STOP

    return ta_header_init(h, size, tactx);
}

void *ta_realloc(void *restrict tactx, void *restrict ptr, size_t size)
{
    if (!ptr)
        return ta_alloc(tactx, size);

    // GCOVR_EXCL_START
    if (__ta_unlikely(size > TA_MAX_SIZE))
        abort();
    // GCOVR_EXCL_STOP

    struct ta_header *h = ta_header_from_ptr(ptr);
    ptr = ta_header_realloc(h, size);
    return ta_set_parent(ptr, tactx);
}

void *ta_alloc_array(void *restrict tactx, size_t size, size_t count)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(!size || count > TA_MAX_SIZE / size))
        abort();
    // GCOVR_EXCL_STOP

    return ta_alloc(tactx, size * count);
}

void *ta_zalloc_array(void *restrict tactx, size_t size, size_t count)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(!size || count > TA_MAX_SIZE / size))
        abort();
    // GCOVR_EXCL_STOP

    return ta_zalloc(tactx, size * count);
}

void *ta_realloc_array(void *restrict tactx, void *restrict ptr, size_t size, size_t count)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(!size || count > TA_MAX_SIZE / size))
        abort();
    // GCOVR_EXCL_STOP

    return ta_realloc(tactx, ptr, size * count);
}

void *ta_assign(void *restrict tactx, void *restrict ptr, size_t size)
{
    if (!ptr)
        return ta_alloc(tactx, size);

    // GCOVR_EXCL_START
    if (__ta_unlikely(size > TA_MAX_SIZE))
        abort();
    // GCOVR_EXCL_STOP

    struct ta_header *h = (struct ta_header *)realloc(ptr, TA_HDR_SIZE + size);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!h))
        abort();
    // GCOVR_EXCL_STOP

    if (__ta_likely(size))
        memmove(TA_PTR_FROM_HDR(h), h, size);

    return ta_header_init(h, size, tactx);
}

void *ta_memdup(void *restrict tactx, const void *restrict ptr, size_t size)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(!ptr))
        abort();

    if (__ta_unlikely(size > TA_MAX_SIZE))
        abort();
    // GCOVR_EXCL_STOP

    struct ta_header *h = (struct ta_header *)malloc(TA_HDR_SIZE + size);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!h))
        abort();
    // GCOVR_EXCL_STOP

    if (__ta_likely(size))
        memcpy(TA_PTR_FROM_HDR(h), ptr, size);

    return ta_header_init(h, size, tactx);
}

char *ta_strdup(void *restrict tactx, const char *restrict str)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(!str))
        abort();
    // GCOVR_EXCL_STOP

    size_t n = strlen(str) + 1;

    // GCOVR_EXCL_START
    if (__ta_unlikely(n > TA_MAX_SIZE))
        abort();
    // GCOVR_EXCL_STOP

    struct ta_header *h = (struct ta_header *)malloc(TA_HDR_SIZE + n);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!h))
        abort();
    // GCOVR_EXCL_STOP

    memcpy(TA_PTR_FROM_HDR(h), str, n);
    return (char *)ta_header_init(h, n, tactx);
}

char *ta_strdup_append(char *restrict str, const char *restrict append)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(!str || !append))
        abort();
    // GCOVR_EXCL_STOP

    struct ta_header *h = ta_header_from_ptr(str);
    return (char *)ta_header_append(h, strnlen(str, h->size), append, strlen(append));
}

char *ta_strdup_append_buffer(char *restrict str, const char *restrict append)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(!str || !append))
        abort();
    // GCOVR_EXCL_STOP

    struct ta_header *h = ta_header_from_ptr(str);
    return (char *)ta_header_append(h, h->size ? h->size - 1 : 0, append, strlen(append));
}

char *ta_strndup(void *restrict tactx, const char *restrict str, size_t n)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(!str))
        abort();
    // GCOVR_EXCL_STOP

    n = strnlen(str, n);

    // GCOVR_EXCL_START
    if (__ta_unlikely(n >= TA_MAX_SIZE))
        abort();
    // GCOVR_EXCL_STOP

    struct ta_header *h = (struct ta_header *)malloc(TA_HDR_SIZE + n + 1);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!h))
        abort();
    // GCOVR_EXCL_STOP

    char *ptr = (char *)TA_PTR_FROM_HDR(h);
    if (__ta_likely(n))
        memcpy(ptr, str, n);

    ptr[n] = '\0';
    return (char *)ta_header_init(h, n + 1, tactx);
}

char *ta_strndup_append(char *restrict str, const char *restrict append, size_t n)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(!str || !append))
        abort();
    // GCOVR_EXCL_STOP

    struct ta_header *h = ta_header_from_ptr(str);
    return (char *)ta_header_append(h, strnlen(str, h->size), append, strnlen(append, n));
}

char *ta_strndup_append_buffer(char *restrict str, const char *restrict append, size_t n)
{
    // GCOVR_EXCL_START
    if (__ta_unlikely(!str || !append))
        abort();
    // GCOVR_EXCL_STOP

    struct ta_header *h = ta_header_from_ptr(str);
    return (char *)ta_header_append(h, h->size ? h->size - 1 : 0, append, strnlen(append, n));
}

char *ta_asprintf(void *restrict tactx, const char *restrict format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *str = ta_vasprintf(tactx, format, ap);
    va_end(ap);
    return str;
}

char *ta_asprintf_append(char *restrict str, const char *restrict format, ...)
{
    va_list ap;
    va_start(ap, format);
    str = ta_vasprintf_append(str, format, ap);
    va_end(ap);
    return str;
}

char *ta_asprintf_append_buffer(char *restrict str, const char *restrict format, ...)
{
    va_list ap;
    va_start(ap, format);
    str = ta_vasprintf_append_buffer(str, format, ap);
    va_end(ap);
    return str;
}

char *ta_vasprintf(void *restrict tactx, const char *restrict format, va_list ap)
{
    va_list copy;
    va_copy(copy, ap);
    int len = vsnprintf(NULL, 0, format, copy);
    va_end(copy);

    // GCOVR_EXCL_START
    if (__ta_unlikely(len < 0 || (size_t)len >= TA_MAX_SIZE))
        abort();
    // GCOVR_EXCL_STOP

    struct ta_header *h = (struct ta_header *)malloc(TA_HDR_SIZE + (size_t)len + 1);

    // GCOVR_EXCL_START
    if (__ta_unlikely(!h))
        abort();
    // GCOVR_EXCL_STOP

    char *str = (char *)TA_PTR_FROM_HDR(h);
    int res = vsnprintf(str, (size_t)len + 1, format, ap);

    // GCOVR_EXCL_START
    if (__ta_unlikely(res != len))
        abort();
    // GCOVR_EXCL_STOP

    return (char *)ta_header_init(h, (size_t)len + 1, tactx);
}

char *ta_vasprintf_append(char *restrict str, const char *restrict format, va_list ap)
{
    struct ta_header *h = ta_header_from_ptr(str);
    return ta_header_printf(h, strnlen(str, h->size), format, ap);
}

char *ta_vasprintf_append_buffer(char *restrict str, const char *restrict format, va_list ap)
{
    struct ta_header *h = ta_header_from_ptr(str);
    return ta_header_printf(h, h->size ? h->size - 1 : 0, format, ap);
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
        ta_header_free(h->list); // NOLINT(clang-analyzer-unix.Malloc)
}

void ta_move_children(void *restrict src, void *restrict dst)
{
    struct ta_header *h_src = ta_header_from_ptr(src);
    struct ta_header *h_dst = dst ? ta_header_from_ptr(dst) : NULL;

    if (!h_src->list)
        return;

    if (!h_dst) {
        do {
            ta_header_set_parent(h_src->list, NULL);
        } while (h_src->list);
        return;
    }

    if (!h_dst->list) {
        h_dst->list = h_src->list;
        h_dst->list->prev = h_dst;
        h_src->list = NULL;
        return;
    }

    struct ta_header *h = h_dst->list;
    while (h->next)
        h = h->next;

    h->next = h_src->list;
    h->next->prev = h;
    h_src->list = NULL;
}

ta_destructor ta_set_destructor(void *restrict ptr, ta_destructor destructor)
{
    struct ta_header *h = ta_header_from_ptr(ptr);
    ta_destructor prev_destructor = h->destructor;
    h->destructor = destructor;
    return prev_destructor;
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
    ta_header_set_parent(h, h_parent);
    return ptr;
}

void *ta_get_parent(void *ptr)
{
    struct ta_header *h = ta_header_from_ptr(ptr);
    if (!h->prev)
        return NULL;
    while (h->prev->list != h)
        h = h->prev;
    return TA_PTR_FROM_HDR(h->prev);
}

static __ta_inline __ta_nodiscard
bool ta_lookup_parent(struct ta_header *h, struct ta_header *h_parent)
{
    do {
        if (!h->prev)
            return false;
        while (h->prev->list != h)
            h = h->prev;
        if (h->prev == h_parent)
            return true;
        h = h->prev;
    } while (true);
}

bool ta_has_parent(void *ptr, void *tactx)
{
    struct ta_header *h = ta_header_from_ptr(ptr);

    if (__ta_unlikely(!tactx || ptr == tactx))
        return false;

    struct ta_header *h_parent = ta_header_from_ptr(tactx);
    return ta_lookup_parent(h, h_parent);
}

bool ta_has_child(void *tactx, void *ptr)
{
    struct ta_header *h_parent = ta_header_from_ptr(tactx);

    if (__ta_unlikely(!ptr || ptr == tactx))
        return false;

    struct ta_header *h = ta_header_from_ptr(ptr);
    return ta_lookup_parent(h, h_parent);
}

void *ta_get_child(void *ptr)
{
    struct ta_header *h = ta_header_from_ptr(ptr);
    return h->list ? TA_PTR_FROM_HDR(h->list) : NULL;
}

void *ta_get_next(void *ptr)
{
    struct ta_header *h = ta_header_from_ptr(ptr);
    return h->next ? TA_PTR_FROM_HDR(h->next) : NULL;
}

void *ta_get_prev(void *ptr)
{
    struct ta_header *h = ta_header_from_ptr(ptr);
    return h->prev && h->prev->list != h ? TA_PTR_FROM_HDR(h->prev) : NULL;
}

size_t ta_get_size(void *ptr)
{
    struct ta_header *h = ta_header_from_ptr(ptr);
    return h->size;
}
