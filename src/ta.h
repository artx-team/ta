#pragma once

/* TA (Tree Allocator) is a hierarchical memory allocator with destructors.
 *
 * Essentially it is a wrapper around malloc and related functions.
 * When a parent allocation is freed all child memory allocations are automatically freed too.
 *
 * Unlike standard malloc, allocation size may be zero, in which case there is an empty
 * allocation which can still be used as a parent for other allocations.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#ifndef __ta_has_builtin
#   ifdef __has_builtin
#       define __ta_has_builtin(x) __has_builtin(x)
#   else
#       define __ta_has_builtin(x) (0)
#   endif
#endif

#ifndef __ta_likely
#   if __ta_has_builtin(__builtin_expect)
#       define __ta_likely(x) (__builtin_expect(!!(x), 1))
#   else
#       define __ta_likely(x) (x)
#   endif
#endif

#ifndef __ta_unlikely
#   if __ta_has_builtin(__builtin_expect)
#       define __ta_unlikely(x) (__builtin_expect(!!(x), 0))
#   else
#       define __ta_unlikely(x) (x)
#   endif
#endif

#ifndef __ta_has_attribute
#   ifdef __has_attribute
#       define __ta_has_attribute(x) __has_attribute(x)
#   else
#       define __ta_has_attribute(x) (0)
#   endif
#endif

#ifndef __ta_public
#   if __ta_has_attribute(__visibility__)
#       define __ta_public __attribute__((__visibility__("default")))
#   else
#       define __ta_public
#   endif
#endif

#ifndef __ta_nodiscard
#   if __ta_has_attribute(__warn_unused_result__)
#       define __ta_nodiscard __attribute__((__warn_unused_result__))
#   else
#       define __ta_nodiscard
#   endif
#endif

#ifndef __ta_nonnull
#   if __ta_has_attribute(__returns_nonnull__)
#       define __ta_nonnull __attribute__((__returns_nonnull__))
#   else
#       define __ta_nonnull
#   endif
#endif

#ifndef __ta_printf
#   if __ta_has_attribute(__format__)
#       define __ta_printf(x, y) __attribute__((__format__(__printf__, x, y)))
#   else
#       define __ta_printf(x, y)
#   endif
#endif

#ifndef __ta_malloc
#   if __ta_has_attribute(__malloc__)
#       define __ta_malloc __attribute__((__malloc__))
#   else
#       define __ta_malloc
#   endif
#endif

#ifndef __ta_alloc_align
#   if __ta_has_attribute(__alloc_align__)
#       define __ta_alloc_align(...) __attribute__((__alloc_align__(__VA_ARGS__)))
#   else
#       define __ta_alloc_align(...)
#   endif
#endif

#ifndef __ta_alloc_size
#   if __ta_has_attribute(__alloc_size__)
#       define __ta_alloc_size(...) __attribute__((__alloc_size__(__VA_ARGS__)))
#   else
#       define __ta_alloc_size(...)
#   endif
#endif

#ifndef __ta_dealloc_free
#   if defined(__GNUC__) && __GNUC__ >= 11 && __ta_has_attribute(__malloc__)
#       define __ta_dealloc_free __attribute__((__malloc__(__builtin_free, 1)))
#   else
#       define __ta_dealloc_free
#   endif
#endif

// Wrapper around standard `malloc()` which aborts on errors.
__ta_public __ta_nodiscard __ta_nonnull
__ta_malloc __ta_alloc_size(1) __ta_dealloc_free
void *ta_xmalloc(size_t size);

// Wrapper around standard `calloc()` which aborts on errors.
__ta_public __ta_nodiscard __ta_nonnull
__ta_malloc __ta_alloc_size(1, 2) __ta_dealloc_free
void *ta_xcalloc(size_t n, size_t size);

// Wrapper around standard `realloc()` which aborts on errors.
__ta_public __ta_nodiscard __ta_nonnull
__ta_alloc_size(2) __ta_dealloc_free
void *ta_xrealloc(void *ptr, size_t size);

// Wrapper around standard `calloc()` which aborts on errors.
__ta_public __ta_nodiscard __ta_nonnull
__ta_malloc __ta_alloc_size(1) __ta_dealloc_free
void *ta_xzalloc(size_t size);

#ifndef _WIN32
// Wrapper around standard `posix_memalign()` which aborts on errors.
__ta_public __ta_nodiscard __ta_nonnull
__ta_malloc __ta_alloc_align(1) __ta_alloc_size(2) __ta_dealloc_free
void *ta_xmemalign(size_t alignment, size_t n);
#endif

// Wrapper around standard `strdup()` which aborts on errors.
__ta_public __ta_nodiscard __ta_nonnull
__ta_malloc __ta_dealloc_free
char *ta_xstrdup(const char *str);

// Wrapper around standard `strndup()` which aborts on errors.
__ta_public __ta_nodiscard __ta_nonnull
__ta_malloc __ta_dealloc_free
char *ta_xstrndup(const char *str, size_t n);

// Wrapper around `malloc()` and `memcpy()` which aborts on errors.
__ta_public __ta_nodiscard __ta_nonnull
__ta_malloc __ta_dealloc_free
void *ta_xmemdup(const void *mem, size_t n);

// Wrapper around `asprintf()` which aborts on errors.
__ta_public __ta_nodiscard __ta_nonnull
__ta_malloc __ta_dealloc_free __ta_printf(1, 2)
char *ta_xasprintf(const char *format, ...);

// Wrapper around `vasprintf()` which aborts on errors.
__ta_public __ta_nodiscard __ta_nonnull
__ta_malloc __ta_dealloc_free __ta_printf(1, 0)
char *ta_xvasprintf(const char *format, va_list ap);

// Wrapper around standard `free()` which sets the pointer to NULL.
#define ta_xfree(ptr) do { if (__ta_likely(ptr)) { free(ptr); (ptr) = NULL; } } while (0)

// Wrapper around `ta_free()` which sets the pointer to NULL.
#define TA_FREE(ptr) do { if (__ta_likely(ptr)) { ta_free(ptr); (ptr) = NULL; } } while (0)

// Prototype of destructor which is called when TA chunk is freed.
typedef void (*ta_destructor)(void *);

// Create a new TA chunk.
__ta_public __ta_nodiscard __ta_nonnull
void *ta_alloc(void *tactx, size_t size);

// Create a new 0-initizialized TA chunk.
__ta_public __ta_nodiscard __ta_nonnull
void *ta_zalloc(void *tactx, size_t size);

// Change the size of a TA chunk.
__ta_public __ta_nodiscard __ta_nonnull
void *ta_realloc(void *restrict tactx, void *restrict ptr, size_t size);

// Create a new TA array.
__ta_public __ta_nodiscard __ta_nonnull
void *ta_alloc_array(void *restrict tactx, size_t size, size_t count);

// Create a new 0-initizialized TA array.
__ta_public __ta_nodiscard __ta_nonnull
void *ta_zalloc_array(void *restrict tactx, size_t size, size_t count);

// Change the size of a TA array.
__ta_public __ta_nodiscard __ta_nonnull
void *ta_realloc_array(void *restrict tactx, void *restrict ptr, size_t size, size_t count);

// Create a new TA chunk from a malloc'ed ptr.
__ta_public __ta_nodiscard __ta_nonnull
void *ta_assign(void *restrict tactx, void *restrict ptr, size_t size);

// Create a new TA chunk from a memory block.
__ta_public __ta_nodiscard __ta_nonnull
void *ta_memdup(void *restrict tactx, const void *restrict ptr, size_t size);

// Create a new TA chunk from a string. The function is similar to `strdup()`.
__ta_public __ta_nodiscard __ta_nonnull
char *ta_strdup(void *restrict tactx, const char *restrict str);

// Append a string to a given TA string.
__ta_public __ta_nodiscard __ta_nonnull
char *ta_strdup_append(char *restrict str, const char *restrict append);

// Append a string to a given TA buffer.
__ta_public __ta_nodiscard __ta_nonnull
char *ta_strdup_append_buffer(char *restrict str, const char *restrict append);

// Create a new TA chunk from a length-limited string. The function is similar to `strndup()`.
__ta_public __ta_nodiscard __ta_nonnull
char *ta_strndup(void *restrict tactx, const char *restrict str, size_t n);

// Append a length-limited string to a given TA string.
__ta_public __ta_nodiscard __ta_nonnull
char *ta_strndup_append(char *restrict str, const char *restrict append, size_t n);

// Append a length-limited string to a given TA buffer.
__ta_public __ta_nodiscard __ta_nonnull
char *ta_strndup_append_buffer(char *restrict str, const char *restrict append, size_t n);

// Create a new TA chunk from a formatted string. The function is similar to `asprintf()`.
__ta_public __ta_nodiscard __ta_nonnull __ta_printf(2, 3)
char *ta_asprintf(void *restrict tactx, const char *restrict format, ...);

// Append a formatted string to a given TA string.
__ta_public __ta_nodiscard __ta_nonnull __ta_printf(2, 3)
char *ta_asprintf_append(char *restrict str, const char *restrict format, ...);

// Append a formatted string to a given TA buffer.
__ta_public __ta_nodiscard __ta_nonnull __ta_printf(2, 3)
char *ta_asprintf_append_buffer(char *restrict str, const char *restrict format, ...);

// Create a new TA chunk from a formatted string. The function is similar to `vasprintf()`.
__ta_public __ta_nodiscard __ta_nonnull __ta_printf(2, 0)
char *ta_vasprintf(void *restrict tactx, const char *restrict format, va_list ap);

// Append a formatted string to a given TA string.
__ta_public __ta_nodiscard __ta_nonnull __ta_printf(2, 0)
char *ta_vasprintf_append(char *restrict str, const char *restrict format, va_list ap);

// Append a formatted string to a given TA buffer.
__ta_public __ta_nodiscard __ta_nonnull __ta_printf(2, 0)
char *ta_vasprintf_append_buffer(char *restrict str, const char *restrict format, va_list ap);

// Free a TA chunk.
__ta_public
void ta_free(void *ptr);

// Free children of a TA chunk.
__ta_public
void ta_free_children(void *ptr);

// Move children from one TA chunk to another.
__ta_public
void ta_move_children(void *restrict src, void *restrict dst);

// Set the destructor function to be called when a TA chunk is freed.
__ta_public
ta_destructor ta_set_destructor(void *restrict ptr, ta_destructor destructor);

// Get the destructor function of a TA chunk.
__ta_public __ta_nodiscard
ta_destructor ta_get_destructor(void *ptr);

// Set the parent to a TA chunk.
__ta_public __ta_nonnull
void *ta_set_parent(void *restrict ptr, void *restrict tactx);

// Get the parent of a TA chunk.
__ta_public __ta_nodiscard
void *ta_get_parent(void *ptr);

// Recursively check if a TA chunk has the parent.
__ta_public __ta_nodiscard
bool ta_has_parent(void *ptr, void *tactx);

// Recursively check if a TA chunk has the child.
__ta_public __ta_nodiscard
bool ta_has_child(void *tactx, void *ptr);

// Get the first child of a TA chunk.
__ta_public __ta_nodiscard
void *ta_get_child(void *ptr);

// Get the next TA chunk which has the same parent.
__ta_public __ta_nodiscard
void *ta_get_next(void *ptr);

// Get the previous TA chunk which has the same parent.
__ta_public __ta_nodiscard
void *ta_get_prev(void *ptr);

// Get the size of a TA chunk.
__ta_public __ta_nodiscard
size_t ta_get_size(void *ptr);

// Forward traversal of all children of a TA chunk.
#define TA_FOREACH(ptr, tactx) \
    for ((ptr) = ta_get_child(tactx); \
         (ptr); \
         (ptr) = ta_get_next(ptr))

// Forward traversal of all children of a TA chunk optionally starting from `ptr`.
#define TA_FOREACH_FROM(ptr, tactx) \
    for ((ptr) = ((ptr) ? (ptr) : ta_get_child(tactx)); \
         (ptr); \
         (ptr) = ta_get_next(ptr))

// Forward traversal of all children of a TA chunk (safe version).
#define TA_FOREACH_SAFE(ptr, tactx, tmp) \
    for ((ptr) = ta_get_child(tactx); \
         (ptr) && ((tmp) = ta_get_next(ptr), 1); \
         (ptr) = (tmp))

// Forward traversal of all children of a TA chunk (safe version) optionally starting from `ptr`.
#define TA_FOREACH_FROM_SAFE(ptr, tactx, tmp) \
    for ((ptr) = ((ptr) ? (ptr) : ta_get_child(tactx)); \
         (ptr) && ((tmp) = ta_get_next(ptr), 1); \
         (ptr) = (tmp))

// Reverse traversal of all children of a TA chunk starting from `ptr`.
#define TA_FOREACH_REVERSE_FROM(ptr, tactx) \
    for (; \
         (ptr); \
         (ptr) = ta_get_prev(ptr))

// Reverse traversal of all children of a TA chunk (safe version) starting from `ptr`.
#define TA_FOREACH_REVERSE_FROM_SAFE(ptr, tactx, tmp) \
    for (; \
         (ptr) && ((tmp) = ta_get_prev(ptr), 1); \
         (ptr) = (tmp))

#ifdef __cplusplus
}
#endif
