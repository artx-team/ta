#pragma once

/* TA ("Tree Allocator") is a hierarchical memory allocator with destructors.
 *
 * Essentially it is a wrapper around `malloc()` and related functions.
 * When a parent allocation is freed all child memory allocations are automatically freed too.
 *
 * Unlike standard malloc, allocation size may be zero, in which case there is an empty
 * allocation which can still be used as a parent for other allocations.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#ifndef __ta_has_attribute
#   ifdef __has_attribute
#       define __ta_has_attribute(x) __has_attribute(x)
#   else
#       define __ta_has_attribute(x) (0)
#   endif
#endif

#ifndef __ta_public
#   if defined(__GNUC__) || __ta_has_attribute(__visibility__)
#       define __ta_public __attribute__((__visibility__("default")))
#   else
#       define __ta_public
#   endif
#endif

#ifndef __ta_nodiscard
#   if defined(__GNUC__) || __ta_has_attribute(__warn_unused_result__)
#       define __ta_nodiscard __attribute__((__warn_unused_result__))
#   else
#       define __ta_nodiscard
#   endif
#endif

#ifndef __ta_nonnull
#   if defined(__GNUC__) || __ta_has_attribute(__returns_nonnull__)
#       define __ta_nonnull __attribute__((__returns_nonnull__))
#   else
#       define __ta_nonnull
#   endif
#endif

#define TA_FREE(ptr) do { if (ptr) { ta_free(ptr); (ptr) = NULL; } } while (0)
#define TA_RAII(type, name, init) RAII(type *, name, ta_free, init)

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

// Create a new TA chunk from malloc'ed ptr.
__ta_public __ta_nodiscard __ta_nonnull
void *ta_assign(void *restrict tactx, void *restrict ptr, size_t size);

// Create a new TA chunk from memory block.
__ta_public __ta_nodiscard __ta_nonnull
void *ta_memdup(void *restrict tactx, const void *restrict ptr, size_t size);

// Free a TA chunk.
__ta_public
void ta_free(void *ptr);

// Free children of a TA chunk.
__ta_public
void ta_free_children(void *ptr);

// Set the destructor function to be called when a TA chunk is freed.
__ta_public
void ta_set_destructor(void *restrict ptr, ta_destructor destructor);

// Get the destructor function of a TA chunk.
__ta_public __ta_nodiscard
ta_destructor ta_get_destructor(void *ptr);

// Set the parent to a TA chunk.
__ta_public __ta_nonnull
void *ta_set_parent(void *restrict ptr, void *restrict tactx);

// Get the parent of a TA chunk.
__ta_public __ta_nodiscard
void *ta_get_parent(void *ptr);

// Get the size of a TA chunk.
__ta_public __ta_nodiscard
size_t ta_get_size(void *ptr);

#ifdef __cplusplus
}
#endif
