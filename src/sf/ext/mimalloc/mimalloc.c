#if defined(SF_USE_MIMALLOC) && SF_USE_MIMALLOC

/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#if defined(__sun)
// same remarks as os.c for the static's context.
#undef _XOPEN_SOURCE
#undef _POSIX_C_SOURCE
#endif

/**** start inlining mimalloc.h ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018-2020, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#ifndef MIMALLOC_H
#define MIMALLOC_H

#define MI_MALLOC_VERSION 164   // major + 2 digits minor

// ------------------------------------------------------
// Compiler specific attributes
// ------------------------------------------------------

#ifdef __cplusplus
  #if (__cplusplus >= 201103L) || (_MSC_VER > 1900)  // C++11
    #define mi_attr_noexcept   noexcept
  #else
    #define mi_attr_noexcept   throw()
  #endif
#else
  #define mi_attr_noexcept
#endif

#if (__cplusplus >= 201703)
  #define mi_decl_nodiscard    [[nodiscard]]
#elif (__GNUC__ >= 4) || defined(__clang__)  // includes clang, icc, and clang-cl
  #define mi_decl_nodiscard    __attribute__((warn_unused_result))
#elif (_MSC_VER >= 1700)
  #define mi_decl_nodiscard    _Check_return_
#else
  #define mi_decl_nodiscard
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
  #if !defined(MI_SHARED_LIB)
    #define mi_decl_export
  #elif defined(MI_SHARED_LIB_EXPORT)
    #define mi_decl_export              __declspec(dllexport)
  #else
    #define mi_decl_export              __declspec(dllimport)
  #endif
  #if defined(__MINGW32__)
    #define mi_decl_restrict
    #define mi_attr_malloc              __attribute__((malloc))
  #else
    #if (_MSC_VER >= 1900) && !defined(__EDG__)
      #define mi_decl_restrict          __declspec(allocator) __declspec(restrict)
    #else
      #define mi_decl_restrict          __declspec(restrict)
    #endif
    #define mi_attr_malloc
  #endif
  #define mi_cdecl                      __cdecl
  #define mi_attr_alloc_size(s)
  #define mi_attr_alloc_size2(s1,s2)
  #define mi_attr_alloc_align(p)
#elif defined(__GNUC__)                 // includes clang and icc
  #define mi_cdecl                      // leads to warnings... __attribute__((cdecl))
  #define mi_decl_export                __attribute__((visibility("default")))
  #define mi_decl_restrict
  #define mi_attr_malloc                __attribute__((malloc))
  #if (defined(__clang_major__) && (__clang_major__ < 4)) || (__GNUC__ < 5)
    #define mi_attr_alloc_size(s)
    #define mi_attr_alloc_size2(s1,s2)
    #define mi_attr_alloc_align(p)
  #elif defined(__INTEL_COMPILER)
    #define mi_attr_alloc_size(s)       __attribute__((alloc_size(s)))
    #define mi_attr_alloc_size2(s1,s2)  __attribute__((alloc_size(s1,s2)))
    #define mi_attr_alloc_align(p)
  #else
    #define mi_attr_alloc_size(s)       __attribute__((alloc_size(s)))
    #define mi_attr_alloc_size2(s1,s2)  __attribute__((alloc_size(s1,s2)))
    #define mi_attr_alloc_align(p)      __attribute__((alloc_align(p)))
  #endif
#else
  #define mi_cdecl
  #define mi_decl_export
  #define mi_decl_restrict
  #define mi_attr_malloc
  #define mi_attr_alloc_size(s)
  #define mi_attr_alloc_size2(s1,s2)
  #define mi_attr_alloc_align(p)
#endif

// ------------------------------------------------------
// Includes
// ------------------------------------------------------

#include <stddef.h>     // size_t
#include <stdbool.h>    // bool

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------------------------------------
// Standard malloc interface
// ------------------------------------------------------

mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_malloc(size_t size)  mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(1);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_calloc(size_t count, size_t size)  mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size2(1,2);
mi_decl_nodiscard mi_decl_export void* mi_realloc(void* p, size_t newsize)      mi_attr_noexcept mi_attr_alloc_size(2);
mi_decl_export void* mi_expand(void* p, size_t newsize)                         mi_attr_noexcept mi_attr_alloc_size(2);

mi_decl_export void mi_free(void* p) mi_attr_noexcept;
mi_decl_nodiscard mi_decl_export mi_decl_restrict char* mi_strdup(const char* s) mi_attr_noexcept mi_attr_malloc;
mi_decl_nodiscard mi_decl_export mi_decl_restrict char* mi_strndup(const char* s, size_t n) mi_attr_noexcept mi_attr_malloc;
mi_decl_nodiscard mi_decl_export mi_decl_restrict char* mi_realpath(const char* fname, char* resolved_name) mi_attr_noexcept mi_attr_malloc;

// ------------------------------------------------------
// Extended functionality
// ------------------------------------------------------
#define MI_SMALL_WSIZE_MAX  (128)
#define MI_SMALL_SIZE_MAX   (MI_SMALL_WSIZE_MAX*sizeof(void*))

mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_malloc_small(size_t size) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(1);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_zalloc_small(size_t size) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(1);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_zalloc(size_t size)       mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(1);

mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_mallocn(size_t count, size_t size) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size2(1,2);
mi_decl_nodiscard mi_decl_export void* mi_reallocn(void* p, size_t count, size_t size)        mi_attr_noexcept mi_attr_alloc_size2(2,3);
mi_decl_nodiscard mi_decl_export void* mi_reallocf(void* p, size_t newsize)                   mi_attr_noexcept mi_attr_alloc_size(2);

mi_decl_nodiscard mi_decl_export size_t mi_usable_size(const void* p) mi_attr_noexcept;
mi_decl_nodiscard mi_decl_export size_t mi_good_size(size_t size)     mi_attr_noexcept;


// ------------------------------------------------------
// Internals
// ------------------------------------------------------

typedef void (mi_cdecl mi_deferred_free_fun)(bool force, unsigned long long heartbeat, void* arg);
mi_decl_export void mi_register_deferred_free(mi_deferred_free_fun* deferred_free, void* arg) mi_attr_noexcept;

typedef void (mi_cdecl mi_output_fun)(const char* msg, void* arg);
mi_decl_export void mi_register_output(mi_output_fun* out, void* arg) mi_attr_noexcept;

typedef void (mi_cdecl mi_error_fun)(int err, void* arg);
mi_decl_export void mi_register_error(mi_error_fun* fun, void* arg);

mi_decl_export void mi_collect(bool force)    mi_attr_noexcept;
mi_decl_export int  mi_version(void)          mi_attr_noexcept;
mi_decl_export void mi_stats_reset(void)      mi_attr_noexcept;
mi_decl_export void mi_stats_merge(void)      mi_attr_noexcept;
mi_decl_export void mi_stats_print(void* out) mi_attr_noexcept;  // backward compatibility: `out` is ignored and should be NULL
mi_decl_export void mi_stats_print_out(mi_output_fun* out, void* arg) mi_attr_noexcept;

mi_decl_export void mi_process_init(void)     mi_attr_noexcept;
mi_decl_export void mi_thread_init(void)      mi_attr_noexcept;
mi_decl_export void mi_thread_done(void)      mi_attr_noexcept;
mi_decl_export void mi_thread_stats_print_out(mi_output_fun* out, void* arg) mi_attr_noexcept;


// -------------------------------------------------------------------------------------
// Aligned allocation
// Note that `alignment` always follows `size` for consistency with unaligned
// allocation, but unfortunately this differs from `posix_memalign` and `aligned_alloc`.
// -------------------------------------------------------------------------------------

mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_malloc_aligned(size_t size, size_t alignment) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(1) mi_attr_alloc_align(2);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_malloc_aligned_at(size_t size, size_t alignment, size_t offset) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(1);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_zalloc_aligned(size_t size, size_t alignment) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(1) mi_attr_alloc_align(2);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_zalloc_aligned_at(size_t size, size_t alignment, size_t offset) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(1);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_calloc_aligned(size_t count, size_t size, size_t alignment) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size2(1,2) mi_attr_alloc_align(3);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_calloc_aligned_at(size_t count, size_t size, size_t alignment, size_t offset) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size2(1,2);
mi_decl_nodiscard mi_decl_export void* mi_realloc_aligned(void* p, size_t newsize, size_t alignment) mi_attr_noexcept mi_attr_alloc_size(2) mi_attr_alloc_align(3);
mi_decl_nodiscard mi_decl_export void* mi_realloc_aligned_at(void* p, size_t newsize, size_t alignment, size_t offset) mi_attr_noexcept mi_attr_alloc_size(2);


// -------------------------------------------------------------------------------------
// Heaps: first-class, but can only allocate from the same thread that created it.
// -------------------------------------------------------------------------------------

struct mi_heap_s;
typedef struct mi_heap_s mi_heap_t;

mi_decl_nodiscard mi_decl_export mi_heap_t* mi_heap_new(void);
mi_decl_export void       mi_heap_delete(mi_heap_t* heap);
mi_decl_export void       mi_heap_destroy(mi_heap_t* heap);
mi_decl_export mi_heap_t* mi_heap_set_default(mi_heap_t* heap);
mi_decl_export mi_heap_t* mi_heap_get_default(void);
mi_decl_export mi_heap_t* mi_heap_get_backing(void);
mi_decl_export void       mi_heap_collect(mi_heap_t* heap, bool force) mi_attr_noexcept;

mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_heap_malloc(mi_heap_t* heap, size_t size) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(2);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_heap_zalloc(mi_heap_t* heap, size_t size) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(2);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_heap_calloc(mi_heap_t* heap, size_t count, size_t size) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size2(2, 3);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_heap_mallocn(mi_heap_t* heap, size_t count, size_t size) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size2(2, 3);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_heap_malloc_small(mi_heap_t* heap, size_t size) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(2);

mi_decl_nodiscard mi_decl_export void* mi_heap_realloc(mi_heap_t* heap, void* p, size_t newsize)              mi_attr_noexcept mi_attr_alloc_size(3);
mi_decl_nodiscard mi_decl_export void* mi_heap_reallocn(mi_heap_t* heap, void* p, size_t count, size_t size)  mi_attr_noexcept mi_attr_alloc_size2(3,4);;
mi_decl_nodiscard mi_decl_export void* mi_heap_reallocf(mi_heap_t* heap, void* p, size_t newsize)             mi_attr_noexcept mi_attr_alloc_size(3);

mi_decl_nodiscard mi_decl_export mi_decl_restrict char* mi_heap_strdup(mi_heap_t* heap, const char* s)            mi_attr_noexcept mi_attr_malloc;
mi_decl_nodiscard mi_decl_export mi_decl_restrict char* mi_heap_strndup(mi_heap_t* heap, const char* s, size_t n) mi_attr_noexcept mi_attr_malloc;
mi_decl_nodiscard mi_decl_export mi_decl_restrict char* mi_heap_realpath(mi_heap_t* heap, const char* fname, char* resolved_name) mi_attr_noexcept mi_attr_malloc;

mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_heap_malloc_aligned(mi_heap_t* heap, size_t size, size_t alignment) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(2) mi_attr_alloc_align(3);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_heap_malloc_aligned_at(mi_heap_t* heap, size_t size, size_t alignment, size_t offset) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(2);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_heap_zalloc_aligned(mi_heap_t* heap, size_t size, size_t alignment) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(2) mi_attr_alloc_align(3);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_heap_zalloc_aligned_at(mi_heap_t* heap, size_t size, size_t alignment, size_t offset) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(2);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_heap_calloc_aligned(mi_heap_t* heap, size_t count, size_t size, size_t alignment) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size2(2, 3) mi_attr_alloc_align(4);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_heap_calloc_aligned_at(mi_heap_t* heap, size_t count, size_t size, size_t alignment, size_t offset) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size2(2, 3);
mi_decl_nodiscard mi_decl_export void* mi_heap_realloc_aligned(mi_heap_t* heap, void* p, size_t newsize, size_t alignment) mi_attr_noexcept mi_attr_alloc_size(3) mi_attr_alloc_align(4);
mi_decl_nodiscard mi_decl_export void* mi_heap_realloc_aligned_at(mi_heap_t* heap, void* p, size_t newsize, size_t alignment, size_t offset) mi_attr_noexcept mi_attr_alloc_size(3);


// --------------------------------------------------------------------------------
// Zero initialized re-allocation.
// Only valid on memory that was originally allocated with zero initialization too.
// e.g. `mi_calloc`, `mi_zalloc`, `mi_zalloc_aligned` etc.
// see <https://github.com/microsoft/mimalloc/issues/63#issuecomment-508272992>
// --------------------------------------------------------------------------------

mi_decl_nodiscard mi_decl_export void* mi_rezalloc(void* p, size_t newsize)                mi_attr_noexcept mi_attr_alloc_size(2);
mi_decl_nodiscard mi_decl_export void* mi_recalloc(void* p, size_t newcount, size_t size)  mi_attr_noexcept mi_attr_alloc_size2(2,3);

mi_decl_nodiscard mi_decl_export void* mi_rezalloc_aligned(void* p, size_t newsize, size_t alignment) mi_attr_noexcept mi_attr_alloc_size(2) mi_attr_alloc_align(3);
mi_decl_nodiscard mi_decl_export void* mi_rezalloc_aligned_at(void* p, size_t newsize, size_t alignment, size_t offset) mi_attr_noexcept mi_attr_alloc_size(2);
mi_decl_nodiscard mi_decl_export void* mi_recalloc_aligned(void* p, size_t newcount, size_t size, size_t alignment) mi_attr_noexcept mi_attr_alloc_size2(2,3) mi_attr_alloc_align(4);
mi_decl_nodiscard mi_decl_export void* mi_recalloc_aligned_at(void* p, size_t newcount, size_t size, size_t alignment, size_t offset) mi_attr_noexcept mi_attr_alloc_size2(2,3);

mi_decl_nodiscard mi_decl_export void* mi_heap_rezalloc(mi_heap_t* heap, void* p, size_t newsize)                mi_attr_noexcept mi_attr_alloc_size(3);
mi_decl_nodiscard mi_decl_export void* mi_heap_recalloc(mi_heap_t* heap, void* p, size_t newcount, size_t size)  mi_attr_noexcept mi_attr_alloc_size2(3,4);

mi_decl_nodiscard mi_decl_export void* mi_heap_rezalloc_aligned(mi_heap_t* heap, void* p, size_t newsize, size_t alignment) mi_attr_noexcept mi_attr_alloc_size(3) mi_attr_alloc_align(4);
mi_decl_nodiscard mi_decl_export void* mi_heap_rezalloc_aligned_at(mi_heap_t* heap, void* p, size_t newsize, size_t alignment, size_t offset) mi_attr_noexcept mi_attr_alloc_size(3);
mi_decl_nodiscard mi_decl_export void* mi_heap_recalloc_aligned(mi_heap_t* heap, void* p, size_t newcount, size_t size, size_t alignment) mi_attr_noexcept mi_attr_alloc_size2(3,4) mi_attr_alloc_align(5);
mi_decl_nodiscard mi_decl_export void* mi_heap_recalloc_aligned_at(mi_heap_t* heap, void* p, size_t newcount, size_t size, size_t alignment, size_t offset) mi_attr_noexcept mi_attr_alloc_size2(3,4);


// ------------------------------------------------------
// Analysis
// ------------------------------------------------------

mi_decl_export bool mi_heap_contains_block(mi_heap_t* heap, const void* p);
mi_decl_export bool mi_heap_check_owned(mi_heap_t* heap, const void* p);
mi_decl_export bool mi_check_owned(const void* p);

// An area of heap space contains blocks of a single size.
typedef struct mi_heap_area_s {
  void*  blocks;      // start of the area containing heap blocks
  size_t reserved;    // bytes reserved for this area (virtual)
  size_t committed;   // current available bytes for this area
  size_t used;        // bytes in use by allocated blocks
  size_t block_size;  // size in bytes of each block
} mi_heap_area_t;

typedef bool (mi_cdecl mi_block_visit_fun)(const mi_heap_t* heap, const mi_heap_area_t* area, void* block, size_t block_size, void* arg);

mi_decl_export bool mi_heap_visit_blocks(const mi_heap_t* heap, bool visit_all_blocks, mi_block_visit_fun* visitor, void* arg);

// Experimental
mi_decl_nodiscard mi_decl_export bool mi_is_in_heap_region(const void* p) mi_attr_noexcept;
mi_decl_nodiscard mi_decl_export bool mi_is_redirected() mi_attr_noexcept;

mi_decl_export int mi_reserve_huge_os_pages_interleave(size_t pages, size_t numa_nodes, size_t timeout_msecs) mi_attr_noexcept;
mi_decl_export int mi_reserve_huge_os_pages_at(size_t pages, int numa_node, size_t timeout_msecs) mi_attr_noexcept;

// deprecated
mi_decl_export int  mi_reserve_huge_os_pages(size_t pages, double max_secs, size_t* pages_reserved) mi_attr_noexcept;


// ------------------------------------------------------
// Convenience
// ------------------------------------------------------

#define mi_malloc_tp(tp)                ((tp*)mi_malloc(sizeof(tp)))
#define mi_zalloc_tp(tp)                ((tp*)mi_zalloc(sizeof(tp)))
#define mi_calloc_tp(tp,n)              ((tp*)mi_calloc(n,sizeof(tp)))
#define mi_mallocn_tp(tp,n)             ((tp*)mi_mallocn(n,sizeof(tp)))
#define mi_reallocn_tp(p,tp,n)          ((tp*)mi_reallocn(p,n,sizeof(tp)))
#define mi_recalloc_tp(p,tp,n)          ((tp*)mi_recalloc(p,n,sizeof(tp)))

#define mi_heap_malloc_tp(hp,tp)        ((tp*)mi_heap_malloc(hp,sizeof(tp)))
#define mi_heap_zalloc_tp(hp,tp)        ((tp*)mi_heap_zalloc(hp,sizeof(tp)))
#define mi_heap_calloc_tp(hp,tp,n)      ((tp*)mi_heap_calloc(hp,n,sizeof(tp)))
#define mi_heap_mallocn_tp(hp,tp,n)     ((tp*)mi_heap_mallocn(hp,n,sizeof(tp)))
#define mi_heap_reallocn_tp(hp,p,tp,n)  ((tp*)mi_heap_reallocn(hp,p,n,sizeof(tp)))
#define mi_heap_recalloc_tp(hp,p,tp,n)  ((tp*)mi_heap_recalloc(hp,p,n,sizeof(tp)))


// ------------------------------------------------------
// Options, all `false` by default
// ------------------------------------------------------

typedef enum mi_option_e {
  // stable options
  mi_option_show_errors,
  mi_option_show_stats,
  mi_option_verbose,
  // the following options are experimental
  mi_option_eager_commit,
  mi_option_eager_region_commit,
  mi_option_reset_decommits,
  mi_option_large_os_pages,         // implies eager commit
  mi_option_reserve_huge_os_pages,
  mi_option_segment_cache,
  mi_option_page_reset,
  mi_option_abandoned_page_reset,
  mi_option_segment_reset,
  mi_option_eager_commit_delay,
  mi_option_reset_delay,
  mi_option_use_numa_nodes,
  mi_option_os_tag,
  mi_option_max_errors,
  _mi_option_last
} mi_option_t;


mi_decl_nodiscard mi_decl_export bool mi_option_is_enabled(mi_option_t option);
mi_decl_export void mi_option_enable(mi_option_t option);
mi_decl_export void mi_option_disable(mi_option_t option);
mi_decl_export void mi_option_set_enabled(mi_option_t option, bool enable);
mi_decl_export void mi_option_set_enabled_default(mi_option_t option, bool enable);

mi_decl_nodiscard mi_decl_export long mi_option_get(mi_option_t option);
mi_decl_export void mi_option_set(mi_option_t option, long value);
mi_decl_export void mi_option_set_default(mi_option_t option, long value);


// -------------------------------------------------------------------------------------------------------
// "mi" prefixed implementations of various posix, Unix, Windows, and C++ allocation functions.
// (This can be convenient when providing overrides of these functions as done in `mimalloc-override.h`.)
// note: we use `mi_cfree` as "checked free" and it checks if the pointer is in our heap before free-ing.
// -------------------------------------------------------------------------------------------------------

mi_decl_export void  mi_cfree(void* p) mi_attr_noexcept;
mi_decl_export void* mi__expand(void* p, size_t newsize) mi_attr_noexcept;
mi_decl_nodiscard mi_decl_export size_t mi_malloc_size(const void* p)        mi_attr_noexcept;
mi_decl_nodiscard mi_decl_export size_t mi_malloc_usable_size(const void *p) mi_attr_noexcept;

mi_decl_export int mi_posix_memalign(void** p, size_t alignment, size_t size)   mi_attr_noexcept;
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_memalign(size_t alignment, size_t size) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(2) mi_attr_alloc_align(1);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_valloc(size_t size)  mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(1);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_pvalloc(size_t size) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(1);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_aligned_alloc(size_t alignment, size_t size) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(2) mi_attr_alloc_align(1);

mi_decl_nodiscard mi_decl_export void* mi_reallocarray(void* p, size_t count, size_t size) mi_attr_noexcept mi_attr_alloc_size2(2,3);
mi_decl_nodiscard mi_decl_export void* mi_aligned_recalloc(void* p, size_t newcount, size_t size, size_t alignment) mi_attr_noexcept;
mi_decl_nodiscard mi_decl_export void* mi_aligned_offset_recalloc(void* p, size_t newcount, size_t size, size_t alignment, size_t offset) mi_attr_noexcept;

mi_decl_nodiscard mi_decl_export mi_decl_restrict unsigned short* mi_wcsdup(const unsigned short* s) mi_attr_noexcept mi_attr_malloc;
mi_decl_nodiscard mi_decl_export mi_decl_restrict unsigned char*  mi_mbsdup(const unsigned char* s)  mi_attr_noexcept mi_attr_malloc;
mi_decl_export int mi_dupenv_s(char** buf, size_t* size, const char* name)                      mi_attr_noexcept;
mi_decl_export int mi_wdupenv_s(unsigned short** buf, size_t* size, const unsigned short* name) mi_attr_noexcept;

mi_decl_export void mi_free_size(void* p, size_t size)                           mi_attr_noexcept;
mi_decl_export void mi_free_size_aligned(void* p, size_t size, size_t alignment) mi_attr_noexcept;
mi_decl_export void mi_free_aligned(void* p, size_t alignment)                   mi_attr_noexcept;

// The `mi_new` wrappers implement C++ semantics on out-of-memory instead of directly returning `NULL`.
// (and call `std::get_new_handler` and potentially raise a `std::bad_alloc` exception).
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_new(size_t size)                   mi_attr_malloc mi_attr_alloc_size(1);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_new_aligned(size_t size, size_t alignment) mi_attr_malloc mi_attr_alloc_size(1) mi_attr_alloc_align(2);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_new_nothrow(size_t size)           mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(1);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_new_aligned_nothrow(size_t size, size_t alignment) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(1) mi_attr_alloc_align(2);
mi_decl_nodiscard mi_decl_export mi_decl_restrict void* mi_new_n(size_t count, size_t size)   mi_attr_malloc mi_attr_alloc_size2(1, 2);
mi_decl_nodiscard mi_decl_export void* mi_new_realloc(void* p, size_t newsize)                mi_attr_alloc_size(2);
mi_decl_nodiscard mi_decl_export void* mi_new_reallocn(void* p, size_t newcount, size_t size) mi_attr_alloc_size2(2, 3);

#ifdef __cplusplus
}
#endif

// ---------------------------------------------------------------------------------------------
// Implement the C++ std::allocator interface for use in STL containers.
// (note: see `mimalloc-new-delete.h` for overriding the new/delete operators globally)
// ---------------------------------------------------------------------------------------------
#ifdef __cplusplus

#include <cstdint>     // PTRDIFF_MAX
#if (__cplusplus >= 201103L) || (_MSC_VER > 1900)  // C++11
#include <type_traits> // std::true_type
#include <utility>     // std::forward
#endif

template<class T> struct mi_stl_allocator {
  typedef T                 value_type;
  typedef std::size_t       size_type;
  typedef std::ptrdiff_t    difference_type;
  typedef value_type&       reference;
  typedef value_type const& const_reference;
  typedef value_type*       pointer;
  typedef value_type const* const_pointer;
  template <class U> struct rebind { typedef mi_stl_allocator<U> other; };

  mi_stl_allocator()                                             mi_attr_noexcept = default;
  mi_stl_allocator(const mi_stl_allocator&)                      mi_attr_noexcept = default;
  template<class U> mi_stl_allocator(const mi_stl_allocator<U>&) mi_attr_noexcept { }
  mi_stl_allocator  select_on_container_copy_construction() const { return *this; }
  void              deallocate(T* p, size_type) { mi_free(p); }

  #if (__cplusplus >= 201703L)  // C++17
  mi_decl_nodiscard T* allocate(size_type count) { return static_cast<T*>(mi_new_n(count, sizeof(T))); }
  mi_decl_nodiscard T* allocate(size_type count, const void*) { return allocate(count); }
  #else
  mi_decl_nodiscard pointer allocate(size_type count, const void* = 0) { return static_cast<pointer>(mi_new_n(count, sizeof(value_type))); }
  #endif

  #if ((__cplusplus >= 201103L) || (_MSC_VER > 1900))  // C++11
  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap            = std::true_type;
  using is_always_equal                        = std::true_type;
  template <class U, class ...Args> void construct(U* p, Args&& ...args) { ::new(p) U(std::forward<Args>(args)...); }
  template <class U> void destroy(U* p) mi_attr_noexcept { p->~U(); }
  #else
  void construct(pointer p, value_type const& val) { ::new(p) value_type(val); }
  void destroy(pointer p) { p->~value_type(); }
  #endif

  size_type     max_size() const mi_attr_noexcept { return (PTRDIFF_MAX/sizeof(value_type)); }
  pointer       address(reference x) const        { return &x; }
  const_pointer address(const_reference x) const  { return &x; }
};

template<class T1,class T2> bool operator==(const mi_stl_allocator<T1>& , const mi_stl_allocator<T2>& ) mi_attr_noexcept { return true; }
template<class T1,class T2> bool operator!=(const mi_stl_allocator<T1>& , const mi_stl_allocator<T2>& ) mi_attr_noexcept { return false; }
#endif // __cplusplus

#endif
/**** ended inlining mimalloc.h ****/
/**** start inlining mimalloc-internal.h ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#ifndef MIMALLOC_INTERNAL_H
#define MIMALLOC_INTERNAL_H

/**** start inlining mimalloc-types.h ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#ifndef MIMALLOC_TYPES_H
#define MIMALLOC_TYPES_H

#include <stddef.h>   // ptrdiff_t
#include <stdint.h>   // uintptr_t, uint16_t, etc
/**** start inlining mimalloc-atomic.h ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#ifndef MIMALLOC_ATOMIC_H
#define MIMALLOC_ATOMIC_H

// ------------------------------------------------------
// Atomics
// We need to be portable between C, C++, and MSVC.
// ------------------------------------------------------

#if defined(_MSC_VER)
#define _Atomic(tp)         tp
#define ATOMIC_VAR_INIT(x)  x
#elif defined(__cplusplus)
#include <atomic>
#define  _Atomic(tp)        std::atomic<tp>
#else
#include <stdatomic.h>
#endif

// ------------------------------------------------------
// Atomic operations specialized for mimalloc
// ------------------------------------------------------

// Atomically add a value; returns the previous value. Memory ordering is relaxed.
static inline uintptr_t mi_atomic_add(volatile _Atomic(uintptr_t)* p, uintptr_t add);

// Atomically "and" a value; returns the previous value. Memory ordering is relaxed.
static inline uintptr_t mi_atomic_and(volatile _Atomic(uintptr_t)* p, uintptr_t x);

// Atomically "or" a value; returns the previous value. Memory ordering is relaxed.
static inline uintptr_t mi_atomic_or(volatile _Atomic(uintptr_t)* p, uintptr_t x);

// Atomically compare and exchange a value; returns `true` if successful.
// May fail spuriously. Memory ordering as release on success, and relaxed on failure.
// (Note: expected and desired are in opposite order from atomic_compare_exchange)
static inline bool mi_atomic_cas_weak(volatile _Atomic(uintptr_t)* p, uintptr_t desired, uintptr_t expected);

// Atomically compare and exchange a value; returns `true` if successful.
// Memory ordering is acquire-release
// (Note: expected and desired are in opposite order from atomic_compare_exchange)
static inline bool mi_atomic_cas_strong(volatile _Atomic(uintptr_t)* p, uintptr_t desired, uintptr_t expected);

// Atomically exchange a value. Memory ordering is acquire-release.
static inline uintptr_t mi_atomic_exchange(volatile _Atomic(uintptr_t)* p, uintptr_t exchange);

// Atomically read a value. Memory ordering is relaxed.
static inline uintptr_t mi_atomic_read_relaxed(const volatile _Atomic(uintptr_t)* p);

// Atomically read a value. Memory ordering is acquire.
static inline uintptr_t mi_atomic_read(const volatile _Atomic(uintptr_t)* p);

// Atomically write a value. Memory ordering is release.
static inline void mi_atomic_write(volatile _Atomic(uintptr_t)* p, uintptr_t x);

// Yield
static inline void mi_atomic_yield(void);

// Atomically add a 64-bit value; returns the previous value.
// Note: not using _Atomic(int64_t) as it is only used for statistics.
static inline void mi_atomic_addi64(volatile int64_t* p, int64_t add);

// Atomically update `*p` with the maximum of `*p` and `x` as a 64-bit value.
// Returns the previous value. Note: not using _Atomic(int64_t) as it is only used for statistics.
static inline void mi_atomic_maxi64(volatile int64_t* p, int64_t x);

// Atomically read a 64-bit value
// Note: not using _Atomic(int64_t) as it is only used for statistics.
static inline int64_t mi_atomic_readi64(volatile int64_t* p);

// Atomically subtract a value; returns the previous value.
static inline uintptr_t mi_atomic_sub(volatile _Atomic(uintptr_t)* p, uintptr_t sub) {
  return mi_atomic_add(p, (uintptr_t)(-((intptr_t)sub)));
}

// Atomically increment a value; returns the incremented result.
static inline uintptr_t mi_atomic_increment(volatile _Atomic(uintptr_t)* p) {
  return mi_atomic_add(p, 1);
}

// Atomically decrement a value; returns the decremented result.
static inline uintptr_t mi_atomic_decrement(volatile _Atomic(uintptr_t)* p) {
  return mi_atomic_sub(p, 1);
}

// Atomically add a signed value; returns the previous value.
static inline intptr_t mi_atomic_addi(volatile _Atomic(intptr_t)* p, intptr_t add) {
  return (intptr_t)mi_atomic_add((volatile _Atomic(uintptr_t)*)p, (uintptr_t)add);
}

// Atomically subtract a signed value; returns the previous value.
static inline intptr_t mi_atomic_subi(volatile _Atomic(intptr_t)* p, intptr_t sub) {
  return (intptr_t)mi_atomic_addi(p,-sub);
}

// Atomically read a pointer; Memory order is relaxed (i.e. no fence, only atomic).
#define mi_atomic_read_ptr_relaxed(T,p)  \
  (T*)(mi_atomic_read_relaxed((const volatile _Atomic(uintptr_t)*)(p)))

// Atomically read a pointer; Memory order is acquire.
#define mi_atomic_read_ptr(T,p) \
  (T*)(mi_atomic_read((const volatile _Atomic(uintptr_t)*)(p)))

// Atomically write a pointer; Memory order is acquire.
#define mi_atomic_write_ptr(T,p,x) \
  mi_atomic_write((volatile _Atomic(uintptr_t)*)(p), (uintptr_t)((T*)x))

// Atomically compare and exchange a pointer; returns `true` if successful. May fail spuriously.
// Memory order is release. (like a write)
// (Note: expected and desired are in opposite order from atomic_compare_exchange)
#define mi_atomic_cas_ptr_weak(T,p,desired,expected) \
  mi_atomic_cas_weak((volatile _Atomic(uintptr_t)*)(p), (uintptr_t)((T*)(desired)), (uintptr_t)((T*)(expected)))
    
// Atomically compare and exchange a pointer; returns `true` if successful. Memory order is acquire_release.
// (Note: expected and desired are in opposite order from atomic_compare_exchange)
#define mi_atomic_cas_ptr_strong(T,p,desired,expected) \
  mi_atomic_cas_strong((volatile _Atomic(uintptr_t)*)(p),(uintptr_t)((T*)(desired)), (uintptr_t)((T*)(expected))) 

// Atomically exchange a pointer value.
#define mi_atomic_exchange_ptr(T,p,exchange) \
  (T*)mi_atomic_exchange((volatile _Atomic(uintptr_t)*)(p), (uintptr_t)((T*)exchange))


#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#ifdef _WIN64
typedef LONG64   msc_intptr_t;
#define MI_64(f) f##64
#else
typedef LONG     msc_intptr_t;
#define MI_64(f) f
#endif
static inline uintptr_t mi_atomic_add(volatile _Atomic(uintptr_t)* p, uintptr_t add) {
  return (uintptr_t)MI_64(_InterlockedExchangeAdd)((volatile msc_intptr_t*)p, (msc_intptr_t)add);
}
static inline uintptr_t mi_atomic_and(volatile _Atomic(uintptr_t)* p, uintptr_t x) {
  return (uintptr_t)MI_64(_InterlockedAnd)((volatile msc_intptr_t*)p, (msc_intptr_t)x);
}
static inline uintptr_t mi_atomic_or(volatile _Atomic(uintptr_t)* p, uintptr_t x) {
  return (uintptr_t)MI_64(_InterlockedOr)((volatile msc_intptr_t*)p, (msc_intptr_t)x);
}
static inline bool mi_atomic_cas_strong(volatile _Atomic(uintptr_t)* p, uintptr_t desired, uintptr_t expected) {
  return (expected == (uintptr_t)MI_64(_InterlockedCompareExchange)((volatile msc_intptr_t*)p, (msc_intptr_t)desired, (msc_intptr_t)expected));
}
static inline bool mi_atomic_cas_weak(volatile _Atomic(uintptr_t)* p, uintptr_t desired, uintptr_t expected) {
  return mi_atomic_cas_strong(p,desired,expected);
}
static inline uintptr_t mi_atomic_exchange(volatile _Atomic(uintptr_t)* p, uintptr_t exchange) {
  return (uintptr_t)MI_64(_InterlockedExchange)((volatile msc_intptr_t*)p, (msc_intptr_t)exchange);
}
static inline uintptr_t mi_atomic_read(volatile _Atomic(uintptr_t) const* p) {
  return *p;
}
static inline uintptr_t mi_atomic_read_relaxed(volatile _Atomic(uintptr_t) const* p) {
  return *p;
}
static inline void mi_atomic_write(volatile _Atomic(uintptr_t)* p, uintptr_t x) {
  #if defined(_M_IX86) || defined(_M_X64)
  *p = x;
  #else
  mi_atomic_exchange(p,x);
  #endif
}
static inline void mi_atomic_yield(void) {
  YieldProcessor();
}
static inline void mi_atomic_addi64(volatile _Atomic(int64_t)* p, int64_t add) {
  #ifdef _WIN64
  mi_atomic_addi(p,add);
  #else
  int64_t current;
  int64_t sum;
  do {
    current = *p;
    sum = current + add;
  } while (_InterlockedCompareExchange64(p, sum, current) != current);
  #endif
}

static inline void mi_atomic_maxi64(volatile _Atomic(int64_t)*p, int64_t x) {
  int64_t current;
  do {
    current = *p;
  } while (current < x && _InterlockedCompareExchange64(p, x, current) != current);
}

static inline int64_t mi_atomic_readi64(volatile _Atomic(int64_t)*p) {
  #ifdef _WIN64
  return *p;
  #else
  int64_t current;
  do {
    current = *p;
  } while (_InterlockedCompareExchange64(p, current, current) != current);
  return current;
  #endif
}

#else
#ifdef __cplusplus
#define  MI_USING_STD   using namespace std;
#else
#define  MI_USING_STD
#endif
static inline uintptr_t mi_atomic_add(volatile _Atomic(uintptr_t)* p, uintptr_t add) {
  MI_USING_STD
  return atomic_fetch_add_explicit(p, add, memory_order_relaxed);
}
static inline uintptr_t mi_atomic_and(volatile _Atomic(uintptr_t)* p, uintptr_t x) {
  MI_USING_STD
  return atomic_fetch_and_explicit(p, x, memory_order_acq_rel);
}
static inline uintptr_t mi_atomic_or(volatile _Atomic(uintptr_t)* p, uintptr_t x) {
  MI_USING_STD
  return atomic_fetch_or_explicit(p, x, memory_order_acq_rel);
}
static inline bool mi_atomic_cas_weak(volatile _Atomic(uintptr_t)* p, uintptr_t desired, uintptr_t expected) {
  MI_USING_STD
  return atomic_compare_exchange_weak_explicit(p, &expected, desired, memory_order_acq_rel, memory_order_acquire);
}
static inline bool mi_atomic_cas_strong(volatile _Atomic(uintptr_t)* p, uintptr_t desired, uintptr_t expected) {
  MI_USING_STD
  return atomic_compare_exchange_strong_explicit(p, &expected, desired, memory_order_acq_rel, memory_order_acquire);
}
static inline uintptr_t mi_atomic_exchange(volatile _Atomic(uintptr_t)* p, uintptr_t exchange) {
  MI_USING_STD
  return atomic_exchange_explicit(p, exchange, memory_order_acq_rel);
}
static inline uintptr_t mi_atomic_read_relaxed(const volatile _Atomic(uintptr_t)* p) {
  MI_USING_STD
  return atomic_load_explicit((volatile _Atomic(uintptr_t)*) p, memory_order_relaxed);
}
static inline uintptr_t mi_atomic_read(const volatile _Atomic(uintptr_t)* p) {
  MI_USING_STD
  return atomic_load_explicit((volatile _Atomic(uintptr_t)*) p, memory_order_acquire);
}
static inline void mi_atomic_write(volatile _Atomic(uintptr_t)* p, uintptr_t x) {
  MI_USING_STD
  return atomic_store_explicit(p, x, memory_order_release);
}
static inline void mi_atomic_addi64(volatile int64_t* p, int64_t add) {
  MI_USING_STD
  atomic_fetch_add_explicit((volatile _Atomic(int64_t)*)p, add, memory_order_relaxed);
}
static inline int64_t mi_atomic_readi64(volatile int64_t* p) {
  MI_USING_STD
  return atomic_load_explicit((volatile _Atomic(int64_t)*) p, memory_order_relaxed);
}
static inline void mi_atomic_maxi64(volatile int64_t* p, int64_t x) {
  MI_USING_STD
  int64_t current;
  do {
    current = mi_atomic_readi64(p);
  } while (current < x && !atomic_compare_exchange_weak_explicit((volatile _Atomic(int64_t)*)p, &current, x, memory_order_acq_rel, memory_order_relaxed));
}

#if defined(__cplusplus)
  #include <thread>
  static inline void mi_atomic_yield(void) {
    std::this_thread::yield();
  }
#elif (defined(__GNUC__) || defined(__clang__)) && \
      (defined(__x86_64__) || defined(__i386__) || defined(__arm__) || defined(__aarch64__))
#if defined(__x86_64__) || defined(__i386__)
  static inline void mi_atomic_yield(void) {
    asm volatile ("pause" ::: "memory");
  }
#elif defined(__arm__) || defined(__aarch64__)
  static inline void mi_atomic_yield(void) {
    asm volatile("yield");
  }
#endif
#elif defined(__wasi__) || defined(__EMSCRIPTEN__)
  #include <sched.h>
  static inline void mi_atomic_yield(void) {
    sched_yield();
  }
#else
  #include <unistd.h>
  static inline void mi_atomic_yield(void) {
    sleep(0);
  }
#endif

#endif

#endif // __MIMALLOC_ATOMIC_H
/**** ended inlining mimalloc-atomic.h ****/

// Minimal alignment necessary. On most platforms 16 bytes are needed
// due to SSE registers for example. This must be at least `MI_INTPTR_SIZE`
#define MI_MAX_ALIGN_SIZE  16   // sizeof(max_align_t)

// ------------------------------------------------------
// Variants
// ------------------------------------------------------

// Define NDEBUG in the release version to disable assertions.
// #define NDEBUG

// Define MI_STAT as 1 to maintain statistics; set it to 2 to have detailed statistics (but costs some performance).
// #define MI_STAT 1

// Define MI_SECURE to enable security mitigations
// #define MI_SECURE 1  // guard page around metadata
// #define MI_SECURE 2  // guard page around each mimalloc page
// #define MI_SECURE 3  // encode free lists (detect corrupted free list (buffer overflow), and invalid pointer free)
// #define MI_SECURE 4  // checks for double free. (may be more expensive)

#if !defined(MI_SECURE)
#define MI_SECURE 0
#endif

// Define MI_DEBUG for debug mode
// #define MI_DEBUG 1  // basic assertion checks and statistics, check double free, corrupted free list, and invalid pointer free.
// #define MI_DEBUG 2  // + internal assertion checks
// #define MI_DEBUG 3  // + extensive internal invariant checking (cmake -DMI_DEBUG_FULL=ON)
#if !defined(MI_DEBUG)
#if !defined(NDEBUG) || defined(_DEBUG)
#define MI_DEBUG 2
#else
#define MI_DEBUG 0
#endif
#endif

// Reserve extra padding at the end of each block to be more resilient against heap block overflows.
// The padding can detect byte-precise buffer overflow on free.
#if !defined(MI_PADDING) && (MI_DEBUG>=1)
#define MI_PADDING  1
#endif


// Encoded free lists allow detection of corrupted free lists
// and can detect buffer overflows, modify after free, and double `free`s.
#if (MI_SECURE>=3 || MI_DEBUG>=1 || MI_PADDING > 0)
#define MI_ENCODE_FREELIST  1
#endif

// ------------------------------------------------------
// Platform specific values
// ------------------------------------------------------

// ------------------------------------------------------
// Size of a pointer.
// We assume that `sizeof(void*)==sizeof(intptr_t)`
// and it holds for all platforms we know of.
//
// However, the C standard only requires that:
//  p == (void*)((intptr_t)p))
// but we also need:
//  i == (intptr_t)((void*)i)
// or otherwise one might define an intptr_t type that is larger than a pointer...
// ------------------------------------------------------

#if INTPTR_MAX == 9223372036854775807LL
# define MI_INTPTR_SHIFT (3)
#elif INTPTR_MAX == 2147483647LL
# define MI_INTPTR_SHIFT (2)
#else
#error platform must be 32 or 64 bits
#endif

#define MI_INTPTR_SIZE  (1<<MI_INTPTR_SHIFT)
#define MI_INTPTR_BITS  (MI_INTPTR_SIZE*8)

#define KiB     ((size_t)1024)
#define MiB     (KiB*KiB)
#define GiB     (MiB*KiB)


// ------------------------------------------------------
// Main internal data-structures
// ------------------------------------------------------

// Main tuning parameters for segment and page sizes
// Sizes for 64-bit, divide by two for 32-bit
#define MI_SMALL_PAGE_SHIFT               (13 + MI_INTPTR_SHIFT)      // 64kb
#define MI_MEDIUM_PAGE_SHIFT              ( 3 + MI_SMALL_PAGE_SHIFT)  // 512kb
#define MI_LARGE_PAGE_SHIFT               ( 3 + MI_MEDIUM_PAGE_SHIFT) // 4mb
#define MI_SEGMENT_SHIFT                  ( MI_LARGE_PAGE_SHIFT)      // 4mb

// Derived constants
#define MI_SEGMENT_SIZE                   (1UL<<MI_SEGMENT_SHIFT)
#define MI_SEGMENT_MASK                   ((uintptr_t)MI_SEGMENT_SIZE - 1)

#define MI_SMALL_PAGE_SIZE                (1UL<<MI_SMALL_PAGE_SHIFT)
#define MI_MEDIUM_PAGE_SIZE               (1UL<<MI_MEDIUM_PAGE_SHIFT)
#define MI_LARGE_PAGE_SIZE                (1UL<<MI_LARGE_PAGE_SHIFT)

#define MI_SMALL_PAGES_PER_SEGMENT        (MI_SEGMENT_SIZE/MI_SMALL_PAGE_SIZE)
#define MI_MEDIUM_PAGES_PER_SEGMENT       (MI_SEGMENT_SIZE/MI_MEDIUM_PAGE_SIZE)
#define MI_LARGE_PAGES_PER_SEGMENT        (MI_SEGMENT_SIZE/MI_LARGE_PAGE_SIZE)

// The max object size are checked to not waste more than 12.5% internally over the page sizes.
// (Except for large pages since huge objects are allocated in 4MiB chunks)
#define MI_SMALL_OBJ_SIZE_MAX             (MI_SMALL_PAGE_SIZE/4)   // 16kb
#define MI_MEDIUM_OBJ_SIZE_MAX            (MI_MEDIUM_PAGE_SIZE/4)  // 128kb
#define MI_LARGE_OBJ_SIZE_MAX             (MI_LARGE_PAGE_SIZE/2)   // 2mb
#define MI_LARGE_OBJ_WSIZE_MAX            (MI_LARGE_OBJ_SIZE_MAX/MI_INTPTR_SIZE)
#define MI_HUGE_OBJ_SIZE_MAX              (2*MI_INTPTR_SIZE*MI_SEGMENT_SIZE)        // (must match MI_REGION_MAX_ALLOC_SIZE in memory.c)

// Maximum number of size classes. (spaced exponentially in 12.5% increments)
#define MI_BIN_HUGE  (73U)

#if (MI_LARGE_OBJ_WSIZE_MAX >= 655360)
#error "define more bins"
#endif

// Used as a special value to encode block sizes in 32 bits.
#define MI_HUGE_BLOCK_SIZE   ((uint32_t)MI_HUGE_OBJ_SIZE_MAX)

// The free lists use encoded next fields
// (Only actually encodes when MI_ENCODED_FREELIST is defined.)
typedef uintptr_t mi_encoded_t;

// free lists contain blocks
typedef struct mi_block_s {
  mi_encoded_t next;
} mi_block_t;


// The delayed flags are used for efficient multi-threaded free-ing
typedef enum mi_delayed_e {
  MI_USE_DELAYED_FREE   = 0, // push on the owning heap thread delayed list
  MI_DELAYED_FREEING    = 1, // temporary: another thread is accessing the owning heap
  MI_NO_DELAYED_FREE    = 2, // optimize: push on page local thread free queue if another block is already in the heap thread delayed free list
  MI_NEVER_DELAYED_FREE = 3  // sticky, only resets on page reclaim
} mi_delayed_t;


// The `in_full` and `has_aligned` page flags are put in a union to efficiently
// test if both are false (`full_aligned == 0`) in the `mi_free` routine.
typedef union mi_page_flags_s {
  uint8_t full_aligned;
  struct {
    uint8_t in_full : 1;
    uint8_t has_aligned : 1;
  } x;
} mi_page_flags_t;

// Thread free list.
// We use the bottom 2 bits of the pointer for mi_delayed_t flags
typedef uintptr_t mi_thread_free_t;

// A page contains blocks of one specific size (`block_size`).
// Each page has three list of free blocks:
// `free` for blocks that can be allocated,
// `local_free` for freed blocks that are not yet available to `mi_malloc`
// `thread_free` for freed blocks by other threads
// The `local_free` and `thread_free` lists are migrated to the `free` list
// when it is exhausted. The separate `local_free` list is necessary to
// implement a monotonic heartbeat. The `thread_free` list is needed for
// avoiding atomic operations in the common case.
//
//
// `used - |thread_free|` == actual blocks that are in use (alive)
// `used - |thread_free| + |free| + |local_free| == capacity`
//
// We don't count `freed` (as |free|) but use `used` to reduce
// the number of memory accesses in the `mi_page_all_free` function(s).
//
// Notes: 
// - Access is optimized for `mi_free` and `mi_page_alloc` (in `alloc.c`)
// - Using `uint16_t` does not seem to slow things down
// - The size is 8 words on 64-bit which helps the page index calculations
//   (and 10 words on 32-bit, and encoded free lists add 2 words. Sizes 10 
//    and 12 are still good for address calculation)
// - To limit the structure size, the `xblock_size` is 32-bits only; for 
//   blocks > MI_HUGE_BLOCK_SIZE the size is determined from the segment page size
// - `thread_free` uses the bottom bits as a delayed-free flags to optimize
//   concurrent frees where only the first concurrent free adds to the owning
//   heap `thread_delayed_free` list (see `alloc.c:mi_free_block_mt`).
//   The invariant is that no-delayed-free is only set if there is
//   at least one block that will be added, or as already been added, to 
//   the owning heap `thread_delayed_free` list. This guarantees that pages
//   will be freed correctly even if only other threads free blocks.
typedef struct mi_page_s {
  // "owned" by the segment
  uint8_t               segment_idx;       // index in the segment `pages` array, `page == &segment->pages[page->segment_idx]`
  uint8_t               segment_in_use:1;  // `true` if the segment allocated this page
  uint8_t               is_reset:1;        // `true` if the page memory was reset
  uint8_t               is_committed:1;    // `true` if the page virtual memory is committed
  uint8_t               is_zero_init:1;    // `true` if the page was zero initialized

  // layout like this to optimize access in `mi_malloc` and `mi_free`
  uint16_t              capacity;          // number of blocks committed, must be the first field, see `segment.c:page_clear`
  uint16_t              reserved;          // number of blocks reserved in memory
  mi_page_flags_t       flags;             // `in_full` and `has_aligned` flags (8 bits)
  uint8_t               is_zero:1;         // `true` if the blocks in the free list are zero initialized
  uint8_t               retire_expire:7;   // expiration count for retired blocks

  mi_block_t*           free;              // list of available free blocks (`malloc` allocates from this list)
  #ifdef MI_ENCODE_FREELIST
  uintptr_t             keys[2];           // two random keys to encode the free lists (see `_mi_block_next`)
  #endif
  uint32_t              used;              // number of blocks in use (including blocks in `local_free` and `thread_free`)
  uint32_t              xblock_size;       // size available in each block (always `>0`) 

  mi_block_t*           local_free;        // list of deferred free blocks by this thread (migrates to `free`)
  volatile _Atomic(mi_thread_free_t) xthread_free;   // list of deferred free blocks freed by other threads
  volatile _Atomic(uintptr_t)        xheap;
  
  struct mi_page_s*     next;              // next page owned by this thread with the same `block_size`
  struct mi_page_s*     prev;              // previous page owned by this thread with the same `block_size`
} mi_page_t;



typedef enum mi_page_kind_e {
  MI_PAGE_SMALL,    // small blocks go into 64kb pages inside a segment
  MI_PAGE_MEDIUM,   // medium blocks go into 512kb pages inside a segment
  MI_PAGE_LARGE,    // larger blocks go into a single page spanning a whole segment
  MI_PAGE_HUGE      // huge blocks (>512kb) are put into a single page in a segment of the exact size (but still 2mb aligned)
} mi_page_kind_t;

// Segments are large allocated memory blocks (2mb on 64 bit) from
// the OS. Inside segments we allocated fixed size _pages_ that
// contain blocks.
typedef struct mi_segment_s {
  // memory fields
  size_t          memid;            // id for the os-level memory manager
  bool            mem_is_fixed;     // `true` if we cannot decommit/reset/protect in this memory (i.e. when allocated using large OS pages)
  bool            mem_is_committed; // `true` if the whole segment is eagerly committed

  // segment fields
  struct mi_segment_s* next;        // must be the first segment field -- see `segment.c:segment_alloc`
  struct mi_segment_s* prev;
  struct mi_segment_s* abandoned_next;
  size_t          abandoned;        // abandoned pages (i.e. the original owning thread stopped) (`abandoned <= used`)
  size_t          abandoned_visits; // count how often this segment is visited in the abandoned list (to force reclaim it it is too long)

  size_t          used;        // count of pages in use (`used <= capacity`)
  size_t          capacity;    // count of available pages (`#free + used`)
  size_t          segment_size;// for huge pages this may be different from `MI_SEGMENT_SIZE`
  size_t          segment_info_size;  // space we are using from the first page for segment meta-data and possible guard pages.
  uintptr_t       cookie;      // verify addresses in secure mode: `_mi_ptr_cookie(segment) == segment->cookie`

  // layout like this to optimize access in `mi_free`
  size_t          page_shift;  // `1 << page_shift` == the page sizes == `page->block_size * page->reserved` (unless the first page, then `-segment_info_size`).
  volatile _Atomic(uintptr_t) thread_id;   // unique id of the thread owning this segment
  mi_page_kind_t  page_kind;   // kind of pages: small, large, or huge
  mi_page_t       pages[1];    // up to `MI_SMALL_PAGES_PER_SEGMENT` pages
} mi_segment_t;


// ------------------------------------------------------
// Heaps
// Provide first-class heaps to allocate from.
// A heap just owns a set of pages for allocation and
// can only be allocate/reallocate from the thread that created it.
// Freeing blocks can be done from any thread though.
// Per thread, the segments are shared among its heaps.
// Per thread, there is always a default heap that is
// used for allocation; it is initialized to statically
// point to an empty heap to avoid initialization checks
// in the fast path.
// ------------------------------------------------------

// Thread local data
typedef struct mi_tld_s mi_tld_t;

// Pages of a certain block size are held in a queue.
typedef struct mi_page_queue_s {
  mi_page_t* first;
  mi_page_t* last;
  size_t     block_size;
} mi_page_queue_t;

#define MI_BIN_FULL  (MI_BIN_HUGE+1)

// Random context
typedef struct mi_random_cxt_s {
  uint32_t input[16];
  uint32_t output[16];
  int      output_available;
} mi_random_ctx_t;


// In debug mode there is a padding stucture at the end of the blocks to check for buffer overflows
#if (MI_PADDING)
typedef struct mi_padding_s {
  uint32_t canary; // encoded block value to check validity of the padding (in case of overflow)
  uint32_t delta;  // padding bytes before the block. (mi_usable_size(p) - delta == exact allocated bytes)
} mi_padding_t;
#define MI_PADDING_SIZE   (sizeof(mi_padding_t))
#define MI_PADDING_WSIZE  ((MI_PADDING_SIZE + MI_INTPTR_SIZE - 1) / MI_INTPTR_SIZE)
#else
#define MI_PADDING_SIZE   0
#define MI_PADDING_WSIZE  0
#endif

#define MI_PAGES_DIRECT   (MI_SMALL_WSIZE_MAX + MI_PADDING_WSIZE + 1)


// A heap owns a set of pages.
struct mi_heap_s {
  mi_tld_t*             tld;
  mi_page_t*            pages_free_direct[MI_PAGES_DIRECT];  // optimize: array where every entry points a page with possibly free blocks in the corresponding queue for that size.
  mi_page_queue_t       pages[MI_BIN_FULL + 1];              // queue of pages for each size class (or "bin")
  volatile _Atomic(mi_block_t*) thread_delayed_free;
  uintptr_t             thread_id;                           // thread this heap belongs too
  uintptr_t             cookie;                              // random cookie to verify pointers (see `_mi_ptr_cookie`)
  uintptr_t             keys[2];                             // two random keys used to encode the `thread_delayed_free` list
  mi_random_ctx_t       random;                              // random number context used for secure allocation
  size_t                page_count;                          // total number of pages in the `pages` queues.
  size_t                page_retired_min;                    // smallest retired index (retired pages are fully free, but still in the page queues)
  size_t                page_retired_max;                    // largest retired index into the `pages` array.
  mi_heap_t*            next;                                // list of heaps per thread
  bool                  no_reclaim;                          // `true` if this heap should not reclaim abandoned pages
};



// ------------------------------------------------------
// Debug
// ------------------------------------------------------

#define MI_DEBUG_UNINIT     (0xD0)
#define MI_DEBUG_FREED      (0xDF)
#define MI_DEBUG_PADDING    (0xDE)

#if (MI_DEBUG)
// use our own assertion to print without memory allocation
void _mi_assert_fail(const char* assertion, const char* fname, unsigned int line, const char* func );
#define mi_assert(expr)     ((expr) ? (void)0 : _mi_assert_fail(#expr,__FILE__,__LINE__,__func__))
#else
#define mi_assert(x)
#endif

#if (MI_DEBUG>1)
#define mi_assert_internal    mi_assert
#else
#define mi_assert_internal(x)
#endif

#if (MI_DEBUG>2)
#define mi_assert_expensive   mi_assert
#else
#define mi_assert_expensive(x)
#endif

// ------------------------------------------------------
// Statistics
// ------------------------------------------------------

#ifndef MI_STAT
#if (MI_DEBUG>0)
#define MI_STAT 2
#else
#define MI_STAT 0
#endif
#endif

typedef struct mi_stat_count_s {
  int64_t allocated;
  int64_t freed;
  int64_t peak;
  int64_t current;
} mi_stat_count_t;

typedef struct mi_stat_counter_s {
  int64_t total;
  int64_t count;
} mi_stat_counter_t;

typedef struct mi_stats_s {
  mi_stat_count_t segments;
  mi_stat_count_t pages;
  mi_stat_count_t reserved;
  mi_stat_count_t committed;
  mi_stat_count_t reset;
  mi_stat_count_t page_committed;
  mi_stat_count_t segments_abandoned;
  mi_stat_count_t pages_abandoned;
  mi_stat_count_t threads;
  mi_stat_count_t huge;
  mi_stat_count_t giant;
  mi_stat_count_t malloc;
  mi_stat_count_t segments_cache;
  mi_stat_counter_t pages_extended;
  mi_stat_counter_t mmap_calls;
  mi_stat_counter_t commit_calls;
  mi_stat_counter_t page_no_retire;
  mi_stat_counter_t searches;
  mi_stat_counter_t huge_count;
  mi_stat_counter_t giant_count;
#if MI_STAT>1
  mi_stat_count_t normal[MI_BIN_HUGE+1];
#endif
} mi_stats_t;


void _mi_stat_increase(mi_stat_count_t* stat, size_t amount);
void _mi_stat_decrease(mi_stat_count_t* stat, size_t amount);
void _mi_stat_counter_increase(mi_stat_counter_t* stat, size_t amount);

#if (MI_STAT)
#define mi_stat_increase(stat,amount)         _mi_stat_increase( &(stat), amount)
#define mi_stat_decrease(stat,amount)         _mi_stat_decrease( &(stat), amount)
#define mi_stat_counter_increase(stat,amount) _mi_stat_counter_increase( &(stat), amount)
#else
#define mi_stat_increase(stat,amount)         (void)0
#define mi_stat_decrease(stat,amount)         (void)0
#define mi_stat_counter_increase(stat,amount) (void)0
#endif

#define mi_heap_stat_increase(heap,stat,amount)  mi_stat_increase( (heap)->tld->stats.stat, amount)
#define mi_heap_stat_decrease(heap,stat,amount)  mi_stat_decrease( (heap)->tld->stats.stat, amount)

// ------------------------------------------------------
// Thread Local data
// ------------------------------------------------------

typedef int64_t  mi_msecs_t;

// Queue of segments
typedef struct mi_segment_queue_s {
  mi_segment_t* first;
  mi_segment_t* last;
} mi_segment_queue_t;

// OS thread local data
typedef struct mi_os_tld_s {
  size_t                region_idx;   // start point for next allocation
  mi_stats_t*           stats;        // points to tld stats
} mi_os_tld_t;

// Segments thread local data
typedef struct mi_segments_tld_s {
  mi_segment_queue_t  small_free;   // queue of segments with free small pages
  mi_segment_queue_t  medium_free;  // queue of segments with free medium pages
  mi_page_queue_t     pages_reset;  // queue of freed pages that can be reset
  size_t              count;        // current number of segments;
  size_t              peak_count;   // peak number of segments
  size_t              current_size; // current size of all segments
  size_t              peak_size;    // peak size of all segments
  size_t              cache_count;  // number of segments in the cache
  size_t              cache_size;   // total size of all segments in the cache
  mi_segment_t*       cache;        // (small) cache of segments
  mi_stats_t*         stats;        // points to tld stats
  mi_os_tld_t*        os;           // points to os stats
} mi_segments_tld_t;

// Thread local data
struct mi_tld_s {
  unsigned long long  heartbeat;     // monotonic heartbeat count
  bool                recurse;       // true if deferred was called; used to prevent infinite recursion.
  mi_heap_t*          heap_backing;  // backing heap of this thread (cannot be deleted)
  mi_heap_t*          heaps;         // list of heaps in this thread (so we can abandon all when the thread terminates)
  mi_segments_tld_t   segments;      // segment tld
  mi_os_tld_t         os;            // os tld
  mi_stats_t          stats;         // statistics
};

#endif
/**** ended inlining mimalloc-types.h ****/

#if (MI_DEBUG>0)
#define mi_trace_message(...)  _mi_trace_message(__VA_ARGS__)
#else
#define mi_trace_message(...)
#endif

#define MI_CACHE_LINE          64
#if defined(_MSC_VER)
#pragma warning(disable:4127)   // suppress constant conditional warning (due to MI_SECURE paths)
#define mi_decl_noinline        __declspec(noinline)
#define mi_decl_thread          __declspec(thread)
#define mi_decl_cache_align     __declspec(align(MI_CACHE_LINE))
#elif (defined(__GNUC__) && (__GNUC__>=3))  // includes clang and icc
#define mi_decl_noinline        __attribute__((noinline))
#define mi_decl_thread          __thread
#define mi_decl_cache_align     __attribute__((aligned(MI_CACHE_LINE)))
#else
#define mi_decl_noinline
#define mi_decl_thread          __thread        // hope for the best :-)
#define mi_decl_cache_align
#endif


// "options.c"
void       _mi_fputs(mi_output_fun* out, void* arg, const char* prefix, const char* message);
void       _mi_fprintf(mi_output_fun* out, void* arg, const char* fmt, ...);
void       _mi_warning_message(const char* fmt, ...);
void       _mi_verbose_message(const char* fmt, ...);
void       _mi_trace_message(const char* fmt, ...);
void       _mi_options_init(void);
void       _mi_error_message(int err, const char* fmt, ...);

// random.c
void       _mi_random_init(mi_random_ctx_t* ctx);
void       _mi_random_split(mi_random_ctx_t* ctx, mi_random_ctx_t* new_ctx);
uintptr_t  _mi_random_next(mi_random_ctx_t* ctx);
uintptr_t  _mi_heap_random_next(mi_heap_t* heap);
uintptr_t  _os_random_weak(uintptr_t extra_seed);
static inline uintptr_t _mi_random_shuffle(uintptr_t x);

// init.c
extern mi_stats_t       _mi_stats_main;
extern const mi_page_t  _mi_page_empty;
bool       _mi_is_main_thread(void);
bool       _mi_preloading();  // true while the C runtime is not ready

// os.c
size_t     _mi_os_page_size(void);
void       _mi_os_init(void);                                      // called from process init
void*      _mi_os_alloc(size_t size, mi_stats_t* stats);           // to allocate thread local data
void       _mi_os_free(void* p, size_t size, mi_stats_t* stats);   // to free thread local data
size_t     _mi_os_good_alloc_size(size_t size);

// memory.c
void*      _mi_mem_alloc_aligned(size_t size, size_t alignment, bool* commit, bool* large, bool* is_zero, size_t* id, mi_os_tld_t* tld);
void       _mi_mem_free(void* p, size_t size, size_t id, bool fully_committed, bool any_reset, mi_os_tld_t* tld);

bool       _mi_mem_reset(void* p, size_t size, mi_os_tld_t* tld);
bool       _mi_mem_unreset(void* p, size_t size, bool* is_zero, mi_os_tld_t* tld);
bool       _mi_mem_commit(void* p, size_t size, bool* is_zero, mi_os_tld_t* tld);
bool       _mi_mem_protect(void* addr, size_t size);
bool       _mi_mem_unprotect(void* addr, size_t size);

void        _mi_mem_collect(mi_os_tld_t* tld);

// "segment.c"
mi_page_t* _mi_segment_page_alloc(mi_heap_t* heap, size_t block_wsize, mi_segments_tld_t* tld, mi_os_tld_t* os_tld);
void       _mi_segment_page_free(mi_page_t* page, bool force, mi_segments_tld_t* tld);
void       _mi_segment_page_abandon(mi_page_t* page, mi_segments_tld_t* tld);
uint8_t*   _mi_segment_page_start(const mi_segment_t* segment, const mi_page_t* page, size_t block_size, size_t* page_size, size_t* pre_size); // page start for any page
void       _mi_segment_huge_page_free(mi_segment_t* segment, mi_page_t* page, mi_block_t* block);

void       _mi_segment_thread_collect(mi_segments_tld_t* tld);
void       _mi_abandoned_reclaim_all(mi_heap_t* heap, mi_segments_tld_t* tld);
void       _mi_abandoned_await_readers(void);



// "page.c"
void*      _mi_malloc_generic(mi_heap_t* heap, size_t size)  mi_attr_noexcept mi_attr_malloc;

void       _mi_page_retire(mi_page_t* page);                                  // free the page if there are no other pages with many free blocks
void       _mi_page_unfull(mi_page_t* page);
void       _mi_page_free(mi_page_t* page, mi_page_queue_t* pq, bool force);   // free the page
void       _mi_page_abandon(mi_page_t* page, mi_page_queue_t* pq);            // abandon the page, to be picked up by another thread...
void       _mi_heap_delayed_free(mi_heap_t* heap);
void       _mi_heap_collect_retired(mi_heap_t* heap, bool force);

void       _mi_page_use_delayed_free(mi_page_t* page, mi_delayed_t delay, bool override_never);
size_t     _mi_page_queue_append(mi_heap_t* heap, mi_page_queue_t* pq, mi_page_queue_t* append);
void       _mi_deferred_free(mi_heap_t* heap, bool force);

void       _mi_page_free_collect(mi_page_t* page,bool force);
void       _mi_page_reclaim(mi_heap_t* heap, mi_page_t* page);   // callback from segments

size_t     _mi_bin_size(uint8_t bin);           // for stats
uint8_t    _mi_bin(size_t size);                // for stats
uint8_t    _mi_bsr(uintptr_t x);                // bit-scan-right, used on BSD in "os.c"

// "heap.c"
void       _mi_heap_destroy_pages(mi_heap_t* heap);
void       _mi_heap_collect_abandon(mi_heap_t* heap);
void       _mi_heap_set_default_direct(mi_heap_t* heap);

// "stats.c"
void       _mi_stats_done(mi_stats_t* stats);

mi_msecs_t  _mi_clock_now(void);
mi_msecs_t  _mi_clock_end(mi_msecs_t start);
mi_msecs_t  _mi_clock_start(void);

// "alloc.c"
void*       _mi_page_malloc(mi_heap_t* heap, mi_page_t* page, size_t size) mi_attr_noexcept;  // called from `_mi_malloc_generic`
void*       _mi_heap_malloc_zero(mi_heap_t* heap, size_t size, bool zero);
void*       _mi_heap_realloc_zero(mi_heap_t* heap, void* p, size_t newsize, bool zero);
mi_block_t* _mi_page_ptr_unalign(const mi_segment_t* segment, const mi_page_t* page, const void* p);
bool        _mi_free_delayed_block(mi_block_t* block);
void        _mi_block_zero_init(const mi_page_t* page, void* p, size_t size);

#if MI_DEBUG>1
bool        _mi_page_is_valid(mi_page_t* page);
#endif


// ------------------------------------------------------
// Branches
// ------------------------------------------------------

#if defined(__GNUC__) || defined(__clang__)
#define mi_unlikely(x)     __builtin_expect((x),0)
#define mi_likely(x)       __builtin_expect((x),1)
#else
#define mi_unlikely(x)     (x)
#define mi_likely(x)       (x)
#endif

#ifndef __has_builtin
#define __has_builtin(x)  0
#endif


/* -----------------------------------------------------------
  Error codes passed to `_mi_fatal_error`
  All are recoverable but EFAULT is a serious error and aborts by default in secure mode.
  For portability define undefined error codes using common Unix codes:
  <https://www-numi.fnal.gov/offline_software/srt_public_context/WebDocs/Errors/unix_system_errors.html>
----------------------------------------------------------- */
#include <errno.h>
#ifndef EAGAIN         // double free
#define EAGAIN (11)
#endif
#ifndef ENOMEM         // out of memory
#define ENOMEM (12)
#endif
#ifndef EFAULT         // corrupted free-list or meta-data
#define EFAULT (14)
#endif
#ifndef EINVAL         // trying to free an invalid pointer
#define EINVAL (22)
#endif
#ifndef EOVERFLOW      // count*size overflow
#define EOVERFLOW (75)
#endif


/* -----------------------------------------------------------
  Inlined definitions
----------------------------------------------------------- */
#define UNUSED(x)     (void)(x)
#if (MI_DEBUG>0)
#define UNUSED_RELEASE(x)
#else
#define UNUSED_RELEASE(x)  UNUSED(x)
#endif

#define MI_INIT4(x)   x(),x(),x(),x()
#define MI_INIT8(x)   MI_INIT4(x),MI_INIT4(x)
#define MI_INIT16(x)  MI_INIT8(x),MI_INIT8(x)
#define MI_INIT32(x)  MI_INIT16(x),MI_INIT16(x)
#define MI_INIT64(x)  MI_INIT32(x),MI_INIT32(x)
#define MI_INIT128(x) MI_INIT64(x),MI_INIT64(x)
#define MI_INIT256(x) MI_INIT128(x),MI_INIT128(x)


// Is `x` a power of two? (0 is considered a power of two)
static inline bool _mi_is_power_of_two(uintptr_t x) {
  return ((x & (x - 1)) == 0);
}

// Align upwards
static inline uintptr_t _mi_align_up(uintptr_t sz, size_t alignment) {
  mi_assert_internal(alignment != 0);
  uintptr_t mask = alignment - 1;
  if ((alignment & mask) == 0) {  // power of two?
    return ((sz + mask) & ~mask);
  }
  else {
    return (((sz + mask)/alignment)*alignment);
  }
}

// Divide upwards: `s <= _mi_divide_up(s,d)*d < s+d`.
static inline uintptr_t _mi_divide_up(uintptr_t size, size_t divider) {
  mi_assert_internal(divider != 0);
  return (divider == 0 ? size : ((size + divider - 1) / divider));
}

// Is memory zero initialized?
static inline bool mi_mem_is_zero(void* p, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (((uint8_t*)p)[i] != 0) return false;
  }
  return true;
}

// Align a byte size to a size in _machine words_,
// i.e. byte size == `wsize*sizeof(void*)`.
static inline size_t _mi_wsize_from_size(size_t size) {
  mi_assert_internal(size <= SIZE_MAX - sizeof(uintptr_t));
  return (size + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
}

// Does malloc satisfy the alignment constraints already?
static inline bool mi_malloc_satisfies_alignment(size_t alignment, size_t size) {
  return (alignment == sizeof(void*) || (alignment == MI_MAX_ALIGN_SIZE && size > (MI_MAX_ALIGN_SIZE/2)));
}

// Overflow detecting multiply
#if __has_builtin(__builtin_umul_overflow) || __GNUC__ >= 5
#include <limits.h>      // UINT_MAX, ULONG_MAX
#if defined(_CLOCK_T)    // for Illumos
#undef _CLOCK_T
#endif
static inline bool mi_mul_overflow(size_t count, size_t size, size_t* total) {
  #if (SIZE_MAX == UINT_MAX)
    return __builtin_umul_overflow(count, size, (unsigned int*)total);
  #elif (SIZE_MAX == ULONG_MAX)
    return __builtin_umull_overflow(count, size, total);
  #else
    return __builtin_umulll_overflow(count, size, total);
  #endif
}
#else /* __builtin_umul_overflow is unavailable */
static inline bool mi_mul_overflow(size_t count, size_t size, size_t* total) {
  #define MI_MUL_NO_OVERFLOW ((size_t)1 << (4*sizeof(size_t)))  // sqrt(SIZE_MAX)
  *total = count * size;
  return ((size >= MI_MUL_NO_OVERFLOW || count >= MI_MUL_NO_OVERFLOW)
    && size > 0 && (SIZE_MAX / size) < count);
}
#endif

// Safe multiply `count*size` into `total`; return `true` on overflow.
static inline bool mi_count_size_overflow(size_t count, size_t size, size_t* total) {
  if (count==1) {  // quick check for the case where count is one (common for C++ allocators)
    *total = size;
    return false;
  }
  else if (mi_unlikely(mi_mul_overflow(count, size, total))) {
    _mi_error_message(EOVERFLOW, "allocation request is too large (%zu * %zu bytes)\n", count, size);
    *total = SIZE_MAX;
    return true;
  }
  else return false;
}


/* ----------------------------------------------------------------------------------------
The thread local default heap: `_mi_get_default_heap` returns the thread local heap.
On most platforms (Windows, Linux, FreeBSD, NetBSD, etc), this just returns a
__thread local variable (`_mi_heap_default`). With the initial-exec TLS model this ensures
that the storage will always be available (allocated on the thread stacks).
On some platforms though we cannot use that when overriding `malloc` since the underlying
TLS implementation (or the loader) will call itself `malloc` on a first access and recurse.
We try to circumvent this in an efficient way:
- macOSX : we use an unused TLS slot from the OS allocated slots (MI_TLS_SLOT). On OSX, the
           loader itself calls `malloc` even before the modules are initialized.
- OpenBSD: we use an unused slot from the pthread block (MI_TLS_PTHREAD_SLOT_OFS).
- DragonFly: not yet working.
------------------------------------------------------------------------------------------- */

extern const mi_heap_t _mi_heap_empty;  // read-only empty heap, initial value of the thread local default heap
extern bool _mi_process_is_initialized;
mi_heap_t*  _mi_heap_main_get(void);    // statically allocated main backing heap

#if defined(MI_MALLOC_OVERRIDE)
#if defined(__MACH__) // OSX
// #define MI_TLS_SLOT               89  // seems unused?
#define MI_TLS_PTHREAD
// other possible unused ones are 9, 29, __PTK_FRAMEWORK_JAVASCRIPTCORE_KEY4 (94), __PTK_FRAMEWORK_GC_KEY9 (112) and __PTK_FRAMEWORK_OLDGC_KEY9 (89)
// see <https://github.com/rweichler/substrate/blob/master/include/pthread_machdep.h>
#elif defined(__OpenBSD__)
// use end bytes of a name; goes wrong if anyone uses names > 23 characters (ptrhread specifies 16) 
// see <https://github.com/openbsd/src/blob/master/lib/libc/include/thread_private.h#L371>
#define MI_TLS_PTHREAD_SLOT_OFS   (6*sizeof(int) + 4*sizeof(void*) + 24)  
#elif defined(__DragonFly__)
#warning "mimalloc is not working correctly on DragonFly yet."
#define MI_TLS_PTHREAD_SLOT_OFS   (4 + 1*sizeof(void*))  // offset `uniqueid` (also used by gdb?) <https://github.com/DragonFlyBSD/DragonFlyBSD/blob/master/lib/libthread_xu/thread/thr_private.h#L458>
#endif
#endif

#if defined(MI_TLS_SLOT)
static inline void* mi_tls_slot(size_t slot) mi_attr_noexcept;   // forward declaration
#elif defined(MI_TLS_PTHREAD_SLOT_OFS)
#include <pthread.h>
static inline mi_heap_t** mi_tls_pthread_heap_slot(void) {
  pthread_t self = pthread_self();
  #if defined(__DragonFly__)
  if (self==NULL) {
    static mi_heap_t* pheap_main = _mi_heap_main_get();
    return &pheap_main;
  }
  #endif
  return (mi_heap_t**)((uint8_t*)self + MI_TLS_PTHREAD_SLOT_OFS);
}
#elif defined(MI_TLS_PTHREAD)
#include <pthread.h>
extern pthread_key_t _mi_heap_default_key;
#else
extern mi_decl_thread mi_heap_t* _mi_heap_default;  // default heap to allocate from
#endif

static inline mi_heap_t* mi_get_default_heap(void) {
#if defined(MI_TLS_SLOT)
  mi_heap_t* heap = (mi_heap_t*)mi_tls_slot(MI_TLS_SLOT);
  return (mi_unlikely(heap == NULL) ? (mi_heap_t*)&_mi_heap_empty : heap);
#elif defined(MI_TLS_PTHREAD_SLOT_OFS)
  mi_heap_t* heap = *mi_tls_pthread_heap_slot();
  return (mi_unlikely(heap == NULL) ? (mi_heap_t*)&_mi_heap_empty : heap);
#elif defined(MI_TLS_PTHREAD)
  mi_heap_t* heap = (mi_unlikely(_mi_heap_default_key == (pthread_key_t)(-1)) ? _mi_heap_main_get() : (mi_heap_t*)pthread_getspecific(_mi_heap_default_key));
  return (mi_unlikely(heap == NULL) ? (mi_heap_t*)&_mi_heap_empty : heap);
#else
  #if defined(MI_TLS_RECURSE_GUARD)
  if (mi_unlikely(!_mi_process_is_initialized)) return _mi_heap_main_get();
  #endif
  return _mi_heap_default;
#endif
}

static inline bool mi_heap_is_default(const mi_heap_t* heap) {
  return (heap == mi_get_default_heap());
}

static inline bool mi_heap_is_backing(const mi_heap_t* heap) {
  return (heap->tld->heap_backing == heap);
}

static inline bool mi_heap_is_initialized(mi_heap_t* heap) {
  mi_assert_internal(heap != NULL);
  return (heap != &_mi_heap_empty);
}

static inline uintptr_t _mi_ptr_cookie(const void* p) {
  extern mi_heap_t _mi_heap_main;
  mi_assert_internal(_mi_heap_main.cookie != 0);
  return ((uintptr_t)p ^ _mi_heap_main.cookie);
}

/* -----------------------------------------------------------
  Pages
----------------------------------------------------------- */

static inline mi_page_t* _mi_heap_get_free_small_page(mi_heap_t* heap, size_t size) {
  mi_assert_internal(size <= (MI_SMALL_SIZE_MAX + MI_PADDING_SIZE));
  const size_t idx = _mi_wsize_from_size(size);
  mi_assert_internal(idx < MI_PAGES_DIRECT);
  return heap->pages_free_direct[idx];
}

// Get the page belonging to a certain size class
static inline mi_page_t* _mi_get_free_small_page(size_t size) {
  return _mi_heap_get_free_small_page(mi_get_default_heap(), size);
}

// Segment that contains the pointer
static inline mi_segment_t* _mi_ptr_segment(const void* p) {
  // mi_assert_internal(p != NULL);
  return (mi_segment_t*)((uintptr_t)p & ~MI_SEGMENT_MASK);
}

// Segment belonging to a page
static inline mi_segment_t* _mi_page_segment(const mi_page_t* page) {
  mi_segment_t* segment = _mi_ptr_segment(page);
  mi_assert_internal(segment == NULL || page == &segment->pages[page->segment_idx]);
  return segment;
}

// used internally
static inline uintptr_t _mi_segment_page_idx_of(const mi_segment_t* segment, const void* p) {
  // if (segment->page_size > MI_SEGMENT_SIZE) return &segment->pages[0];  // huge pages
  ptrdiff_t diff = (uint8_t*)p - (uint8_t*)segment;
  mi_assert_internal(diff >= 0 && (size_t)diff < MI_SEGMENT_SIZE);
  uintptr_t idx = (uintptr_t)diff >> segment->page_shift;
  mi_assert_internal(idx < segment->capacity);
  mi_assert_internal(segment->page_kind <= MI_PAGE_MEDIUM || idx == 0);
  return idx;
}

// Get the page containing the pointer
static inline mi_page_t* _mi_segment_page_of(const mi_segment_t* segment, const void* p) {
  uintptr_t idx = _mi_segment_page_idx_of(segment, p);
  return &((mi_segment_t*)segment)->pages[idx];
}

// Quick page start for initialized pages
static inline uint8_t* _mi_page_start(const mi_segment_t* segment, const mi_page_t* page, size_t* page_size) {
  const size_t bsize = page->xblock_size;
  mi_assert_internal(bsize > 0 && (bsize%sizeof(void*)) == 0);
  return _mi_segment_page_start(segment, page, bsize, page_size, NULL);
}

// Get the page containing the pointer
static inline mi_page_t* _mi_ptr_page(void* p) {
  return _mi_segment_page_of(_mi_ptr_segment(p), p);
}

// Get the block size of a page (special cased for huge objects)
static inline size_t mi_page_block_size(const mi_page_t* page) {
  const size_t bsize = page->xblock_size;
  mi_assert_internal(bsize > 0);
  if (mi_likely(bsize < MI_HUGE_BLOCK_SIZE)) {
    return bsize;
  }
  else {
    size_t psize;
    _mi_segment_page_start(_mi_page_segment(page), page, bsize, &psize, NULL);
    return psize;
  }
}

// Get the usable block size of a page without fixed padding.
// This may still include internal padding due to alignment and rounding up size classes.
static inline size_t mi_page_usable_block_size(const mi_page_t* page) {
  return mi_page_block_size(page) - MI_PADDING_SIZE;
}


// Thread free access
static inline mi_block_t* mi_page_thread_free(const mi_page_t* page) {
  return (mi_block_t*)(mi_atomic_read_relaxed(&page->xthread_free) & ~3);
}

static inline mi_delayed_t mi_page_thread_free_flag(const mi_page_t* page) {
  return (mi_delayed_t)(mi_atomic_read_relaxed(&page->xthread_free) & 3);
}

// Heap access
static inline mi_heap_t* mi_page_heap(const mi_page_t* page) {
  return (mi_heap_t*)(mi_atomic_read_relaxed(&page->xheap));
}

static inline void mi_page_set_heap(mi_page_t* page, mi_heap_t* heap) {
  mi_assert_internal(mi_page_thread_free_flag(page) != MI_DELAYED_FREEING);
  mi_atomic_write(&page->xheap,(uintptr_t)heap);
}

// Thread free flag helpers
static inline mi_block_t* mi_tf_block(mi_thread_free_t tf) {
  return (mi_block_t*)(tf & ~0x03);
}
static inline mi_delayed_t mi_tf_delayed(mi_thread_free_t tf) {
  return (mi_delayed_t)(tf & 0x03);
}
static inline mi_thread_free_t mi_tf_make(mi_block_t* block, mi_delayed_t delayed) {
  return (mi_thread_free_t)((uintptr_t)block | (uintptr_t)delayed);
}
static inline mi_thread_free_t mi_tf_set_delayed(mi_thread_free_t tf, mi_delayed_t delayed) {
  return mi_tf_make(mi_tf_block(tf),delayed);
}
static inline mi_thread_free_t mi_tf_set_block(mi_thread_free_t tf, mi_block_t* block) {
  return mi_tf_make(block, mi_tf_delayed(tf));
}

// are all blocks in a page freed?
// note: needs up-to-date used count, (as the `xthread_free` list may not be empty). see `_mi_page_collect_free`.
static inline bool mi_page_all_free(const mi_page_t* page) {
  mi_assert_internal(page != NULL);
  return (page->used == 0);
}

// are there any available blocks?
static inline bool mi_page_has_any_available(const mi_page_t* page) {
  mi_assert_internal(page != NULL && page->reserved > 0);
  return (page->used < page->reserved || (mi_page_thread_free(page) != NULL));
}

// are there immediately available blocks, i.e. blocks available on the free list.
static inline bool mi_page_immediate_available(const mi_page_t* page) {
  mi_assert_internal(page != NULL);
  return (page->free != NULL);
}

// is more than 7/8th of a page in use?
static inline bool mi_page_mostly_used(const mi_page_t* page) {
  if (page==NULL) return true;
  uint16_t frac = page->reserved / 8U;
  return (page->reserved - page->used <= frac);
}

static inline mi_page_queue_t* mi_page_queue(const mi_heap_t* heap, size_t size) {
  return &((mi_heap_t*)heap)->pages[_mi_bin(size)];
}



//-----------------------------------------------------------
// Page flags
//-----------------------------------------------------------
static inline bool mi_page_is_in_full(const mi_page_t* page) {
  return page->flags.x.in_full;
}

static inline void mi_page_set_in_full(mi_page_t* page, bool in_full) {
  page->flags.x.in_full = in_full;
}

static inline bool mi_page_has_aligned(const mi_page_t* page) {
  return page->flags.x.has_aligned;
}

static inline void mi_page_set_has_aligned(mi_page_t* page, bool has_aligned) {
  page->flags.x.has_aligned = has_aligned;
}


/* -------------------------------------------------------------------
Encoding/Decoding the free list next pointers

This is to protect against buffer overflow exploits where the
free list is mutated. Many hardened allocators xor the next pointer `p`
with a secret key `k1`, as `p^k1`. This prevents overwriting with known
values but might be still too weak: if the attacker can guess
the pointer `p` this  can reveal `k1` (since `p^k1^p == k1`).
Moreover, if multiple blocks can be read as well, the attacker can
xor both as `(p1^k1) ^ (p2^k1) == p1^p2` which may reveal a lot
about the pointers (and subsequently `k1`).

Instead mimalloc uses an extra key `k2` and encodes as `((p^k2)<<<k1)+k1`.
Since these operations are not associative, the above approaches do not
work so well any more even if the `p` can be guesstimated. For example,
for the read case we can subtract two entries to discard the `+k1` term,
but that leads to `((p1^k2)<<<k1) - ((p2^k2)<<<k1)` at best.
We include the left-rotation since xor and addition are otherwise linear
in the lowest bit. Finally, both keys are unique per page which reduces
the re-use of keys by a large factor.

We also pass a separate `null` value to be used as `NULL` or otherwise
`(k2<<<k1)+k1` would appear (too) often as a sentinel value.
------------------------------------------------------------------- */

static inline bool mi_is_in_same_segment(const void* p, const void* q) {
  return (_mi_ptr_segment(p) == _mi_ptr_segment(q));
}

static inline bool mi_is_in_same_page(const void* p, const void* q) {
  mi_segment_t* segmentp = _mi_ptr_segment(p);
  mi_segment_t* segmentq = _mi_ptr_segment(q);
  if (segmentp != segmentq) return false;
  uintptr_t idxp = _mi_segment_page_idx_of(segmentp, p);
  uintptr_t idxq = _mi_segment_page_idx_of(segmentq, q);
  return (idxp == idxq);
}

static inline uintptr_t mi_rotl(uintptr_t x, uintptr_t shift) {
  shift %= MI_INTPTR_BITS;
  return (shift==0 ? x : ((x << shift) | (x >> (MI_INTPTR_BITS - shift))));
}
static inline uintptr_t mi_rotr(uintptr_t x, uintptr_t shift) {
  shift %= MI_INTPTR_BITS;
  return (shift==0 ? x : ((x >> shift) | (x << (MI_INTPTR_BITS - shift))));
}

static inline void* mi_ptr_decode(const void* null, const mi_encoded_t x, const uintptr_t* keys) {
  void* p = (void*)(mi_rotr(x - keys[0], keys[0]) ^ keys[1]);
  return (mi_unlikely(p==null) ? NULL : p);
}

static inline mi_encoded_t mi_ptr_encode(const void* null, const void* p, const uintptr_t* keys) {
  uintptr_t x = (uintptr_t)(mi_unlikely(p==NULL) ? null : p);
  return mi_rotl(x ^ keys[1], keys[0]) + keys[0];
}

static inline mi_block_t* mi_block_nextx( const void* null, const mi_block_t* block, const uintptr_t* keys ) {
  #ifdef MI_ENCODE_FREELIST
  return (mi_block_t*)mi_ptr_decode(null, block->next, keys);
  #else
  UNUSED(keys); UNUSED(null);
  return (mi_block_t*)block->next;
  #endif
}

static inline void mi_block_set_nextx(const void* null, mi_block_t* block, const mi_block_t* next, const uintptr_t* keys) {
  #ifdef MI_ENCODE_FREELIST
  block->next = mi_ptr_encode(null, next, keys);
  #else
  UNUSED(keys); UNUSED(null);
  block->next = (mi_encoded_t)next;
  #endif
}

static inline mi_block_t* mi_block_next(const mi_page_t* page, const mi_block_t* block) {
  #ifdef MI_ENCODE_FREELIST
  mi_block_t* next = mi_block_nextx(page,block,page->keys);
  // check for free list corruption: is `next` at least in the same page?
  // TODO: check if `next` is `page->block_size` aligned?
  if (mi_unlikely(next!=NULL && !mi_is_in_same_page(block, next))) {
    _mi_error_message(EFAULT, "corrupted free list entry of size %zub at %p: value 0x%zx\n", mi_page_block_size(page), block, (uintptr_t)next);
    next = NULL;
  }
  return next;
  #else
  UNUSED(page);
  return mi_block_nextx(page,block,NULL);
  #endif
}

static inline void mi_block_set_next(const mi_page_t* page, mi_block_t* block, const mi_block_t* next) {
  #ifdef MI_ENCODE_FREELIST
  mi_block_set_nextx(page,block,next, page->keys);
  #else
  UNUSED(page);
  mi_block_set_nextx(page,block,next,NULL);
  #endif
}

// -------------------------------------------------------------------
// Fast "random" shuffle
// -------------------------------------------------------------------

static inline uintptr_t _mi_random_shuffle(uintptr_t x) {
  if (x==0) { x = 17; }   // ensure we don't get stuck in generating zeros
#if (MI_INTPTR_SIZE==8)
  // by Sebastiano Vigna, see: <http://xoshiro.di.unimi.it/splitmix64.c>
  x ^= x >> 30;
  x *= 0xbf58476d1ce4e5b9UL;
  x ^= x >> 27;
  x *= 0x94d049bb133111ebUL;
  x ^= x >> 31;
#elif (MI_INTPTR_SIZE==4)
  // by Chris Wellons, see: <https://nullprogram.com/blog/2018/07/31/>
  x ^= x >> 16;
  x *= 0x7feb352dUL;
  x ^= x >> 15;
  x *= 0x846ca68bUL;
  x ^= x >> 16;
#endif
  return x;
}

// -------------------------------------------------------------------
// Optimize numa node access for the common case (= one node)
// -------------------------------------------------------------------

int    _mi_os_numa_node_get(mi_os_tld_t* tld);
size_t _mi_os_numa_node_count_get(void);

extern size_t _mi_numa_node_count;
static inline int _mi_os_numa_node(mi_os_tld_t* tld) {
  if (mi_likely(_mi_numa_node_count == 1)) return 0;
  else return _mi_os_numa_node_get(tld);
}
static inline size_t _mi_os_numa_node_count(void) {
  if (mi_likely(_mi_numa_node_count>0)) return _mi_numa_node_count;
  else return _mi_os_numa_node_count_get();
}


// -------------------------------------------------------------------
// Getting the thread id should be performant as it is called in the
// fast path of `_mi_free` and we specialize for various platforms.
// -------------------------------------------------------------------
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static inline uintptr_t _mi_thread_id(void) mi_attr_noexcept {
  // Windows: works on Intel and ARM in both 32- and 64-bit
  return (uintptr_t)NtCurrentTeb();
}

#elif defined(__GNUC__) && !defined(__MACH__) && \
      (defined(__x86_64__) || defined(__i386__) || defined(__arm__) || defined(__aarch64__))

// TLS register on x86 is in the FS or GS register, see: https://akkadia.org/drepper/tls.pdf
static inline void* mi_tls_slot(size_t slot) mi_attr_noexcept {
  void* res;
  const size_t ofs = (slot*sizeof(void*));
#if defined(__i386__)
  __asm__("movl %%gs:%1, %0" : "=r" (res) : "m" (*((void**)ofs)) : );  // 32-bit always uses GS
#elif defined(__MACH__) && defined(__x86_64__)
  __asm__("movq %%gs:%1, %0" : "=r" (res) : "m" (*((void**)ofs)) : );  // x86_64 macOSX uses GS
#elif defined(__x86_64__)
  __asm__("movq %%fs:%1, %0" : "=r" (res) : "m" (*((void**)ofs)) : );  // x86_64 Linux, BSD uses FS
#elif defined(__arm__)
  void** tcb; UNUSED(ofs);
  asm volatile ("mrc p15, 0, %0, c13, c0, 3\nbic %0, %0, #3" : "=r" (tcb));
  res = tcb[slot];
#elif defined(__aarch64__)
  void** tcb; UNUSED(ofs);
  asm volatile ("mrs %0, tpidr_el0" : "=r" (tcb));
  res = tcb[slot];
#endif
  return res;
}

// setting is only used on macOSX for now
static inline void mi_tls_slot_set(size_t slot, void* value) mi_attr_noexcept {
  const size_t ofs = (slot*sizeof(void*));
#if defined(__i386__)
  __asm__("movl %1,%%gs:%0" : "=m" (*((void**)ofs)) : "rn" (value) : );  // 32-bit always uses GS
#elif defined(__MACH__) && defined(__x86_64__)
  __asm__("movq %1,%%gs:%0" : "=m" (*((void**)ofs)) : "rn" (value) : );  // x86_64 macOSX uses GS
#elif defined(__x86_64__)
  __asm__("movq %1,%%fs:%1" : "=m" (*((void**)ofs)) : "rn" (value) : );  // x86_64 Linux, BSD uses FS
#elif defined(__arm__)
  void** tcb; UNUSED(ofs);
  asm volatile ("mrc p15, 0, %0, c13, c0, 3\nbic %0, %0, #3" : "=r" (tcb));
  tcb[slot] = value;
#elif defined(__aarch64__)
  void** tcb; UNUSED(ofs);
  asm volatile ("mrs %0, tpidr_el0" : "=r" (tcb));
  tcb[slot] = value;
#endif
}

static inline uintptr_t _mi_thread_id(void) mi_attr_noexcept {
  // in all our targets, slot 0 is the pointer to the thread control block
  return (uintptr_t)mi_tls_slot(0);
}
#else
// otherwise use standard C
static inline uintptr_t _mi_thread_id(void) mi_attr_noexcept {
  return (uintptr_t)&_mi_heap_default;
}
#endif


#endif
/**** ended inlining mimalloc-internal.h ****/

// For a static override we create a single object file
// containing the whole library. If it is linked first
// it will override all the standard library allocation
// functions (on Unix's).
/**** start inlining stats.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/
/**** skipping file: mimalloc-atomic.h ****/

#include <stdio.h>  // fputs, stderr
#include <string.h> // memset


/* -----------------------------------------------------------
  Statistics operations
----------------------------------------------------------- */

static bool mi_is_in_main(void* stat) {
  return ((uint8_t*)stat >= (uint8_t*)&_mi_stats_main
         && (uint8_t*)stat < ((uint8_t*)&_mi_stats_main + sizeof(mi_stats_t)));  
}

static void mi_stat_update(mi_stat_count_t* stat, int64_t amount) {
  if (amount == 0) return;
  if (mi_is_in_main(stat))
  {
    // add atomically (for abandoned pages)
    mi_atomic_addi64(&stat->current,amount);
    mi_atomic_maxi64(&stat->peak, mi_atomic_readi64(&stat->current));
    if (amount > 0) {
      mi_atomic_addi64(&stat->allocated,amount);
    }
    else {
      mi_atomic_addi64(&stat->freed, -amount);
    }
  }
  else {
    // add thread local
    stat->current += amount;
    if (stat->current > stat->peak) stat->peak = stat->current;
    if (amount > 0) {
      stat->allocated += amount;
    }
    else {
      stat->freed += -amount;
    }
  }
}

void _mi_stat_counter_increase(mi_stat_counter_t* stat, size_t amount) {  
  if (mi_is_in_main(stat)) {
    mi_atomic_addi64( &stat->count, 1 );
    mi_atomic_addi64( &stat->total, (int64_t)amount );
  }
  else {
    stat->count++;
    stat->total += amount;
  }
}

void _mi_stat_increase(mi_stat_count_t* stat, size_t amount) {
  mi_stat_update(stat, (int64_t)amount);
}

void _mi_stat_decrease(mi_stat_count_t* stat, size_t amount) {
  mi_stat_update(stat, -((int64_t)amount));
}

// must be thread safe as it is called from stats_merge
static void mi_stat_add(mi_stat_count_t* stat, const mi_stat_count_t* src, int64_t unit) {
  if (stat==src) return;
  if (src->allocated==0 && src->freed==0) return;
  mi_atomic_addi64( &stat->allocated, src->allocated * unit);
  mi_atomic_addi64( &stat->current, src->current * unit);
  mi_atomic_addi64( &stat->freed, src->freed * unit);
  // peak scores do not work across threads..
  mi_atomic_addi64( &stat->peak, src->peak * unit);
}

static void mi_stat_counter_add(mi_stat_counter_t* stat, const mi_stat_counter_t* src, int64_t unit) {
  if (stat==src) return;
  mi_atomic_addi64( &stat->total, src->total * unit);
  mi_atomic_addi64( &stat->count, src->count * unit);
}

// must be thread safe as it is called from stats_merge
static void mi_stats_add(mi_stats_t* stats, const mi_stats_t* src) {
  if (stats==src) return;
  mi_stat_add(&stats->segments, &src->segments,1);
  mi_stat_add(&stats->pages, &src->pages,1);
  mi_stat_add(&stats->reserved, &src->reserved, 1);
  mi_stat_add(&stats->committed, &src->committed, 1);
  mi_stat_add(&stats->reset, &src->reset, 1);
  mi_stat_add(&stats->page_committed, &src->page_committed, 1);

  mi_stat_add(&stats->pages_abandoned, &src->pages_abandoned, 1);
  mi_stat_add(&stats->segments_abandoned, &src->segments_abandoned, 1);
  mi_stat_add(&stats->threads, &src->threads, 1);

  mi_stat_add(&stats->malloc, &src->malloc, 1);
  mi_stat_add(&stats->segments_cache, &src->segments_cache, 1);
  mi_stat_add(&stats->huge, &src->huge, 1);
  mi_stat_add(&stats->giant, &src->giant, 1);

  mi_stat_counter_add(&stats->pages_extended, &src->pages_extended, 1);
  mi_stat_counter_add(&stats->mmap_calls, &src->mmap_calls, 1);
  mi_stat_counter_add(&stats->commit_calls, &src->commit_calls, 1);

  mi_stat_counter_add(&stats->page_no_retire, &src->page_no_retire, 1);
  mi_stat_counter_add(&stats->searches, &src->searches, 1);
  mi_stat_counter_add(&stats->huge_count, &src->huge_count, 1);
  mi_stat_counter_add(&stats->giant_count, &src->giant_count, 1);
#if MI_STAT>1
  for (size_t i = 0; i <= MI_BIN_HUGE; i++) {
    if (src->normal[i].allocated > 0 || src->normal[i].freed > 0) {
      mi_stat_add(&stats->normal[i], &src->normal[i], 1);
    }
  }
#endif
}

/* -----------------------------------------------------------
  Display statistics
----------------------------------------------------------- */

// unit > 0 : size in binary bytes 
// unit == 0: count as decimal
// unit < 0 : count in binary
static void mi_printf_amount(int64_t n, int64_t unit, mi_output_fun* out, void* arg, const char* fmt) {
  char buf[32];
  int  len = 32;
  const char* suffix = (unit <= 0 ? " " : "b");
  const int64_t base = (unit == 0 ? 1000 : 1024);
  if (unit>0) n *= unit;

  const int64_t pos = (n < 0 ? -n : n);
  if (pos < base) {
    snprintf(buf, len, "%d %s ", (int)n, suffix);
  }
  else {
    int64_t divider = base;
    const char* magnitude = "k";
    if (pos >= divider*base) { divider *= base; magnitude = "m"; }
    if (pos >= divider*base) { divider *= base; magnitude = "g"; }
    const int64_t tens = (n / (divider/10));
    const long whole = (long)(tens/10);
    const long frac1 = (long)(tens%10);
    snprintf(buf, len, "%ld.%ld %s%s", whole, frac1, magnitude, suffix);
  }
  _mi_fprintf(out, arg, (fmt==NULL ? "%11s" : fmt), buf);
}


static void mi_print_amount(int64_t n, int64_t unit, mi_output_fun* out, void* arg) {
  mi_printf_amount(n,unit,out,arg,NULL);
}

static void mi_print_count(int64_t n, int64_t unit, mi_output_fun* out, void* arg) {
  if (unit==1) _mi_fprintf(out, arg, "%11s"," ");
          else mi_print_amount(n,0,out,arg);
}

static void mi_stat_print(const mi_stat_count_t* stat, const char* msg, int64_t unit, mi_output_fun* out, void* arg ) {
  _mi_fprintf(out, arg,"%10s:", msg);
  if (unit>0) {
    mi_print_amount(stat->peak, unit, out, arg);
    mi_print_amount(stat->allocated, unit, out, arg);
    mi_print_amount(stat->freed, unit, out, arg);
    mi_print_amount(unit, 1, out, arg);
    mi_print_count(stat->allocated, unit, out, arg);
    if (stat->allocated > stat->freed)
      _mi_fprintf(out, arg, "  not all freed!\n");
    else
      _mi_fprintf(out, arg, "  ok\n");
  }
  else if (unit<0) {
    mi_print_amount(stat->peak, -1, out, arg);
    mi_print_amount(stat->allocated, -1, out, arg);
    mi_print_amount(stat->freed, -1, out, arg);
    if (unit==-1) {
      _mi_fprintf(out, arg, "%22s", "");
    }
    else {
      mi_print_amount(-unit, 1, out, arg);
      mi_print_count((stat->allocated / -unit), 0, out, arg);
    }
    if (stat->allocated > stat->freed)
      _mi_fprintf(out, arg, "  not all freed!\n");
    else
      _mi_fprintf(out, arg, "  ok\n");
  }
  else {
    mi_print_amount(stat->peak, 1, out, arg);
    mi_print_amount(stat->allocated, 1, out, arg);
    _mi_fprintf(out, arg, "\n");
  }
}

static void mi_stat_counter_print(const mi_stat_counter_t* stat, const char* msg, mi_output_fun* out, void* arg ) {
  _mi_fprintf(out, arg, "%10s:", msg);
  mi_print_amount(stat->total, -1, out, arg);
  _mi_fprintf(out, arg, "\n");
}

static void mi_stat_counter_print_avg(const mi_stat_counter_t* stat, const char* msg, mi_output_fun* out, void* arg) {
  const int64_t avg_tens = (stat->count == 0 ? 0 : (stat->total*10 / stat->count)); 
  const long avg_whole = (long)(avg_tens/10);
  const long avg_frac1 = (long)(avg_tens%10);
  _mi_fprintf(out, arg, "%10s: %5ld.%ld avg\n", msg, avg_whole, avg_frac1);
}


static void mi_print_header(mi_output_fun* out, void* arg ) {
  _mi_fprintf(out, arg, "%10s: %10s %10s %10s %10s %10s\n", "heap stats", "peak  ", "total  ", "freed  ", "unit  ", "count  ");
}

#if MI_STAT>1
static void mi_stats_print_bins(mi_stat_count_t* all, const mi_stat_count_t* bins, size_t max, const char* fmt, mi_output_fun* out, void* arg) {
  bool found = false;
  char buf[64];
  for (size_t i = 0; i <= max; i++) {
    if (bins[i].allocated > 0) {
      found = true;
      int64_t unit = _mi_bin_size((uint8_t)i);
      snprintf(buf, 64, "%s %3zu", fmt, i);
      mi_stat_add(all, &bins[i], unit);
      mi_stat_print(&bins[i], buf, unit, out, arg);
    }
  }
  //snprintf(buf, 64, "%s all", fmt);
  //mi_stat_print(all, buf, 1);
  if (found) {
    _mi_fprintf(out, arg, "\n");
    mi_print_header(out, arg);
  }
}
#endif



//------------------------------------------------------------
// Use an output wrapper for line-buffered output
// (which is nice when using loggers etc.)
//------------------------------------------------------------
typedef struct buffered_s {
  mi_output_fun* out;   // original output function
  void*          arg;   // and state
  char*          buf;   // local buffer of at least size `count+1`
  size_t         used;  // currently used chars `used <= count`  
  size_t         count; // total chars available for output
} buffered_t;

static void mi_buffered_flush(buffered_t* buf) {
  buf->buf[buf->used] = 0;
  _mi_fputs(buf->out, buf->arg, NULL, buf->buf);
  buf->used = 0;
}

static void mi_buffered_out(const char* msg, void* arg) {
  buffered_t* buf = (buffered_t*)arg;
  if (msg==NULL || buf==NULL) return;
  for (const char* src = msg; *src != 0; src++) {
    char c = *src;
    if (buf->used >= buf->count) mi_buffered_flush(buf);
    mi_assert_internal(buf->used < buf->count);
    buf->buf[buf->used++] = c;
    if (c == '\n') mi_buffered_flush(buf);
  }
}

//------------------------------------------------------------
// Print statistics
//------------------------------------------------------------

static void mi_process_info(mi_msecs_t* utime, mi_msecs_t* stime, size_t* peak_rss, size_t* page_faults, size_t* page_reclaim, size_t* peak_commit);

static void _mi_stats_print(mi_stats_t* stats, mi_msecs_t elapsed, mi_output_fun* out0, void* arg0) mi_attr_noexcept {
  // wrap the output function to be line buffered
  char buf[256];
  buffered_t buffer = { out0, arg0, buf, 0, 255 };
  mi_output_fun* out = &mi_buffered_out;
  void* arg = &buffer;

  // and print using that
  mi_print_header(out,arg);
  #if MI_STAT>1
  mi_stat_count_t normal = { 0,0,0,0 };
  mi_stats_print_bins(&normal, stats->normal, MI_BIN_HUGE, "normal",out,arg);
  mi_stat_print(&normal, "normal", 1, out, arg);
  mi_stat_print(&stats->huge, "huge", (stats->huge_count.count == 0 ? 1 : -(stats->huge.allocated / stats->huge_count.count)), out, arg);
  mi_stat_print(&stats->giant, "giant", (stats->giant_count.count == 0 ? 1 : -(stats->giant.allocated / stats->giant_count.count)), out, arg);
  mi_stat_count_t total = { 0,0,0,0 };
  mi_stat_add(&total, &normal, 1);
  mi_stat_add(&total, &stats->huge, 1);
  mi_stat_add(&total, &stats->giant, 1);
  mi_stat_print(&total, "total", 1, out, arg);
  _mi_fprintf(out, arg, "malloc requested:     ");
  mi_print_amount(stats->malloc.allocated, 1, out, arg);
  _mi_fprintf(out, arg, "\n\n");
  #endif
  mi_stat_print(&stats->reserved, "reserved", 1, out, arg);
  mi_stat_print(&stats->committed, "committed", 1, out, arg);
  mi_stat_print(&stats->reset, "reset", 1, out, arg);
  mi_stat_print(&stats->page_committed, "touched", 1, out, arg);
  mi_stat_print(&stats->segments, "segments", -1, out, arg);
  mi_stat_print(&stats->segments_abandoned, "-abandoned", -1, out, arg);
  mi_stat_print(&stats->segments_cache, "-cached", -1, out, arg);
  mi_stat_print(&stats->pages, "pages", -1, out, arg);
  mi_stat_print(&stats->pages_abandoned, "-abandoned", -1, out, arg);
  mi_stat_counter_print(&stats->pages_extended, "-extended", out, arg);
  mi_stat_counter_print(&stats->page_no_retire, "-noretire", out, arg);
  mi_stat_counter_print(&stats->mmap_calls, "mmaps", out, arg);
  mi_stat_counter_print(&stats->commit_calls, "commits", out, arg);
  mi_stat_print(&stats->threads, "threads", -1, out, arg);
  mi_stat_counter_print_avg(&stats->searches, "searches", out, arg);
  _mi_fprintf(out, arg, "%10s: %7i\n", "numa nodes", _mi_os_numa_node_count());
  if (elapsed > 0) _mi_fprintf(out, arg, "%10s: %7ld.%03ld s\n", "elapsed", elapsed/1000, elapsed%1000);

  mi_msecs_t user_time;
  mi_msecs_t sys_time;
  size_t peak_rss;
  size_t page_faults;
  size_t page_reclaim;
  size_t peak_commit;
  mi_process_info(&user_time, &sys_time, &peak_rss, &page_faults, &page_reclaim, &peak_commit);
  _mi_fprintf(out, arg, "%10s: user: %ld.%03ld s, system: %ld.%03ld s, faults: %lu, reclaims: %lu, rss: ", "process", user_time/1000, user_time%1000, sys_time/1000, sys_time%1000, (unsigned long)page_faults, (unsigned long)page_reclaim );
  mi_printf_amount((int64_t)peak_rss, 1, out, arg, "%s");
  if (peak_commit > 0) {
    _mi_fprintf(out, arg, ", commit charge: ");
    mi_printf_amount((int64_t)peak_commit, 1, out, arg, "%s");
  }
  _mi_fprintf(out, arg, "\n");  
}

static mi_msecs_t mi_time_start; // = 0

static mi_stats_t* mi_stats_get_default(void) {
  mi_heap_t* heap = mi_heap_get_default();
  return &heap->tld->stats;
}

static void mi_stats_merge_from(mi_stats_t* stats) {
  if (stats != &_mi_stats_main) {
    mi_stats_add(&_mi_stats_main, stats);
    memset(stats, 0, sizeof(mi_stats_t));
  }
}

void mi_stats_reset(void) mi_attr_noexcept {
  mi_stats_t* stats = mi_stats_get_default();
  if (stats != &_mi_stats_main) { memset(stats, 0, sizeof(mi_stats_t)); }
  memset(&_mi_stats_main, 0, sizeof(mi_stats_t));
  mi_time_start = _mi_clock_start();
}

void mi_stats_merge(void) mi_attr_noexcept {
  mi_stats_merge_from( mi_stats_get_default() );
}

void _mi_stats_done(mi_stats_t* stats) {  // called from `mi_thread_done`
  mi_stats_merge_from(stats);
}

void mi_stats_print_out(mi_output_fun* out, void* arg) mi_attr_noexcept {
  mi_msecs_t elapsed = _mi_clock_end(mi_time_start);
  mi_stats_merge_from(mi_stats_get_default());
  _mi_stats_print(&_mi_stats_main, elapsed, out, arg);
}

void mi_stats_print(void* out) mi_attr_noexcept {
  // for compatibility there is an `out` parameter (which can be `stdout` or `stderr`)
  mi_stats_print_out((mi_output_fun*)out, NULL);
}

void mi_thread_stats_print_out(mi_output_fun* out, void* arg) mi_attr_noexcept {
  mi_msecs_t elapsed = _mi_clock_end(mi_time_start);
  _mi_stats_print(mi_stats_get_default(), elapsed, out, arg);
}


// ----------------------------------------------------------------
// Basic timer for convenience; use milli-seconds to avoid doubles
// ----------------------------------------------------------------
#ifdef _WIN32
#include <windows.h>
static mi_msecs_t mi_to_msecs(LARGE_INTEGER t) {
  static LARGE_INTEGER mfreq; // = 0
  if (mfreq.QuadPart == 0LL) {
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    mfreq.QuadPart = f.QuadPart/1000LL;
    if (mfreq.QuadPart == 0) mfreq.QuadPart = 1;
  }
  return (mi_msecs_t)(t.QuadPart / mfreq.QuadPart);  
}

mi_msecs_t _mi_clock_now(void) {
  LARGE_INTEGER t;
  QueryPerformanceCounter(&t);
  return mi_to_msecs(t);
}
#else
#include <time.h>
#ifdef CLOCK_REALTIME
mi_msecs_t _mi_clock_now(void) {
  struct timespec t;
  clock_gettime(CLOCK_REALTIME, &t);
  return ((mi_msecs_t)t.tv_sec * 1000) + ((mi_msecs_t)t.tv_nsec / 1000000);
}
#else
// low resolution timer
mi_msecs_t _mi_clock_now(void) {
  return ((mi_msecs_t)clock() / ((mi_msecs_t)CLOCKS_PER_SEC / 1000));
}
#endif
#endif


static mi_msecs_t mi_clock_diff;

mi_msecs_t _mi_clock_start(void) {
  if (mi_clock_diff == 0.0) {
    mi_msecs_t t0 = _mi_clock_now();
    mi_clock_diff = _mi_clock_now() - t0;
  }
  return _mi_clock_now();
}

mi_msecs_t _mi_clock_end(mi_msecs_t start) {
  mi_msecs_t end = _mi_clock_now();
  return (end - start - mi_clock_diff);
}


// --------------------------------------------------------
// Basic process statistics
// --------------------------------------------------------

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#pragma comment(lib,"psapi.lib")

static mi_msecs_t filetime_msecs(const FILETIME* ftime) {
  ULARGE_INTEGER i;
  i.LowPart = ftime->dwLowDateTime;
  i.HighPart = ftime->dwHighDateTime;
  mi_msecs_t msecs = (i.QuadPart / 10000); // FILETIME is in 100 nano seconds
  return msecs;
}
static void mi_process_info(mi_msecs_t* utime, mi_msecs_t* stime, size_t* peak_rss, size_t* page_faults, size_t* page_reclaim, size_t* peak_commit) {
  FILETIME ct;
  FILETIME ut;
  FILETIME st;
  FILETIME et;
  GetProcessTimes(GetCurrentProcess(), &ct, &et, &st, &ut);
  *utime = filetime_msecs(&ut);
  *stime = filetime_msecs(&st);

  PROCESS_MEMORY_COUNTERS info;
  GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
  *peak_rss = (size_t)info.PeakWorkingSetSize;
  *page_faults = (size_t)info.PageFaultCount;
  *peak_commit = (size_t)info.PeakPagefileUsage;
  *page_reclaim = 0;
}

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__)) || defined(__HAIKU__)
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>
#endif

#if defined(__HAIKU__)
#include <kernel/OS.h>
#endif

static mi_msecs_t timeval_secs(const struct timeval* tv) {
  return ((mi_msecs_t)tv->tv_sec * 1000L) + ((mi_msecs_t)tv->tv_usec / 1000L);
}

static void mi_process_info(mi_msecs_t* utime, mi_msecs_t* stime, size_t* peak_rss, size_t* page_faults, size_t* page_reclaim, size_t* peak_commit) {
  struct rusage rusage;
  getrusage(RUSAGE_SELF, &rusage);
#if !defined(__HAIKU__)
#if defined(__APPLE__) && defined(__MACH__)
  *peak_rss = rusage.ru_maxrss;
#else
  *peak_rss = rusage.ru_maxrss * 1024;
#endif
  *page_faults = rusage.ru_majflt;
  *page_reclaim = rusage.ru_minflt;
  *peak_commit = 0;
#else
// Haiku does not have (yet?) a way to
// get these stats per process
  thread_info tid;
  area_info mem;
  ssize_t c;
  *peak_rss = 0;
  *page_faults = 0;
  *page_reclaim = 0;
  *peak_commit = 0;
  get_thread_info(find_thread(0), &tid);

  while (get_next_area_info(tid.team, &c, &mem) == B_OK) {
      *peak_rss += mem.ram_size;
  }
#endif
  *utime = timeval_secs(&rusage.ru_utime);
  *stime = timeval_secs(&rusage.ru_stime);
}

#else
#ifndef __wasi__
// WebAssembly instances are not processes
#pragma message("define a way to get process info")
#endif

static void mi_process_info(mi_msecs_t* utime, mi_msecs_t* stime, size_t* peak_rss, size_t* page_faults, size_t* page_reclaim, size_t* peak_commit) {
  *peak_rss = 0;
  *page_faults = 0;
  *page_reclaim = 0;
  *peak_commit = 0;
  *utime = 0;
  *stime = 0;
}
#endif
/**** ended inlining stats.c ****/
/**** start inlining random.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2019, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/

#include <string.h> // memset

/* ----------------------------------------------------------------------------
We use our own PRNG to keep predictable performance of random number generation
and to avoid implementations that use a lock. We only use the OS provided
random source to initialize the initial seeds. Since we do not need ultimate
performance but we do rely on the security (for secret cookies in secure mode)
we use a cryptographically secure generator (chacha20).
-----------------------------------------------------------------------------*/

#define MI_CHACHA_ROUNDS (20)   // perhaps use 12 for better performance?


/* ----------------------------------------------------------------------------
Chacha20 implementation as the original algorithm with a 64-bit nonce
and counter: https://en.wikipedia.org/wiki/Salsa20
The input matrix has sixteen 32-bit values:
Position  0 to  3: constant key
Position  4 to 11: the key
Position 12 to 13: the counter.
Position 14 to 15: the nonce.

The implementation uses regular C code which compiles very well on modern compilers.
(gcc x64 has no register spills, and clang 6+ uses SSE instructions)
-----------------------------------------------------------------------------*/

static inline uint32_t rotl(uint32_t x, uint32_t shift) {
  return (x << shift) | (x >> (32 - shift));
}

static inline void qround(uint32_t x[16], size_t a, size_t b, size_t c, size_t d) {
  x[a] += x[b]; x[d] = rotl(x[d] ^ x[a], 16);
  x[c] += x[d]; x[b] = rotl(x[b] ^ x[c], 12);
  x[a] += x[b]; x[d] = rotl(x[d] ^ x[a], 8);
  x[c] += x[d]; x[b] = rotl(x[b] ^ x[c], 7);
}

static void chacha_block(mi_random_ctx_t* ctx)
{
  // scramble into `x`
  uint32_t x[16];
  for (size_t i = 0; i < 16; i++) {
    x[i] = ctx->input[i];
  }
  for (size_t i = 0; i < MI_CHACHA_ROUNDS; i += 2) {
    qround(x, 0, 4,  8, 12);
    qround(x, 1, 5,  9, 13);
    qround(x, 2, 6, 10, 14);
    qround(x, 3, 7, 11, 15);
    qround(x, 0, 5, 10, 15);
    qround(x, 1, 6, 11, 12);
    qround(x, 2, 7,  8, 13);
    qround(x, 3, 4,  9, 14);
  }

  // add scrambled data to the initial state
  for (size_t i = 0; i < 16; i++) {
    ctx->output[i] = x[i] + ctx->input[i];
  }
  ctx->output_available = 16;

  // increment the counter for the next round
  ctx->input[12] += 1;
  if (ctx->input[12] == 0) {
    ctx->input[13] += 1;
    if (ctx->input[13] == 0) {  // and keep increasing into the nonce
      ctx->input[14] += 1;
    }
  }
}

static uint32_t chacha_next32(mi_random_ctx_t* ctx) {
  if (ctx->output_available <= 0) {
    chacha_block(ctx);
    ctx->output_available = 16; // (assign again to suppress static analysis warning)
  }
  const uint32_t x = ctx->output[16 - ctx->output_available];
  ctx->output[16 - ctx->output_available] = 0; // reset once the data is handed out
  ctx->output_available--;
  return x;
}

static inline uint32_t read32(const uint8_t* p, size_t idx32) {
  const size_t i = 4*idx32;
  return ((uint32_t)p[i+0] | (uint32_t)p[i+1] << 8 | (uint32_t)p[i+2] << 16 | (uint32_t)p[i+3] << 24);
}

static void chacha_init(mi_random_ctx_t* ctx, const uint8_t key[32], uint64_t nonce)
{
  // since we only use chacha for randomness (and not encryption) we
  // do not _need_ to read 32-bit values as little endian but we do anyways
  // just for being compatible :-)
  memset(ctx, 0, sizeof(*ctx));
  for (size_t i = 0; i < 4; i++) {
    const uint8_t* sigma = (uint8_t*)"expand 32-byte k";
    ctx->input[i] = read32(sigma,i);
  }
  for (size_t i = 0; i < 8; i++) {
    ctx->input[i + 4] = read32(key,i);
  }
  ctx->input[12] = 0;
  ctx->input[13] = 0;
  ctx->input[14] = (uint32_t)nonce;
  ctx->input[15] = (uint32_t)(nonce >> 32);
}

static void chacha_split(mi_random_ctx_t* ctx, uint64_t nonce, mi_random_ctx_t* ctx_new) {
  memset(ctx_new, 0, sizeof(*ctx_new));
  memcpy(ctx_new->input, ctx->input, sizeof(ctx_new->input));
  ctx_new->input[12] = 0;
  ctx_new->input[13] = 0;
  ctx_new->input[14] = (uint32_t)nonce;
  ctx_new->input[15] = (uint32_t)(nonce >> 32);
  mi_assert_internal(ctx->input[14] != ctx_new->input[14] || ctx->input[15] != ctx_new->input[15]); // do not reuse nonces!
  chacha_block(ctx_new);
}


/* ----------------------------------------------------------------------------
Random interface
-----------------------------------------------------------------------------*/

#if MI_DEBUG>1
static bool mi_random_is_initialized(mi_random_ctx_t* ctx) {
  return (ctx != NULL && ctx->input[0] != 0);
}
#endif

void _mi_random_split(mi_random_ctx_t* ctx, mi_random_ctx_t* ctx_new) {
  mi_assert_internal(mi_random_is_initialized(ctx));
  mi_assert_internal(ctx != ctx_new);
  chacha_split(ctx, (uintptr_t)ctx_new /*nonce*/, ctx_new);
}

uintptr_t _mi_random_next(mi_random_ctx_t* ctx) {
  mi_assert_internal(mi_random_is_initialized(ctx));
  #if MI_INTPTR_SIZE <= 4
    return chacha_next32(ctx);
  #elif MI_INTPTR_SIZE == 8
    return (((uintptr_t)chacha_next32(ctx) << 32) | chacha_next32(ctx));
  #else
  # error "define mi_random_next for this platform"
  #endif
}


/* ----------------------------------------------------------------------------
To initialize a fresh random context we rely on the OS:
- Windows     : BCryptGenRandom
- osX,bsd,wasi: arc4random_buf
- Linux       : getrandom,/dev/urandom
If we cannot get good randomness, we fall back to weak randomness based on a timer and ASLR.
-----------------------------------------------------------------------------*/

#if defined(_WIN32)
#pragma comment (lib,"bcrypt.lib")
#include <bcrypt.h>
static bool os_random_buf(void* buf, size_t buf_len) {
  return (BCryptGenRandom(NULL, (PUCHAR)buf, (ULONG)buf_len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) >= 0);
}
/*
#define SystemFunction036 NTAPI SystemFunction036
#include <NTSecAPI.h>
#undef SystemFunction036
static bool os_random_buf(void* buf, size_t buf_len) {
  RtlGenRandom(buf, (ULONG)buf_len);
  return true;
}
*/
#elif defined(ANDROID) || defined(XP_DARWIN) || defined(__APPLE__) || defined(__DragonFly__) || \
      defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
      defined(__sun) || defined(__wasi__)
#include <stdlib.h>
static bool os_random_buf(void* buf, size_t buf_len) {
  arc4random_buf(buf, buf_len);
  return true;
}
#elif defined(__linux__)
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
static bool os_random_buf(void* buf, size_t buf_len) {
  // Modern Linux provides `getrandom` but different distributions either use `sys/random.h` or `linux/random.h`
  // and for the latter the actual `getrandom` call is not always defined.
  // (see <https://stackoverflow.com/questions/45237324/why-doesnt-getrandom-compile>)
  // We therefore use a syscall directly and fall back dynamically to /dev/urandom when needed.
#ifdef SYS_getrandom
  #ifndef GRND_NONBLOCK
  #define GRND_NONBLOCK (1)
  #endif
  static volatile _Atomic(uintptr_t) no_getrandom; // = 0
  if (mi_atomic_read(&no_getrandom)==0) {
    ssize_t ret = syscall(SYS_getrandom, buf, buf_len, GRND_NONBLOCK);
    if (ret >= 0) return (buf_len == (size_t)ret);
    if (ret != ENOSYS) return false;
    mi_atomic_write(&no_getrandom,1); // don't call again, and fall back to /dev/urandom
  }
#endif
  int flags = O_RDONLY;
  #if defined(O_CLOEXEC)
  flags |= O_CLOEXEC;
  #endif
  int fd = open("/dev/urandom", flags, 0);
  if (fd < 0) return false;
  size_t count = 0;
  while(count < buf_len) {
    ssize_t ret = read(fd, (char*)buf + count, buf_len - count);
    if (ret<=0) {
      if (errno!=EAGAIN && errno!=EINTR) break;
    }
    else {
      count += ret;
    }
  }
  close(fd);
  return (count==buf_len);
}
#elif defined(__EMSCRIPTEN__)
#include <emscripten.h>

EM_JS(int, mi_emsc_random_bytes, (void *buf, size_t buf_len), {
  if (window && window.crypto && window.crypto.getRandomValues) {
  #if defined(__EMSCRIPTEN_PTHREADS__)
    window.crypto.getRandomValues(new Uint8Array(HEAPU8.subarray(buf, buf + buf_len)));
  #else
    window.crypto.getRandomValues(HEAPU8.subarray(buf, buf + buf_len));
  #endif
    return 1;
  } else {
    return 0;
  }
});

static bool os_random_buf(void* buf, size_t buf_len) {
  if (mi_emsc_random_bytes(buf, buf_len)) {
    return true;
  } else {
    return false;
  }
}
#else
static bool os_random_buf(void* buf, size_t buf_len) {
  return false;
}
#endif

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach/mach_time.h>
#else
#include <time.h>
#endif

uintptr_t _os_random_weak(uintptr_t extra_seed) {
  uintptr_t x = (uintptr_t)&_os_random_weak ^ extra_seed; // ASLR makes the address random
  #if defined(_WIN32)
    LARGE_INTEGER pcount;
    QueryPerformanceCounter(&pcount);
    x ^= (uintptr_t)(pcount.QuadPart);
  #elif defined(__APPLE__)
    x ^= (uintptr_t)mach_absolute_time();
  #else
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    x ^= (uintptr_t)time.tv_sec;
    x ^= (uintptr_t)time.tv_nsec;
  #endif
  // and do a few randomization steps
  uintptr_t max = ((x ^ (x >> 17)) & 0x0F) + 1;
  for (uintptr_t i = 0; i < max; i++) {
    x = _mi_random_shuffle(x);
  }
  mi_assert_internal(x != 0);
  return x;
}

void _mi_random_init(mi_random_ctx_t* ctx) {
  uint8_t key[32];
  if (!os_random_buf(key, sizeof(key))) {
    // if we fail to get random data from the OS, we fall back to a
    // weak random source based on the current time
    _mi_warning_message("unable to use secure randomness\n");
    uintptr_t x = _os_random_weak(0);
    for (size_t i = 0; i < 8; i++) {  // key is eight 32-bit words.
      x = _mi_random_shuffle(x);
      ((uint32_t*)key)[i] = (uint32_t)x;
    }
  }
  chacha_init(ctx, key, (uintptr_t)ctx /*nonce*/ );
}

/* --------------------------------------------------------
test vectors from <https://tools.ietf.org/html/rfc8439>
----------------------------------------------------------- */
/*
static bool array_equals(uint32_t* x, uint32_t* y, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (x[i] != y[i]) return false;
  }
  return true;
}
static void chacha_test(void)
{
  uint32_t x[4] = { 0x11111111, 0x01020304, 0x9b8d6f43, 0x01234567 };
  uint32_t x_out[4] = { 0xea2a92f4, 0xcb1cf8ce, 0x4581472e, 0x5881c4bb };
  qround(x, 0, 1, 2, 3);
  mi_assert_internal(array_equals(x, x_out, 4));

  uint32_t y[16] = {
       0x879531e0,  0xc5ecf37d,  0x516461b1,  0xc9a62f8a,
       0x44c20ef3,  0x3390af7f,  0xd9fc690b,  0x2a5f714c,
       0x53372767,  0xb00a5631,  0x974c541a,  0x359e9963,
       0x5c971061,  0x3d631689,  0x2098d9d6,  0x91dbd320 };
  uint32_t y_out[16] = {
       0x879531e0,  0xc5ecf37d,  0xbdb886dc,  0xc9a62f8a,
       0x44c20ef3,  0x3390af7f,  0xd9fc690b,  0xcfacafd2,
       0xe46bea80,  0xb00a5631,  0x974c541a,  0x359e9963,
       0x5c971061,  0xccc07c79,  0x2098d9d6,  0x91dbd320 };
  qround(y, 2, 7, 8, 13);
  mi_assert_internal(array_equals(y, y_out, 16));

  mi_random_ctx_t r = {
    { 0x61707865, 0x3320646e, 0x79622d32, 0x6b206574,
      0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
      0x13121110, 0x17161514, 0x1b1a1918, 0x1f1e1d1c,
      0x00000001, 0x09000000, 0x4a000000, 0x00000000 },
    {0},
    0
  };
  uint32_t r_out[16] = {
       0xe4e7f110, 0x15593bd1, 0x1fdd0f50, 0xc47120a3,
       0xc7f4d1c7, 0x0368c033, 0x9aaa2204, 0x4e6cd4c3,
       0x466482d2, 0x09aa9f07, 0x05d7c214, 0xa2028bd9,
       0xd19c12b5, 0xb94e16de, 0xe883d0cb, 0x4e3c50a2 };
  chacha_block(&r);
  mi_assert_internal(array_equals(r.output, r_out, 16));
}
*/
/**** ended inlining random.c ****/
/**** start inlining os.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE   // ensure mmap flags are defined
#endif

#if defined(__sun)
// illumos provides new mman.h api when any of these are defined
// otherwise the old api based on caddr_t which predates the void pointers one.
// stock solaris provides only the former, chose to atomically to discard those
// flags only here rather than project wide tough.
#undef _XOPEN_SOURCE
#undef _POSIX_C_SOURCE
#endif
/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/
/**** skipping file: mimalloc-atomic.h ****/

#include <string.h>  // strerror


#if defined(_WIN32)
#include <windows.h>
#elif defined(__wasi__)
// stdlib.h is all we need, and has already been included in mimalloc.h
#else
#include <sys/mman.h>  // mmap
#include <unistd.h>    // sysconf
#if defined(__linux__)
#include <features.h>
#if defined(__GLIBC__)
#include <linux/mman.h> // linux mmap flags
#else
#include <sys/mman.h>
#endif
#endif
#if defined(__APPLE__)
#include <TargetConditionals.h>
#if !TARGET_IOS_IPHONE && !TARGET_IOS_SIMULATOR
#include <mach/vm_statistics.h>
#endif
#endif
#if defined(__HAIKU__)
#define madvise posix_madvise
#define MADV_DONTNEED POSIX_MADV_DONTNEED
#endif
#endif

/* -----------------------------------------------------------
  Initialization.
  On windows initializes support for aligned allocation and
  large OS pages (if MIMALLOC_LARGE_OS_PAGES is true).
----------------------------------------------------------- */
bool    _mi_os_decommit(void* addr, size_t size, mi_stats_t* stats);

static void* mi_align_up_ptr(void* p, size_t alignment) {
  return (void*)_mi_align_up((uintptr_t)p, alignment);
}

static uintptr_t _mi_align_down(uintptr_t sz, size_t alignment) {
  return (sz / alignment) * alignment;
}

static void* mi_align_down_ptr(void* p, size_t alignment) {
  return (void*)_mi_align_down((uintptr_t)p, alignment);
}

// page size (initialized properly in `os_init`)
static size_t os_page_size = 4096;

// minimal allocation granularity
static size_t os_alloc_granularity = 4096;

// if non-zero, use large page allocation
static size_t large_os_page_size = 0;

// OS (small) page size
size_t _mi_os_page_size() {
  return os_page_size;
}

// if large OS pages are supported (2 or 4MiB), then return the size, otherwise return the small page size (4KiB)
size_t _mi_os_large_page_size() {
  return (large_os_page_size != 0 ? large_os_page_size : _mi_os_page_size());
}

static bool use_large_os_page(size_t size, size_t alignment) {
  // if we have access, check the size and alignment requirements
  if (large_os_page_size == 0 || !mi_option_is_enabled(mi_option_large_os_pages)) return false;
  return ((size % large_os_page_size) == 0 && (alignment % large_os_page_size) == 0);
}

// round to a good OS allocation size (bounded by max 12.5% waste)
size_t _mi_os_good_alloc_size(size_t size) {
  size_t align_size;
  if (size < 512*KiB) align_size = _mi_os_page_size();
  else if (size < 2*MiB) align_size = 64*KiB;
  else if (size < 8*MiB) align_size = 256*KiB;
  else if (size < 32*MiB) align_size = 1*MiB;
  else align_size = 4*MiB;
  if (size >= (SIZE_MAX - align_size)) return size; // possible overflow?
  return _mi_align_up(size, align_size);
}

#if defined(_WIN32)
// We use VirtualAlloc2 for aligned allocation, but it is only supported on Windows 10 and Windows Server 2016.
// So, we need to look it up dynamically to run on older systems. (use __stdcall for 32-bit compatibility)
// NtAllocateVirtualAllocEx is used for huge OS page allocation (1GiB)
//
// We hide MEM_EXTENDED_PARAMETER to compile with older SDK's.
#include <winternl.h>
typedef PVOID    (__stdcall *PVirtualAlloc2)(HANDLE, PVOID, SIZE_T, ULONG, ULONG, /* MEM_EXTENDED_PARAMETER* */ void*, ULONG);
typedef NTSTATUS (__stdcall *PNtAllocateVirtualMemoryEx)(HANDLE, PVOID*, SIZE_T*, ULONG, ULONG, /* MEM_EXTENDED_PARAMETER* */ PVOID, ULONG);
static PVirtualAlloc2 pVirtualAlloc2 = NULL;
static PNtAllocateVirtualMemoryEx pNtAllocateVirtualMemoryEx = NULL;

// Similarly, GetNumaProcesorNodeEx is only supported since Windows 7
#if (_WIN32_WINNT < 0x601)  // before Win7
typedef struct _PROCESSOR_NUMBER { WORD Group; BYTE Number; BYTE Reserved; } PROCESSOR_NUMBER, *PPROCESSOR_NUMBER;
#endif
typedef VOID (__stdcall *PGetCurrentProcessorNumberEx)(PPROCESSOR_NUMBER ProcNumber);
typedef BOOL (__stdcall *PGetNumaProcessorNodeEx)(PPROCESSOR_NUMBER Processor, PUSHORT NodeNumber);
typedef BOOL (__stdcall* PGetNumaNodeProcessorMaskEx)(USHORT Node, PGROUP_AFFINITY ProcessorMask);
static PGetCurrentProcessorNumberEx pGetCurrentProcessorNumberEx = NULL;
static PGetNumaProcessorNodeEx      pGetNumaProcessorNodeEx = NULL;
static PGetNumaNodeProcessorMaskEx  pGetNumaNodeProcessorMaskEx = NULL;

static bool mi_win_enable_large_os_pages()
{
  if (large_os_page_size > 0) return true;

  // Try to see if large OS pages are supported
  // To use large pages on Windows, we first need access permission
  // Set "Lock pages in memory" permission in the group policy editor
  // <https://devblogs.microsoft.com/oldnewthing/20110128-00/?p=11643>
  unsigned long err = 0;
  HANDLE token = NULL;
  BOOL ok = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
  if (ok) {
    TOKEN_PRIVILEGES tp;
    ok = LookupPrivilegeValue(NULL, TEXT("SeLockMemoryPrivilege"), &tp.Privileges[0].Luid);
    if (ok) {
      tp.PrivilegeCount = 1;
      tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
      ok = AdjustTokenPrivileges(token, FALSE, &tp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
      if (ok) {
        err = GetLastError();
        ok = (err == ERROR_SUCCESS);
        if (ok) {
          large_os_page_size = GetLargePageMinimum();
        }
      }
    }
    CloseHandle(token);
  }
  if (!ok) {
    if (err == 0) err = GetLastError();
    _mi_warning_message("cannot enable large OS page support, error %lu\n", err);
  }
  return (ok!=0);
}

void _mi_os_init(void) {
  // get the page size
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  if (si.dwPageSize > 0) os_page_size = si.dwPageSize;
  if (si.dwAllocationGranularity > 0) os_alloc_granularity = si.dwAllocationGranularity;
  // get the VirtualAlloc2 function
  HINSTANCE  hDll;
  hDll = LoadLibrary(TEXT("kernelbase.dll"));
  if (hDll != NULL) {
    // use VirtualAlloc2FromApp if possible as it is available to Windows store apps
    pVirtualAlloc2 = (PVirtualAlloc2)(void (*)(void))GetProcAddress(hDll, "VirtualAlloc2FromApp");
    if (pVirtualAlloc2==NULL) pVirtualAlloc2 = (PVirtualAlloc2)(void (*)(void))GetProcAddress(hDll, "VirtualAlloc2");
    FreeLibrary(hDll);
  }
  // NtAllocateVirtualMemoryEx is used for huge page allocation
  hDll = LoadLibrary(TEXT("ntdll.dll"));
  if (hDll != NULL) {
    pNtAllocateVirtualMemoryEx = (PNtAllocateVirtualMemoryEx)(void (*)(void))GetProcAddress(hDll, "NtAllocateVirtualMemoryEx");
    FreeLibrary(hDll);
  }
  // Try to use Win7+ numa API
  hDll = LoadLibrary(TEXT("kernel32.dll"));
  if (hDll != NULL) {
    pGetCurrentProcessorNumberEx = (PGetCurrentProcessorNumberEx)(void (*)(void))GetProcAddress(hDll, "GetCurrentProcessorNumberEx");
    pGetNumaProcessorNodeEx = (PGetNumaProcessorNodeEx)(void (*)(void))GetProcAddress(hDll, "GetNumaProcessorNodeEx");
    pGetNumaNodeProcessorMaskEx = (PGetNumaNodeProcessorMaskEx)(void (*)(void))GetProcAddress(hDll, "GetNumaNodeProcessorMaskEx");
    FreeLibrary(hDll);
  }
  if (mi_option_is_enabled(mi_option_large_os_pages) || mi_option_is_enabled(mi_option_reserve_huge_os_pages)) {
    mi_win_enable_large_os_pages();
  }
}
#elif defined(__wasi__) || defined(__EMSCRIPTEN__)
void _mi_os_init() {
  os_page_size = 0x10000; // WebAssembly has a fixed page size: 64KB
  os_alloc_granularity = 16;
}
#else
void _mi_os_init() {
  // get the page size
  long result = sysconf(_SC_PAGESIZE);
  if (result > 0) {
    os_page_size = (size_t)result;
    os_alloc_granularity = os_page_size;
  }
  large_os_page_size = 2*MiB; // TODO: can we query the OS for this?
}
#endif


/* -----------------------------------------------------------
  Raw allocation on Windows (VirtualAlloc) and Unix's (mmap).
----------------------------------------------------------- */

static bool mi_os_mem_free(void* addr, size_t size, bool was_committed, mi_stats_t* stats)
{
  if (addr == NULL || size == 0) return true; // || _mi_os_is_huge_reserved(addr)
  bool err = false;
#if defined(_WIN32)
  err = (VirtualFree(addr, 0, MEM_RELEASE) == 0);
#elif defined(__wasi__) || defined(__EMSCRIPTEN__)
  err = 0; // WebAssembly's heap cannot be shrunk
#else
  err = (munmap(addr, size) == -1);
#endif
  if (was_committed) _mi_stat_decrease(&stats->committed, size);
  _mi_stat_decrease(&stats->reserved, size);
  if (err) {
    #pragma warning(suppress:4996)
    _mi_warning_message("munmap failed: %s, addr 0x%8li, size %lu\n", strerror(errno), (size_t)addr, size);
    return false;
  }
  else {
    return true;
  }
}

static void* mi_os_get_aligned_hint(size_t try_alignment, size_t size);

#ifdef _WIN32
static void* mi_win_virtual_allocx(void* addr, size_t size, size_t try_alignment, DWORD flags) {
#if (MI_INTPTR_SIZE >= 8)
  // on 64-bit systems, try to use the virtual address area after 4TiB for 4MiB aligned allocations
  void* hint;
  if (addr == NULL && (hint = mi_os_get_aligned_hint(try_alignment,size)) != NULL) {
    void* p = VirtualAlloc(hint, size, flags, PAGE_READWRITE);
    if (p != NULL) return p;
    DWORD err = GetLastError();
    if (err != ERROR_INVALID_ADDRESS &&   // If linked with multiple instances, we may have tried to allocate at an already allocated area (#210)
        err != ERROR_INVALID_PARAMETER) { // Windows7 instability (#230)
      return NULL;
    }
    // fall through
  } 
#endif
#if defined(MEM_EXTENDED_PARAMETER_TYPE_BITS)
  // on modern Windows try use VirtualAlloc2 for aligned allocation
  if (try_alignment > 0 && (try_alignment % _mi_os_page_size()) == 0 && pVirtualAlloc2 != NULL) {
    MEM_ADDRESS_REQUIREMENTS reqs = { 0, 0, 0 };
    reqs.Alignment = try_alignment;
    MEM_EXTENDED_PARAMETER param = { {0, 0}, {0} };
    param.Type = MemExtendedParameterAddressRequirements;
    param.Pointer = &reqs;
    return (*pVirtualAlloc2)(GetCurrentProcess(), addr, size, flags, PAGE_READWRITE, &param, 1);
  }
#endif
  // last resort
  return VirtualAlloc(addr, size, flags, PAGE_READWRITE);
}

static void* mi_win_virtual_alloc(void* addr, size_t size, size_t try_alignment, DWORD flags, bool large_only, bool allow_large, bool* is_large) {
  mi_assert_internal(!(large_only && !allow_large));
  static volatile _Atomic(uintptr_t) large_page_try_ok; // = 0;
  void* p = NULL;
  if ((large_only || use_large_os_page(size, try_alignment))
      && allow_large && (flags&MEM_COMMIT)!=0 && (flags&MEM_RESERVE)!=0) {
    uintptr_t try_ok = mi_atomic_read(&large_page_try_ok);
    if (!large_only && try_ok > 0) {
      // if a large page allocation fails, it seems the calls to VirtualAlloc get very expensive.
      // therefore, once a large page allocation failed, we don't try again for `large_page_try_ok` times.
      mi_atomic_cas_weak(&large_page_try_ok, try_ok - 1, try_ok);
    }
    else {
      // large OS pages must always reserve and commit.
      *is_large = true;
      p = mi_win_virtual_allocx(addr, size, try_alignment, flags | MEM_LARGE_PAGES);
      if (large_only) return p;
      // fall back to non-large page allocation on error (`p == NULL`).
      if (p == NULL) {
        mi_atomic_write(&large_page_try_ok,10);  // on error, don't try again for the next N allocations
      }
    }
  }
  if (p == NULL) {
    *is_large = ((flags&MEM_LARGE_PAGES) != 0);
    p = mi_win_virtual_allocx(addr, size, try_alignment, flags);
  }
  if (p == NULL) {
    _mi_warning_message("unable to allocate OS memory (%zu bytes, error code: %i, address: %p, large only: %d, allow large: %d)\n", size, GetLastError(), addr, large_only, allow_large);
  }
  return p;
}

#elif defined(__wasi__) || (defined(__EMSCRIPTEN__) && defined(EMSCRIPTEN_ALLOW_MEMORY_GROWTH))

static void* mi_wasm_heap_grow(size_t size, size_t try_alignment) {
  uintptr_t base = __builtin_wasm_memory_size(0) * _mi_os_page_size();
  uintptr_t aligned_base = _mi_align_up(base, (uintptr_t) try_alignment);
  size_t alloc_size = _mi_align_up( aligned_base - base + size, _mi_os_page_size());
  mi_assert(alloc_size >= size && (alloc_size % _mi_os_page_size()) == 0);
  if (alloc_size < size) return NULL;
  if (__builtin_wasm_memory_grow(0, alloc_size / _mi_os_page_size()) == SIZE_MAX) {
    errno = ENOMEM;
    return NULL;
  }
  return (void*)aligned_base;
}

#elif defined(__EMSCRIPTEN__)

static _Atomic(uintptr_t) mi_emscripten_base = 0x10000;

static void* mi_wasm_heap_grow(size_t size, size_t try_alignment) {
  size_t over_size = 0;
  if (try_alignment > 0x10000) over_size = try_alignment;
  uintptr_t base = mi_atomic_add(&mi_emscripten_base, size + over_size);
  uintptr_t top = __builtin_wasm_memory_size(0) * _mi_os_page_size();
  uintptr_t aligned_base = _mi_align_up(base, (uintptr_t) try_alignment);
  size_t alloc_size = _mi_align_up( aligned_base - base + size, _mi_os_page_size());
  mi_assert(alloc_size >= size && (alloc_size % _mi_os_page_size()) == 0);
  if (alloc_size < size) return NULL;
  if (aligned_base + alloc_size >= top) {
    errno = ENOMEM;
    return NULL;
  }
  return (void*)aligned_base;
}

#else
#define MI_OS_USE_MMAP
static void* mi_unix_mmapx(void* addr, size_t size, size_t try_alignment, int protect_flags, int flags, int fd) {
  void* p = NULL;
  #if (MI_INTPTR_SIZE >= 8) && !defined(MAP_ALIGNED)
  // on 64-bit systems, use the virtual address area after 4TiB for 4MiB aligned allocations
  void* hint;
  if (addr == NULL && (hint = mi_os_get_aligned_hint(try_alignment, size)) != NULL) {
    p = mmap(hint,size,protect_flags,flags,fd,0);
    if (p==MAP_FAILED) p = NULL; // fall back to regular mmap
  }
  #else
  UNUSED(try_alignment);
  UNUSED(mi_os_get_aligned_hint);
  #endif
  if (p==NULL) {
    p = mmap(addr,size,protect_flags,flags,fd,0);
    if (p==MAP_FAILED) p = NULL;
  }
  return p;
}

static void* mi_unix_mmap(void* addr, size_t size, size_t try_alignment, int protect_flags, bool large_only, bool allow_large, bool* is_large) {
  void* p = NULL;
  #if !defined(MAP_ANONYMOUS)
  #define MAP_ANONYMOUS  MAP_ANON
  #endif
  #if !defined(MAP_NORESERVE)
  #define MAP_NORESERVE  0
  #endif
  int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
  int fd = -1;
  #if defined(MAP_ALIGNED)  // BSD
  if (try_alignment > 0) {
    size_t n = _mi_bsr(try_alignment);
    if (((size_t)1 << n) == try_alignment && n >= 12 && n <= 30) {  // alignment is a power of 2 and 4096 <= alignment <= 1GiB
      flags |= MAP_ALIGNED(n);
    }
  }
  #endif
  #if defined(PROT_MAX)
  protect_flags |= PROT_MAX(PROT_READ | PROT_WRITE); // BSD
  #endif
  #if defined(VM_MAKE_TAG)
  // macOS: tracking anonymous page with a specific ID. (All up to 98 are taken officially but LLVM sanitizers had taken 99)
  int os_tag = (int)mi_option_get(mi_option_os_tag);
  if (os_tag < 100 || os_tag > 255) os_tag = 100;
  fd = VM_MAKE_TAG(os_tag);
  #endif
  if ((large_only || use_large_os_page(size, try_alignment)) && allow_large) {
    static volatile _Atomic(uintptr_t) large_page_try_ok; // = 0;
    uintptr_t try_ok = mi_atomic_read(&large_page_try_ok);
    if (!large_only && try_ok > 0) {
      // If the OS is not configured for large OS pages, or the user does not have
      // enough permission, the `mmap` will always fail (but it might also fail for other reasons).
      // Therefore, once a large page allocation failed, we don't try again for `large_page_try_ok` times
      // to avoid too many failing calls to mmap.
      mi_atomic_cas_weak(&large_page_try_ok, try_ok - 1, try_ok);
    }
    else {
      int lflags = flags & ~MAP_NORESERVE;  // using NORESERVE on huge pages seems to fail on Linux
      int lfd = fd;
      #ifdef MAP_ALIGNED_SUPER
      lflags |= MAP_ALIGNED_SUPER;
      #endif
      #ifdef MAP_HUGETLB
      lflags |= MAP_HUGETLB;
      #endif
      #ifdef MAP_HUGE_1GB
      static bool mi_huge_pages_available = true;
      if ((size % GiB) == 0 && mi_huge_pages_available) {
        lflags |= MAP_HUGE_1GB;
      }
      else
      #endif
      {
        #ifdef MAP_HUGE_2MB
        lflags |= MAP_HUGE_2MB;
        #endif
      }
      #ifdef VM_FLAGS_SUPERPAGE_SIZE_2MB
      lfd |= VM_FLAGS_SUPERPAGE_SIZE_2MB;
      #endif
      if (large_only || lflags != flags) {
        // try large OS page allocation
        *is_large = true;
        p = mi_unix_mmapx(addr, size, try_alignment, protect_flags, lflags, lfd);
        #ifdef MAP_HUGE_1GB
        if (p == NULL && (lflags & MAP_HUGE_1GB) != 0) {
          mi_huge_pages_available = false; // don't try huge 1GiB pages again
          _mi_warning_message("unable to allocate huge (1GiB) page, trying large (2MiB) pages instead (error %i)\n", errno);
          lflags = ((lflags & ~MAP_HUGE_1GB) | MAP_HUGE_2MB);
          p = mi_unix_mmapx(addr, size, try_alignment, protect_flags, lflags, lfd);
        }
        #endif
        if (large_only) return p;
        if (p == NULL) {
          mi_atomic_write(&large_page_try_ok, 10);  // on error, don't try again for the next N allocations
        }
      }
    }
  }
  if (p == NULL) {
    *is_large = false;
    p = mi_unix_mmapx(addr, size, try_alignment, protect_flags, flags, fd);
    #if defined(MADV_HUGEPAGE)
    // Many Linux systems don't allow MAP_HUGETLB but they support instead
    // transparent huge pages (THP). It is not required to call `madvise` with MADV_HUGE
    // though since properly aligned allocations will already use large pages if available
    // in that case -- in particular for our large regions (in `memory.c`).
    // However, some systems only allow THP if called with explicit `madvise`, so
    // when large OS pages are enabled for mimalloc, we call `madvice` anyways.
    if (allow_large && use_large_os_page(size, try_alignment)) {
      if (madvise(p, size, MADV_HUGEPAGE) == 0) {
        *is_large = true; // possibly
      };
    }
    #endif
    #if defined(__sun)
    if (allow_large && use_large_os_page(size, try_alignment)) {
      struct memcntl_mha cmd = {0};
      cmd.mha_pagesize = large_os_page_size;
      cmd.mha_cmd = MHA_MAPSIZE_VA;
      if (memcntl(p, size, MC_HAT_ADVISE, (caddr_t)&cmd, 0, 0) == 0) {
        *is_large = true;
      }
    }
    #endif
  }
  if (p == NULL) {
    _mi_warning_message("unable to allocate OS memory (%zu bytes, error code: %i, address: %p, large only: %d, allow large: %d)\n", size, errno, addr, large_only, allow_large);
  }
  return p;
}
#endif

// On 64-bit systems, we can do efficient aligned allocation by using
// the 4TiB to 30TiB area to allocate them.
#if (MI_INTPTR_SIZE >= 8) && (defined(_WIN32) || (defined(MI_OS_USE_MMAP) && !defined(MAP_ALIGNED)))
static volatile mi_decl_cache_align _Atomic(uintptr_t) aligned_base;

// Return a 4MiB aligned address that is probably available
static void* mi_os_get_aligned_hint(size_t try_alignment, size_t size) {
  if (try_alignment == 0 || try_alignment > MI_SEGMENT_SIZE) return NULL;
  if ((size%MI_SEGMENT_SIZE) != 0) return NULL;
  uintptr_t hint = mi_atomic_add(&aligned_base, size);
  if (hint == 0 || hint > ((intptr_t)30<<40)) { // try to wrap around after 30TiB (area after 32TiB is used for huge OS pages)
    uintptr_t init = ((uintptr_t)4 << 40); // start at 4TiB area
    #if (MI_SECURE>0 || MI_DEBUG==0)     // security: randomize start of aligned allocations unless in debug mode
    uintptr_t r = _mi_heap_random_next(mi_get_default_heap());
    init = init + (MI_SEGMENT_SIZE * ((r>>17) & 0xFFFFF));  // (randomly 20 bits)*4MiB == 0 to 4TiB
    #endif
    mi_atomic_cas_strong(&aligned_base, init, hint + size);
    hint = mi_atomic_add(&aligned_base, size); // this may still give 0 or > 30TiB but that is ok, it is a hint after all
  }
  if (hint%try_alignment != 0) return NULL;
  return (void*)hint;
}
#else
static void* mi_os_get_aligned_hint(size_t try_alignment, size_t size) {
  UNUSED(try_alignment); UNUSED(size);
  return NULL;
}
#endif


// Primitive allocation from the OS.
// Note: the `try_alignment` is just a hint and the returned pointer is not guaranteed to be aligned.
static void* mi_os_mem_alloc(size_t size, size_t try_alignment, bool commit, bool allow_large, bool* is_large, mi_stats_t* stats) {
  mi_assert_internal(size > 0 && (size % _mi_os_page_size()) == 0);
  if (size == 0) return NULL;
  if (!commit) allow_large = false;

  void* p = NULL;
  /*
  if (commit && allow_large) {
    p = _mi_os_try_alloc_from_huge_reserved(size, try_alignment);
    if (p != NULL) {
      *is_large = true;
      return p;
    }
  }
  */

  #if defined(_WIN32)
    int flags = MEM_RESERVE;
    if (commit) flags |= MEM_COMMIT;
    p = mi_win_virtual_alloc(NULL, size, try_alignment, flags, false, allow_large, is_large);
  #elif defined(__wasi__) || defined(__EMSCRIPTEN__)
    *is_large = false;
    p = mi_wasm_heap_grow(size, try_alignment);
  #else
    int protect_flags = (commit ? (PROT_WRITE | PROT_READ) : PROT_NONE);
    p = mi_unix_mmap(NULL, size, try_alignment, protect_flags, false, allow_large, is_large);
  #endif
  mi_stat_counter_increase(stats->mmap_calls, 1);
  if (p != NULL) {
    _mi_stat_increase(&stats->reserved, size);
    if (commit) { _mi_stat_increase(&stats->committed, size); }
  }
  return p;
}


// Primitive aligned allocation from the OS.
// This function guarantees the allocated memory is aligned.
static void* mi_os_mem_alloc_aligned(size_t size, size_t alignment, bool commit, bool allow_large, bool* is_large, mi_stats_t* stats) {
  mi_assert_internal(alignment >= _mi_os_page_size() && ((alignment & (alignment - 1)) == 0));
  mi_assert_internal(size > 0 && (size % _mi_os_page_size()) == 0);
  if (!commit) allow_large = false;
  if (!(alignment >= _mi_os_page_size() && ((alignment & (alignment - 1)) == 0))) return NULL;
  size = _mi_align_up(size, _mi_os_page_size());

  // try first with a hint (this will be aligned directly on Win 10+ or BSD)
  void* p = mi_os_mem_alloc(size, alignment, commit, allow_large, is_large, stats);
  if (p == NULL) return NULL;

  // if not aligned, free it, overallocate, and unmap around it
  if (((uintptr_t)p % alignment != 0)) {
    mi_os_mem_free(p, size, commit, stats);
    if (size >= (SIZE_MAX - alignment)) return NULL; // overflow
    size_t over_size = size + alignment;

#if _WIN32
    // over-allocate and than re-allocate exactly at an aligned address in there.
    // this may fail due to threads allocating at the same time so we
    // retry this at most 3 times before giving up.
    // (we can not decommit around the overallocation on Windows, because we can only
    //  free the original pointer, not one pointing inside the area)
    int flags = MEM_RESERVE;
    if (commit) flags |= MEM_COMMIT;
    for (int tries = 0; tries < 3; tries++) {
      // over-allocate to determine a virtual memory range
      p = mi_os_mem_alloc(over_size, alignment, commit, false, is_large, stats);
      if (p == NULL) return NULL; // error
      if (((uintptr_t)p % alignment) == 0) {
        // if p happens to be aligned, just decommit the left-over area
        _mi_os_decommit((uint8_t*)p + size, over_size - size, stats);
        break;
      }
      else {
        // otherwise free and allocate at an aligned address in there
        mi_os_mem_free(p, over_size, commit, stats);
        void* aligned_p = mi_align_up_ptr(p, alignment);
        p = mi_win_virtual_alloc(aligned_p, size, alignment, flags, false, allow_large, is_large);
        if (p == aligned_p) break; // success!
        if (p != NULL) { // should not happen?
          mi_os_mem_free(p, size, commit, stats);
          p = NULL;
        }
      }
    }
#else
    // overallocate...
    p = mi_os_mem_alloc(over_size, alignment, commit, false, is_large, stats);
    if (p == NULL) return NULL;
    // and selectively unmap parts around the over-allocated area.
    void* aligned_p = mi_align_up_ptr(p, alignment);
    size_t pre_size = (uint8_t*)aligned_p - (uint8_t*)p;
    size_t mid_size = _mi_align_up(size, _mi_os_page_size());
    size_t post_size = over_size - pre_size - mid_size;
    mi_assert_internal(pre_size < over_size && post_size < over_size && mid_size >= size);
    if (pre_size > 0)  mi_os_mem_free(p, pre_size, commit, stats);
    if (post_size > 0) mi_os_mem_free((uint8_t*)aligned_p + mid_size, post_size, commit, stats);
    // we can return the aligned pointer on `mmap` systems
    p = aligned_p;
#endif
  }

  mi_assert_internal(p == NULL || (p != NULL && ((uintptr_t)p % alignment) == 0));
  return p;
}

/* -----------------------------------------------------------
  OS API: alloc, free, alloc_aligned
----------------------------------------------------------- */

void* _mi_os_alloc(size_t size, mi_stats_t* stats) {
  if (size == 0) return NULL;
  size = _mi_os_good_alloc_size(size);
  bool is_large = false;
  return mi_os_mem_alloc(size, 0, true, false, &is_large, stats);
}

void  _mi_os_free_ex(void* p, size_t size, bool was_committed, mi_stats_t* stats) {
  if (size == 0 || p == NULL) return;
  size = _mi_os_good_alloc_size(size);
  mi_os_mem_free(p, size, was_committed, stats);
}

void  _mi_os_free(void* p, size_t size, mi_stats_t* stats) {
  _mi_os_free_ex(p, size, true, stats);
}

void* _mi_os_alloc_aligned(size_t size, size_t alignment, bool commit, bool* large, mi_os_tld_t* tld)
{
  if (size == 0) return NULL;
  size = _mi_os_good_alloc_size(size);
  alignment = _mi_align_up(alignment, _mi_os_page_size());
  bool allow_large = false;
  if (large != NULL) {
    allow_large = *large;
    *large = false;
  }
  return mi_os_mem_alloc_aligned(size, alignment, commit, allow_large, (large!=NULL?large:&allow_large), tld->stats);
}



/* -----------------------------------------------------------
  OS memory API: reset, commit, decommit, protect, unprotect.
----------------------------------------------------------- */


// OS page align within a given area, either conservative (pages inside the area only),
// or not (straddling pages outside the area is possible)
static void* mi_os_page_align_areax(bool conservative, void* addr, size_t size, size_t* newsize) {
  mi_assert(addr != NULL && size > 0);
  if (newsize != NULL) *newsize = 0;
  if (size == 0 || addr == NULL) return NULL;

  // page align conservatively within the range
  void* start = (conservative ? mi_align_up_ptr(addr, _mi_os_page_size())
    : mi_align_down_ptr(addr, _mi_os_page_size()));
  void* end = (conservative ? mi_align_down_ptr((uint8_t*)addr + size, _mi_os_page_size())
    : mi_align_up_ptr((uint8_t*)addr + size, _mi_os_page_size()));
  ptrdiff_t diff = (uint8_t*)end - (uint8_t*)start;
  if (diff <= 0) return NULL;

  mi_assert_internal((conservative && (size_t)diff <= size) || (!conservative && (size_t)diff >= size));
  if (newsize != NULL) *newsize = (size_t)diff;
  return start;
}

static void* mi_os_page_align_area_conservative(void* addr, size_t size, size_t* newsize) {
  return mi_os_page_align_areax(true, addr, size, newsize);
}

static void mi_mprotect_hint(int err) {
#if defined(MI_OS_USE_MMAP) && (MI_SECURE>=2) // guard page around every mimalloc page
  if (err == ENOMEM) {
    _mi_warning_message("the previous warning may have been caused by a low memory map limit.\n"
                        "  On Linux this is controlled by the vm.max_map_count. For example:\n"
                        "  > sudo sysctl -w vm.max_map_count=262144\n");
  }
#else
  UNUSED(err);
#endif
}

// Commit/Decommit memory.
// Usually commit is aligned liberal, while decommit is aligned conservative.
// (but not for the reset version where we want commit to be conservative as well)
static bool mi_os_commitx(void* addr, size_t size, bool commit, bool conservative, bool* is_zero, mi_stats_t* stats) {
  // page align in the range, commit liberally, decommit conservative
  if (is_zero != NULL) { *is_zero = false; }
  size_t csize;
  void* start = mi_os_page_align_areax(conservative, addr, size, &csize);
  if (csize == 0) return true;  // || _mi_os_is_huge_reserved(addr))
  int err = 0;
  if (commit) {
    _mi_stat_increase(&stats->committed, csize);
    _mi_stat_counter_increase(&stats->commit_calls, 1);
  }
  else {
    _mi_stat_decrease(&stats->committed, csize);
  }

  #if defined(_WIN32)
  if (commit) {
    // if the memory was already committed, the call succeeds but it is not zero'd
    // *is_zero = true;
    void* p = VirtualAlloc(start, csize, MEM_COMMIT, PAGE_READWRITE);
    err = (p == start ? 0 : GetLastError());
  }
  else {
    BOOL ok = VirtualFree(start, csize, MEM_DECOMMIT);
    err = (ok ? 0 : GetLastError());
  }
  #elif defined(__wasi__) || defined(__EMSCRIPTEN__)
  // WebAssembly guests can't control memory protection
  #elif defined(MAP_FIXED)
  if (!commit) {
    // use mmap with MAP_FIXED to discard the existing memory (and reduce commit charge)
    void* p = mmap(start, csize, PROT_NONE, (MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE), -1, 0);
    if (p != start) { err = errno; }
  }
  else {
    // for commit, just change the protection
    err = mprotect(start, csize, (PROT_READ | PROT_WRITE));
    if (err != 0) { err = errno; }
  }
  #else
  err = mprotect(start, csize, (commit ? (PROT_READ | PROT_WRITE) : PROT_NONE));
  if (err != 0) { err = errno; }
  #endif
  if (err != 0) {
    _mi_warning_message("%s error: start: %p, csize: 0x%x, err: %i\n", commit ? "commit" : "decommit", start, csize, err);
    mi_mprotect_hint(err);
  }
  mi_assert_internal(err == 0);
  return (err == 0);
}

bool _mi_os_commit(void* addr, size_t size, bool* is_zero, mi_stats_t* stats) {
  return mi_os_commitx(addr, size, true, false /* liberal */, is_zero, stats);
}

bool _mi_os_decommit(void* addr, size_t size, mi_stats_t* stats) {
  bool is_zero;
  return mi_os_commitx(addr, size, false, true /* conservative */, &is_zero, stats);
}

bool _mi_os_commit_unreset(void* addr, size_t size, bool* is_zero, mi_stats_t* stats) {
  return mi_os_commitx(addr, size, true, true /* conservative */, is_zero, stats);
}

// Signal to the OS that the address range is no longer in use
// but may be used later again. This will release physical memory
// pages and reduce swapping while keeping the memory committed.
// We page align to a conservative area inside the range to reset.
static bool mi_os_resetx(void* addr, size_t size, bool reset, mi_stats_t* stats) {
  // page align conservatively within the range
  size_t csize;
  void* start = mi_os_page_align_area_conservative(addr, size, &csize);
  if (csize == 0) return true;  // || _mi_os_is_huge_reserved(addr)
  if (reset) _mi_stat_increase(&stats->reset, csize);
        else _mi_stat_decrease(&stats->reset, csize);
  if (!reset) return true; // nothing to do on unreset!

  #if (MI_DEBUG>1)
  if (MI_SECURE==0) {
    memset(start, 0, csize); // pretend it is eagerly reset
  }
  #endif

#if defined(_WIN32)
  // Testing shows that for us (on `malloc-large`) MEM_RESET is 2x faster than DiscardVirtualMemory
  void* p = VirtualAlloc(start, csize, MEM_RESET, PAGE_READWRITE);
  mi_assert_internal(p == start);
  #if 1
  if (p == start && start != NULL) {
    VirtualUnlock(start,csize); // VirtualUnlock after MEM_RESET removes the memory from the working set
  }
  #endif
  if (p != start) return false;
#else
#if defined(MADV_FREE)
  static int advice = MADV_FREE;
  int err = madvise(start, csize, advice);
  if (err != 0 && errno == EINVAL && advice == MADV_FREE) {
    // if MADV_FREE is not supported, fall back to MADV_DONTNEED from now on
    advice = MADV_DONTNEED;
    err = madvise(start, csize, advice);
  }
#elif defined(__wasi__) || defined(__EMSCRIPTEN__)
  int err = 0;
#else
  int err = madvise(start, csize, MADV_DONTNEED);
#endif
  if (err != 0) {
    _mi_warning_message("madvise reset error: start: %p, csize: 0x%x, errno: %i\n", start, csize, errno);
  }
  //mi_assert(err == 0);
  if (err != 0) return false;
#endif
  return true;
}

// Signal to the OS that the address range is no longer in use
// but may be used later again. This will release physical memory
// pages and reduce swapping while keeping the memory committed.
// We page align to a conservative area inside the range to reset.
bool _mi_os_reset(void* addr, size_t size, mi_stats_t* stats) {
  if (mi_option_is_enabled(mi_option_reset_decommits)) {
    return _mi_os_decommit(addr, size, stats);
  }
  else {
    return mi_os_resetx(addr, size, true, stats);
  }
}

bool _mi_os_unreset(void* addr, size_t size, bool* is_zero, mi_stats_t* stats) {
  if (mi_option_is_enabled(mi_option_reset_decommits)) {
    return _mi_os_commit_unreset(addr, size, is_zero, stats);  // re-commit it (conservatively!)
  }
  else {
    *is_zero = false;
    return mi_os_resetx(addr, size, false, stats);
  }
}


// Protect a region in memory to be not accessible.
static  bool mi_os_protectx(void* addr, size_t size, bool protect) {
  // page align conservatively within the range
  size_t csize = 0;
  void* start = mi_os_page_align_area_conservative(addr, size, &csize);
  if (csize == 0) return false;
  /*
  if (_mi_os_is_huge_reserved(addr)) {
	  _mi_warning_message("cannot mprotect memory allocated in huge OS pages\n");
  }
  */
  int err = 0;
#ifdef _WIN32
  DWORD oldprotect = 0;
  BOOL ok = VirtualProtect(start, csize, protect ? PAGE_NOACCESS : PAGE_READWRITE, &oldprotect);
  err = (ok ? 0 : GetLastError());
#elif defined(__wasi__) || defined(__EMSCRIPTEN__)
  err = 0;
#else
  err = mprotect(start, csize, protect ? PROT_NONE : (PROT_READ | PROT_WRITE));
  if (err != 0) { err = errno; }
#endif
  if (err != 0) {
    _mi_warning_message("mprotect error: start: %p, csize: 0x%x, err: %i\n", start, csize, err);
    mi_mprotect_hint(err);
  }
  return (err == 0);
}

bool _mi_os_protect(void* addr, size_t size) {
  return mi_os_protectx(addr, size, true);
}

bool _mi_os_unprotect(void* addr, size_t size) {
  return mi_os_protectx(addr, size, false);
}



bool _mi_os_shrink(void* p, size_t oldsize, size_t newsize, mi_stats_t* stats) {
  // page align conservatively within the range
  mi_assert_internal(oldsize > newsize && p != NULL);
  if (oldsize < newsize || p == NULL) return false;
  if (oldsize == newsize) return true;

  // oldsize and newsize should be page aligned or we cannot shrink precisely
  void* addr = (uint8_t*)p + newsize;
  size_t size = 0;
  void* start = mi_os_page_align_area_conservative(addr, oldsize - newsize, &size);
  if (size == 0 || start != addr) return false;

#ifdef _WIN32
  // we cannot shrink on windows, but we can decommit
  return _mi_os_decommit(start, size, stats);
#else
  return mi_os_mem_free(start, size, true, stats);
#endif
}


/* ----------------------------------------------------------------------------
Support for allocating huge OS pages (1Gib) that are reserved up-front
and possibly associated with a specific NUMA node. (use `numa_node>=0`)
-----------------------------------------------------------------------------*/
#define MI_HUGE_OS_PAGE_SIZE  (GiB)

#if defined(_WIN32) && (MI_INTPTR_SIZE >= 8)
static void* mi_os_alloc_huge_os_pagesx(void* addr, size_t size, int numa_node)
{
  mi_assert_internal(size%GiB == 0);
  mi_assert_internal(addr != NULL);
  const DWORD flags = MEM_LARGE_PAGES | MEM_COMMIT | MEM_RESERVE;

  mi_win_enable_large_os_pages();

  #if defined(MEM_EXTENDED_PARAMETER_TYPE_BITS)
  MEM_EXTENDED_PARAMETER params[3] = { {{0,0},{0}},{{0,0},{0}},{{0,0},{0}} };
  // on modern Windows try use NtAllocateVirtualMemoryEx for 1GiB huge pages
  static bool mi_huge_pages_available = true;
  if (pNtAllocateVirtualMemoryEx != NULL && mi_huge_pages_available) {
    #ifndef MEM_EXTENDED_PARAMETER_NONPAGED_HUGE
    #define MEM_EXTENDED_PARAMETER_NONPAGED_HUGE  (0x10)
    #endif
    params[0].Type = 5; // == MemExtendedParameterAttributeFlags;
    params[0].ULong64 = MEM_EXTENDED_PARAMETER_NONPAGED_HUGE;
    ULONG param_count = 1;
    if (numa_node >= 0) {
      param_count++;
      params[1].Type = MemExtendedParameterNumaNode;
      params[1].ULong = (unsigned)numa_node;
    }
    SIZE_T psize = size;
    void* base = addr;
    NTSTATUS err = (*pNtAllocateVirtualMemoryEx)(GetCurrentProcess(), &base, &psize, flags, PAGE_READWRITE, params, param_count);
    if (err == 0 && base != NULL) {
      return base;
    }
    else {
      // fall back to regular large pages
      mi_huge_pages_available = false; // don't try further huge pages
      _mi_warning_message("unable to allocate using huge (1gb) pages, trying large (2mb) pages instead (status 0x%lx)\n", err);
    }
  }
  // on modern Windows try use VirtualAlloc2 for numa aware large OS page allocation
  if (pVirtualAlloc2 != NULL && numa_node >= 0) {
    params[0].Type = MemExtendedParameterNumaNode;
    params[0].ULong = (unsigned)numa_node;
    return (*pVirtualAlloc2)(GetCurrentProcess(), addr, size, flags, PAGE_READWRITE, params, 1);
  }
  #else
    UNUSED(numa_node);
  #endif
  // otherwise use regular virtual alloc on older windows
  return VirtualAlloc(addr, size, flags, PAGE_READWRITE);
}

#elif defined(MI_OS_USE_MMAP) && (MI_INTPTR_SIZE >= 8) && !defined(__HAIKU__)
#include <sys/syscall.h>
#ifndef MPOL_PREFERRED
#define MPOL_PREFERRED 1
#endif
#if defined(SYS_mbind)
static long mi_os_mbind(void* start, unsigned long len, unsigned long mode, const unsigned long* nmask, unsigned long maxnode, unsigned flags) {
  return syscall(SYS_mbind, start, len, mode, nmask, maxnode, flags);
}
#else
static long mi_os_mbind(void* start, unsigned long len, unsigned long mode, const unsigned long* nmask, unsigned long maxnode, unsigned flags) {
  UNUSED(start); UNUSED(len); UNUSED(mode); UNUSED(nmask); UNUSED(maxnode); UNUSED(flags);
  return 0;
}
#endif
static void* mi_os_alloc_huge_os_pagesx(void* addr, size_t size, int numa_node) {
  mi_assert_internal(size%GiB == 0);
  bool is_large = true;
  void* p = mi_unix_mmap(addr, size, MI_SEGMENT_SIZE, PROT_READ | PROT_WRITE, true, true, &is_large);
  if (p == NULL) return NULL;
  if (numa_node >= 0 && numa_node < 8*MI_INTPTR_SIZE) { // at most 64 nodes
    uintptr_t numa_mask = (1UL << numa_node);
    // TODO: does `mbind` work correctly for huge OS pages? should we
    // use `set_mempolicy` before calling mmap instead?
    // see: <https://lkml.org/lkml/2017/2/9/875>
    long err = mi_os_mbind(p, size, MPOL_PREFERRED, &numa_mask, 8*MI_INTPTR_SIZE, 0);
    if (err != 0) {
      _mi_warning_message("failed to bind huge (1gb) pages to numa node %d: %s\n", numa_node, strerror(errno));
    }
  }
  return p;
}
#else
static void* mi_os_alloc_huge_os_pagesx(void* addr, size_t size, int numa_node) {
  UNUSED(addr); UNUSED(size); UNUSED(numa_node);
  return NULL;
}
#endif

#if (MI_INTPTR_SIZE >= 8)
// To ensure proper alignment, use our own area for huge OS pages
static mi_decl_cache_align _Atomic(uintptr_t)  mi_huge_start; // = 0

// Claim an aligned address range for huge pages
static uint8_t* mi_os_claim_huge_pages(size_t pages, size_t* total_size) {
  if (total_size != NULL) *total_size = 0;
  const size_t size = pages * MI_HUGE_OS_PAGE_SIZE;

  uintptr_t start = 0;
  uintptr_t end = 0;
  uintptr_t expected;
  do {
    start = expected = mi_atomic_read_relaxed(&mi_huge_start);
    if (start == 0) {
      // Initialize the start address after the 32TiB area
      start = ((uintptr_t)32 << 40);  // 32TiB virtual start address
#if (MI_SECURE>0 || MI_DEBUG==0)      // security: randomize start of huge pages unless in debug mode
      uintptr_t r = _mi_heap_random_next(mi_get_default_heap());
      start = start + ((uintptr_t)MI_HUGE_OS_PAGE_SIZE * ((r>>17) & 0x0FFF));  // (randomly 12bits)*1GiB == between 0 to 4TiB
#endif
    }
    end = start + size;
    mi_assert_internal(end % MI_SEGMENT_SIZE == 0);
  } while (!mi_atomic_cas_strong(&mi_huge_start, end, expected));

  if (total_size != NULL) *total_size = size;
  return (uint8_t*)start;
}
#else
static uint8_t* mi_os_claim_huge_pages(size_t pages, size_t* total_size) {
  UNUSED(pages);
  if (total_size != NULL) *total_size = 0;
  return NULL;
}
#endif

// Allocate MI_SEGMENT_SIZE aligned huge pages
void* _mi_os_alloc_huge_os_pages(size_t pages, int numa_node, mi_msecs_t max_msecs, size_t* pages_reserved, size_t* psize) {
  if (psize != NULL) *psize = 0;
  if (pages_reserved != NULL) *pages_reserved = 0;
  size_t size = 0;
  uint8_t* start = mi_os_claim_huge_pages(pages, &size);
  if (start == NULL) return NULL; // or 32-bit systems

  // Allocate one page at the time but try to place them contiguously
  // We allocate one page at the time to be able to abort if it takes too long
  // or to at least allocate as many as available on the system.
  mi_msecs_t start_t = _mi_clock_start();
  size_t page;
  for (page = 0; page < pages; page++) {
    // allocate a page
    void* addr = start + (page * MI_HUGE_OS_PAGE_SIZE);
    void* p = mi_os_alloc_huge_os_pagesx(addr, MI_HUGE_OS_PAGE_SIZE, numa_node);

    // Did we succeed at a contiguous address?
    if (p != addr) {
      // no success, issue a warning and break
      if (p != NULL) {
        _mi_warning_message("could not allocate contiguous huge page %zu at %p\n", page, addr);
        _mi_os_free(p, MI_HUGE_OS_PAGE_SIZE, &_mi_stats_main);
      }
      break;
    }

    // success, record it
    _mi_stat_increase(&_mi_stats_main.committed, MI_HUGE_OS_PAGE_SIZE);
    _mi_stat_increase(&_mi_stats_main.reserved, MI_HUGE_OS_PAGE_SIZE);

    // check for timeout
    if (max_msecs > 0) {
      mi_msecs_t elapsed = _mi_clock_end(start_t);
      if (page >= 1) {
        mi_msecs_t estimate = ((elapsed / (page+1)) * pages);
        if (estimate > 2*max_msecs) { // seems like we are going to timeout, break
          elapsed = max_msecs + 1;
        }
      }
      if (elapsed > max_msecs) {
        _mi_warning_message("huge page allocation timed out\n");
        break;
      }
    }
  }
  mi_assert_internal(page*MI_HUGE_OS_PAGE_SIZE <= size);
  if (pages_reserved != NULL) *pages_reserved = page;
  if (psize != NULL) *psize = page * MI_HUGE_OS_PAGE_SIZE;
  return (page == 0 ? NULL : start);
}

// free every huge page in a range individually (as we allocated per page)
// note: needed with VirtualAlloc but could potentially be done in one go on mmap'd systems.
void _mi_os_free_huge_pages(void* p, size_t size, mi_stats_t* stats) {
  if (p==NULL || size==0) return;
  uint8_t* base = (uint8_t*)p;
  while (size >= MI_HUGE_OS_PAGE_SIZE) {
    _mi_os_free(base, MI_HUGE_OS_PAGE_SIZE, stats);
    size -= MI_HUGE_OS_PAGE_SIZE;
  }
}

/* ----------------------------------------------------------------------------
Support NUMA aware allocation
-----------------------------------------------------------------------------*/
#ifdef _WIN32  
static size_t mi_os_numa_nodex() {
  USHORT numa_node = 0;
  if (pGetCurrentProcessorNumberEx != NULL && pGetNumaProcessorNodeEx != NULL) {
    // Extended API is supported
    PROCESSOR_NUMBER pnum;
    (*pGetCurrentProcessorNumberEx)(&pnum);
    USHORT nnode = 0;
    BOOL ok = (*pGetNumaProcessorNodeEx)(&pnum, &nnode);
    if (ok) numa_node = nnode;
  }
  else {
    // Vista or earlier, use older API that is limited to 64 processors. Issue #277
    DWORD pnum = GetCurrentProcessorNumber();
    UCHAR nnode = 0;
    BOOL ok = GetNumaProcessorNode((UCHAR)pnum, &nnode);
    if (ok) numa_node = nnode;    
  }
  return numa_node;
}

static size_t mi_os_numa_node_countx(void) {
  ULONG numa_max = 0;
  GetNumaHighestNodeNumber(&numa_max);
  // find the highest node number that has actual processors assigned to it. Issue #282
  while(numa_max > 0) {
    if (pGetNumaNodeProcessorMaskEx != NULL) {
      // Extended API is supported
      GROUP_AFFINITY affinity;
      if ((*pGetNumaNodeProcessorMaskEx)((USHORT)numa_max, &affinity)) {
        if (affinity.Mask != 0) break;  // found the maximum non-empty node
      }
    }
    else {
      // Vista or earlier, use older API that is limited to 64 processors.
      ULONGLONG mask;
      if (GetNumaNodeProcessorMask((UCHAR)numa_max, &mask)) {
        if (mask != 0) break; // found the maximum non-empty node
      };
    }
    // max node was invalid or had no processor assigned, try again
    numa_max--;
  }
  return ((size_t)numa_max + 1);
}
#elif defined(__linux__)
#include <sys/syscall.h>  // getcpu
#include <stdio.h>        // access

static size_t mi_os_numa_nodex(void) {
#ifdef SYS_getcpu
  unsigned long node = 0;
  unsigned long ncpu = 0;
  long err = syscall(SYS_getcpu, &ncpu, &node, NULL);
  if (err != 0) return 0;
  return node;
#else
  return 0;
#endif
}
static size_t mi_os_numa_node_countx(void) {
  char buf[128];
  unsigned node = 0;
  for(node = 0; node < 256; node++) {
    // enumerate node entries -- todo: it there a more efficient way to do this? (but ensure there is no allocation)
    snprintf(buf, 127, "/sys/devices/system/node/node%u", node + 1);
    if (access(buf,R_OK) != 0) break;
  }
  return (node+1);
}
#else
static size_t mi_os_numa_nodex(void) {
  return 0;
}
static size_t mi_os_numa_node_countx(void) {
  return 1;
}
#endif

size_t _mi_numa_node_count = 0;   // cache the node count

size_t _mi_os_numa_node_count_get(void) {
  if (mi_unlikely(_mi_numa_node_count <= 0)) {
    long ncount = mi_option_get(mi_option_use_numa_nodes); // given explicitly?
    if (ncount <= 0) ncount = (long)mi_os_numa_node_countx();        // or detect dynamically
    _mi_numa_node_count = (size_t)(ncount <= 0 ? 1 : ncount);
    _mi_verbose_message("using %zd numa regions\n", _mi_numa_node_count);
  }
  mi_assert_internal(_mi_numa_node_count >= 1);
  return _mi_numa_node_count;
}

int _mi_os_numa_node_get(mi_os_tld_t* tld) {
  UNUSED(tld);
  size_t numa_count = _mi_os_numa_node_count();
  if (numa_count<=1) return 0; // optimize on single numa node systems: always node 0
  // never more than the node count and >= 0
  size_t numa_node = mi_os_numa_nodex();
  if (numa_node >= numa_count) { numa_node = numa_node % numa_count; }
  return (int)numa_node;
}
/**** ended inlining os.c ****/
/**** start inlining arena.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2019, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
"Arenas" are fixed area's of OS memory from which we can allocate
large blocks (>= MI_ARENA_BLOCK_SIZE, 32MiB).
In contrast to the rest of mimalloc, the arenas are shared between
threads and need to be accessed using atomic operations.

Currently arenas are only used to for huge OS page (1GiB) reservations,
otherwise it delegates to direct allocation from the OS.
In the future, we can expose an API to manually add more kinds of arenas
which is sometimes needed for embedded devices or shared memory for example.
(We can also employ this with WASI or `sbrk` systems to reserve large arenas
 on demand and be able to reuse them efficiently).

The arena allocation needs to be thread safe and we use an atomic
bitmap to allocate. The current implementation of the bitmap can
only do this within a field (`uintptr_t`) so we can allocate at most
blocks of 2GiB (64*32MiB) and no object can cross the boundary. This
can lead to fragmentation but fortunately most objects will be regions
of 256MiB in practice.
-----------------------------------------------------------------------------*/
/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/
/**** skipping file: mimalloc-atomic.h ****/

#include <string.h>  // memset

/**** start inlining bitmap.inc.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2019, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
This file is meant to be included in other files for efficiency.
It implements a bitmap that can set/reset sequences of bits atomically
and is used to concurrently claim memory ranges.

A bitmap is an array of fields where each field is a machine word (`uintptr_t`)

A current limitation is that the bit sequences cannot cross fields
and that the sequence must be smaller or equal to the bits in a field.
---------------------------------------------------------------------------- */
#ifndef MI_BITMAP_C
#define MI_BITMAP_C

/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/

/* -----------------------------------------------------------
  Bitmap definition
----------------------------------------------------------- */

#define MI_BITMAP_FIELD_BITS   (8*MI_INTPTR_SIZE)
#define MI_BITMAP_FIELD_FULL   (~((uintptr_t)0))   // all bits set

// An atomic bitmap of `uintptr_t` fields
typedef volatile _Atomic(uintptr_t)  mi_bitmap_field_t;
typedef mi_bitmap_field_t*           mi_bitmap_t;

// A bitmap index is the index of the bit in a bitmap.
typedef size_t mi_bitmap_index_t;

// Create a bit index.
static inline mi_bitmap_index_t mi_bitmap_index_create(size_t idx, size_t bitidx) {
  mi_assert_internal(bitidx < MI_BITMAP_FIELD_BITS);
  return (idx*MI_BITMAP_FIELD_BITS) + bitidx;
}

// Get the field index from a bit index.
static inline size_t mi_bitmap_index_field(mi_bitmap_index_t bitmap_idx) {
  return (bitmap_idx / MI_BITMAP_FIELD_BITS);
}

// Get the bit index in a bitmap field
static inline size_t mi_bitmap_index_bit_in_field(mi_bitmap_index_t bitmap_idx) {
  return (bitmap_idx % MI_BITMAP_FIELD_BITS);
}

// Get the full bit index
static inline size_t mi_bitmap_index_bit(mi_bitmap_index_t bitmap_idx) {
  return bitmap_idx;
}


// The bit mask for a given number of blocks at a specified bit index.
static inline uintptr_t mi_bitmap_mask_(size_t count, size_t bitidx) {
  mi_assert_internal(count + bitidx <= MI_BITMAP_FIELD_BITS);
  if (count == MI_BITMAP_FIELD_BITS) return MI_BITMAP_FIELD_FULL;
  return ((((uintptr_t)1 << count) - 1) << bitidx);
}


/* -----------------------------------------------------------
  Use bit scan forward/reverse to quickly find the first zero bit if it is available
----------------------------------------------------------- */
#if defined(_MSC_VER)
#define MI_HAVE_BITSCAN
#include <intrin.h>
static inline size_t mi_bsf(uintptr_t x) {
  if (x==0) return 8*MI_INTPTR_SIZE;
  DWORD idx;
  MI_64(_BitScanForward)(&idx, x);
  return idx;
}
static inline size_t mi_bsr(uintptr_t x) {
  if (x==0) return 8*MI_INTPTR_SIZE;
  DWORD idx;
  MI_64(_BitScanReverse)(&idx, x);
  return idx;
}
#elif defined(__GNUC__) || defined(__clang__)
#include <limits.h> // LONG_MAX
#define MI_HAVE_BITSCAN
#if (INTPTR_MAX == LONG_MAX)
# define MI_L(x)  x##l
#else
# define MI_L(x)  x##ll
#endif
static inline size_t mi_bsf(uintptr_t x) {
  return (x==0 ? 8*MI_INTPTR_SIZE : MI_L(__builtin_ctz)(x));
}
static inline size_t mi_bsr(uintptr_t x) {
  return (x==0 ? 8*MI_INTPTR_SIZE : (8*MI_INTPTR_SIZE - 1) - MI_L(__builtin_clz)(x));
}
#endif

/* -----------------------------------------------------------
  Claim a bit sequence atomically
----------------------------------------------------------- */

// Try to atomically claim a sequence of `count` bits at in `idx`
// in the bitmap field. Returns `true` on success.
static inline bool mi_bitmap_try_claim_field(mi_bitmap_t bitmap, size_t bitmap_fields, const size_t count, mi_bitmap_index_t bitmap_idx) {
  const size_t idx = mi_bitmap_index_field(bitmap_idx);
  const size_t bitidx = mi_bitmap_index_bit_in_field(bitmap_idx);
  const uintptr_t mask = mi_bitmap_mask_(count, bitidx);
  mi_assert_internal(bitmap_fields > idx); UNUSED(bitmap_fields);
  mi_assert_internal(bitidx + count <= MI_BITMAP_FIELD_BITS);

  uintptr_t field = mi_atomic_read_relaxed(&bitmap[idx]);
  if ((field & mask) == 0) { // free?
    if (mi_atomic_cas_strong(&bitmap[idx], (field|mask), field)) {
      // claimed!
      return true;
    }
  }
  return false;
}


// Try to atomically claim a sequence of `count` bits in a single
// field at `idx` in `bitmap`. Returns `true` on success.
static inline bool mi_bitmap_try_find_claim_field(mi_bitmap_t bitmap, size_t idx, const size_t count, mi_bitmap_index_t* bitmap_idx)
{
  mi_assert_internal(bitmap_idx != NULL);
  volatile _Atomic(uintptr_t)* field = &bitmap[idx];
  uintptr_t map  = mi_atomic_read(field);
  if (map==MI_BITMAP_FIELD_FULL) return false; // short cut

  // search for 0-bit sequence of length count
  const uintptr_t mask = mi_bitmap_mask_(count, 0);
  const size_t    bitidx_max = MI_BITMAP_FIELD_BITS - count;

#ifdef MI_HAVE_BITSCAN
  size_t bitidx = mi_bsf(~map);    // quickly find the first zero bit if possible
#else
  size_t bitidx = 0;               // otherwise start at 0
#endif
  uintptr_t m = (mask << bitidx);     // invariant: m == mask shifted by bitidx

  // scan linearly for a free range of zero bits
  while (bitidx <= bitidx_max) {
    if ((map & m) == 0) {  // are the mask bits free at bitidx?
      mi_assert_internal((m >> bitidx) == mask); // no overflow?
      const uintptr_t newmap = map | m;
      mi_assert_internal((newmap^map) >> bitidx == mask);
      if (!mi_atomic_cas_weak(field, newmap, map)) {  // TODO: use strong cas here?
        // no success, another thread claimed concurrently.. keep going
        map = mi_atomic_read(field);
        continue;
      }
      else {
        // success, we claimed the bits!
        *bitmap_idx = mi_bitmap_index_create(idx, bitidx);
        return true;
      }
    }
    else {
      // on to the next bit range
#ifdef MI_HAVE_BITSCAN
      const size_t shift = (count == 1 ? 1 : mi_bsr(map & m) - bitidx + 1);
      mi_assert_internal(shift > 0 && shift <= count);
#else
      const size_t shift = 1;
#endif
      bitidx += shift;
      m <<= shift;
    }
  }
  // no bits found
  return false;
}


// Find `count` bits of 0 and set them to 1 atomically; returns `true` on success.
// For now, `count` can be at most MI_BITMAP_FIELD_BITS and will never span fields.
static inline bool mi_bitmap_try_find_claim(mi_bitmap_t bitmap, size_t bitmap_fields, size_t count, mi_bitmap_index_t* bitmap_idx) {
  for (size_t idx = 0; idx < bitmap_fields; idx++) {
    if (mi_bitmap_try_find_claim_field(bitmap, idx, count, bitmap_idx)) {
      return true;
    }
  }
  return false;
}

// Set `count` bits at `bitmap_idx` to 0 atomically
// Returns `true` if all `count` bits were 1 previously.
static inline bool mi_bitmap_unclaim(mi_bitmap_t bitmap, size_t bitmap_fields, size_t count, mi_bitmap_index_t bitmap_idx) {
  const size_t idx = mi_bitmap_index_field(bitmap_idx);
  const size_t bitidx = mi_bitmap_index_bit_in_field(bitmap_idx);
  const uintptr_t mask = mi_bitmap_mask_(count, bitidx);
  mi_assert_internal(bitmap_fields > idx); UNUSED(bitmap_fields);
  // mi_assert_internal((bitmap[idx] & mask) == mask);
  uintptr_t prev = mi_atomic_and(&bitmap[idx], ~mask);
  return ((prev & mask) == mask);
}


// Set `count` bits at `bitmap_idx` to 1 atomically
// Returns `true` if all `count` bits were 0 previously. `any_zero` is `true` if there was at least one zero bit.
static inline bool mi_bitmap_claim(mi_bitmap_t bitmap, size_t bitmap_fields, size_t count, mi_bitmap_index_t bitmap_idx, bool* any_zero) {
  const size_t idx = mi_bitmap_index_field(bitmap_idx);
  const size_t bitidx = mi_bitmap_index_bit_in_field(bitmap_idx);
  const uintptr_t mask = mi_bitmap_mask_(count, bitidx);
  mi_assert_internal(bitmap_fields > idx); UNUSED(bitmap_fields);
  //mi_assert_internal(any_zero != NULL || (bitmap[idx] & mask) == 0);
  uintptr_t prev = mi_atomic_or(&bitmap[idx], mask);
  if (any_zero != NULL) *any_zero = ((prev & mask) != mask);
  return ((prev & mask) == 0);
}

// Returns `true` if all `count` bits were 1. `any_ones` is `true` if there was at least one bit set to one.
static inline bool mi_bitmap_is_claimedx(mi_bitmap_t bitmap, size_t bitmap_fields, size_t count, mi_bitmap_index_t bitmap_idx, bool* any_ones) {
  const size_t idx = mi_bitmap_index_field(bitmap_idx);
  const size_t bitidx = mi_bitmap_index_bit_in_field(bitmap_idx);
  const uintptr_t mask = mi_bitmap_mask_(count, bitidx);
  mi_assert_internal(bitmap_fields > idx); UNUSED(bitmap_fields);
  uintptr_t field = mi_atomic_read_relaxed(&bitmap[idx]);
  if (any_ones != NULL) *any_ones = ((field & mask) != 0);
  return ((field & mask) == mask);
}

static inline bool mi_bitmap_is_claimed(mi_bitmap_t bitmap, size_t bitmap_fields, size_t count, mi_bitmap_index_t bitmap_idx) {
  return mi_bitmap_is_claimedx(bitmap, bitmap_fields, count, bitmap_idx, NULL);
}

static inline bool mi_bitmap_is_any_claimed(mi_bitmap_t bitmap, size_t bitmap_fields, size_t count, mi_bitmap_index_t bitmap_idx) {
  bool any_ones;
  mi_bitmap_is_claimedx(bitmap, bitmap_fields, count, bitmap_idx, &any_ones);
  return any_ones;
}


#endif
/**** ended inlining bitmap.inc.c ****/


// os.c
void* _mi_os_alloc_aligned(size_t size, size_t alignment, bool commit, bool* large, mi_os_tld_t* tld);
void  _mi_os_free_ex(void* p, size_t size, bool was_committed, mi_stats_t* stats);
void  _mi_os_free(void* p, size_t size, mi_stats_t* stats);

void* _mi_os_alloc_huge_os_pages(size_t pages, int numa_node, mi_msecs_t max_secs, size_t* pages_reserved, size_t* psize);
void  _mi_os_free_huge_pages(void* p, size_t size, mi_stats_t* stats);

bool  _mi_os_commit(void* p, size_t size, bool* is_zero, mi_stats_t* stats);

/* -----------------------------------------------------------
  Arena allocation
----------------------------------------------------------- */

#define MI_SEGMENT_ALIGN      MI_SEGMENT_SIZE
#define MI_ARENA_BLOCK_SIZE   (8*MI_SEGMENT_ALIGN)     // 32MiB
#define MI_ARENA_MAX_OBJ_SIZE (MI_BITMAP_FIELD_BITS * MI_ARENA_BLOCK_SIZE)  // 2GiB
#define MI_ARENA_MIN_OBJ_SIZE (MI_ARENA_BLOCK_SIZE/2)  // 16MiB
#define MI_MAX_ARENAS         (64)                     // not more than 256 (since we use 8 bits in the memid)

// A memory arena descriptor
typedef struct mi_arena_s {
  _Atomic(uint8_t*) start;                // the start of the memory area
  size_t   block_count;                   // size of the area in arena blocks (of `MI_ARENA_BLOCK_SIZE`)
  size_t   field_count;                   // number of bitmap fields (where `field_count * MI_BITMAP_FIELD_BITS >= block_count`)
  int      numa_node;                     // associated NUMA node
  bool     is_zero_init;                  // is the arena zero initialized?
  bool     is_committed;                  // is the memory committed
  bool     is_large;                      // large OS page allocated
  volatile _Atomic(uintptr_t) search_idx; // optimization to start the search for free blocks
  mi_bitmap_field_t* blocks_dirty;        // are the blocks potentially non-zero?
  mi_bitmap_field_t* blocks_committed;    // if `!is_committed`, are the blocks committed?
  mi_bitmap_field_t  blocks_inuse[1];       // in-place bitmap of in-use blocks (of size `field_count`)
} mi_arena_t;


// The available arenas
static mi_decl_cache_align _Atomic(mi_arena_t*) mi_arenas[MI_MAX_ARENAS];
static mi_decl_cache_align _Atomic(uintptr_t)   mi_arena_count; // = 0


/* -----------------------------------------------------------
  Arena allocations get a memory id where the lower 8 bits are
  the arena index +1, and the upper bits the block index.
----------------------------------------------------------- */

// Use `0` as a special id for direct OS allocated memory.
#define MI_MEMID_OS   0

static size_t mi_arena_id_create(size_t arena_index, mi_bitmap_index_t bitmap_index) {
  mi_assert_internal(arena_index < 0xFE);
  mi_assert_internal(((bitmap_index << 8) >> 8) == bitmap_index); // no overflow?
  return ((bitmap_index << 8) | ((arena_index+1) & 0xFF));
}

static void mi_arena_id_indices(size_t memid, size_t* arena_index, mi_bitmap_index_t* bitmap_index) {
  mi_assert_internal(memid != MI_MEMID_OS);
  *arena_index = (memid & 0xFF) - 1;
  *bitmap_index = (memid >> 8);
}

static size_t mi_block_count_of_size(size_t size) {
  return _mi_divide_up(size, MI_ARENA_BLOCK_SIZE);
}

/* -----------------------------------------------------------
  Thread safe allocation in an arena
----------------------------------------------------------- */
static bool mi_arena_alloc(mi_arena_t* arena, size_t blocks, mi_bitmap_index_t* bitmap_idx)
{
  const size_t fcount = arena->field_count;
  size_t idx = mi_atomic_read(&arena->search_idx);  // start from last search
  for (size_t visited = 0; visited < fcount; visited++, idx++) {
    if (idx >= fcount) idx = 0;  // wrap around
    // try to atomically claim a range of bits
    if (mi_bitmap_try_find_claim_field(arena->blocks_inuse, idx, blocks, bitmap_idx)) {
      mi_atomic_write(&arena->search_idx, idx);  // start search from here next time
      return true;
    }
  }
  return false;
}


/* -----------------------------------------------------------
  Arena Allocation
----------------------------------------------------------- */

static void* mi_arena_alloc_from(mi_arena_t* arena, size_t arena_index, size_t needed_bcount,
                                 bool* commit, bool* large, bool* is_zero, size_t* memid, mi_os_tld_t* tld)
{
  mi_bitmap_index_t bitmap_index;
  if (!mi_arena_alloc(arena, needed_bcount, &bitmap_index)) return NULL;

  // claimed it! set the dirty bits (todo: no need for an atomic op here?)
  void* p  = arena->start + (mi_bitmap_index_bit(bitmap_index)*MI_ARENA_BLOCK_SIZE);
  *memid   = mi_arena_id_create(arena_index, bitmap_index);
  *is_zero = mi_bitmap_claim(arena->blocks_dirty, arena->field_count, needed_bcount, bitmap_index, NULL);
  *large   = arena->is_large;
  if (arena->is_committed) {
    // always committed
    *commit = true;
  }
  else if (*commit) {
    // arena not committed as a whole, but commit requested: ensure commit now
    bool any_uncommitted;
    mi_bitmap_claim(arena->blocks_committed, arena->field_count, needed_bcount, bitmap_index, &any_uncommitted);
    if (any_uncommitted) {
      bool commit_zero;
      _mi_os_commit(p, needed_bcount * MI_ARENA_BLOCK_SIZE, &commit_zero, tld->stats);
      if (commit_zero) *is_zero = true;
    }
  }
  else {
    // no need to commit, but check if already fully committed
    *commit = mi_bitmap_is_claimed(arena->blocks_committed, arena->field_count, needed_bcount, bitmap_index);
  }
  return p;
}

void* _mi_arena_alloc_aligned(size_t size, size_t alignment,
                              bool* commit, bool* large, bool* is_zero,
                              size_t* memid, mi_os_tld_t* tld)
{
  mi_assert_internal(commit != NULL && large != NULL && is_zero != NULL && memid != NULL && tld != NULL);
  mi_assert_internal(size > 0);
  *memid   = MI_MEMID_OS;
  *is_zero = false;

  // try to allocate in an arena if the alignment is small enough
  // and the object is not too large or too small.
  if (alignment <= MI_SEGMENT_ALIGN &&
      size <= MI_ARENA_MAX_OBJ_SIZE &&
      size >= MI_ARENA_MIN_OBJ_SIZE)
  {
    const size_t bcount = mi_block_count_of_size(size);
    const int numa_node = _mi_os_numa_node(tld); // current numa node

    mi_assert_internal(size <= bcount*MI_ARENA_BLOCK_SIZE);
    // try numa affine allocation
    for (size_t i = 0; i < MI_MAX_ARENAS; i++) {
      mi_arena_t* arena = mi_atomic_read_ptr_relaxed(mi_arena_t, &mi_arenas[i]);
      if (arena==NULL) break; // end reached
      if ((arena->numa_node<0 || arena->numa_node==numa_node) && // numa local?
          (*large || !arena->is_large)) // large OS pages allowed, or arena is not large OS pages
      {
        void* p = mi_arena_alloc_from(arena, i, bcount, commit, large, is_zero, memid, tld);
        mi_assert_internal((uintptr_t)p % alignment == 0);
        if (p != NULL) return p;
      }
    }
    // try from another numa node instead..
    for (size_t i = 0; i < MI_MAX_ARENAS; i++) {
      mi_arena_t* arena = mi_atomic_read_ptr_relaxed(mi_arena_t, &mi_arenas[i]);
      if (arena==NULL) break; // end reached
      if ((arena->numa_node>=0 && arena->numa_node!=numa_node) && // not numa local!
          (*large || !arena->is_large)) // large OS pages allowed, or arena is not large OS pages
      {
        void* p = mi_arena_alloc_from(arena, i, bcount, commit, large, is_zero, memid, tld);
        mi_assert_internal((uintptr_t)p % alignment == 0);
        if (p != NULL) return p;
      }
    }
  }

  // finally, fall back to the OS
  *is_zero = true;
  *memid   = MI_MEMID_OS;
  return _mi_os_alloc_aligned(size, alignment, *commit, large, tld);
}

void* _mi_arena_alloc(size_t size, bool* commit, bool* large, bool* is_zero, size_t* memid, mi_os_tld_t* tld)
{
  return _mi_arena_alloc_aligned(size, MI_ARENA_BLOCK_SIZE, commit, large, is_zero, memid, tld);
}

/* -----------------------------------------------------------
  Arena free
----------------------------------------------------------- */

void _mi_arena_free(void* p, size_t size, size_t memid, bool all_committed, mi_stats_t* stats) {
  mi_assert_internal(size > 0 && stats != NULL);
  if (p==NULL) return;
  if (size==0) return;
  if (memid == MI_MEMID_OS) {
    // was a direct OS allocation, pass through
    _mi_os_free_ex(p, size, all_committed, stats);
  }
  else {
    // allocated in an arena
    size_t arena_idx;
    size_t bitmap_idx;
    mi_arena_id_indices(memid, &arena_idx, &bitmap_idx);
    mi_assert_internal(arena_idx < MI_MAX_ARENAS);
    mi_arena_t* arena = mi_atomic_read_ptr_relaxed(mi_arena_t,&mi_arenas[arena_idx]);
    mi_assert_internal(arena != NULL);
    if (arena == NULL) {
      _mi_error_message(EINVAL, "trying to free from non-existent arena: %p, size %zu, memid: 0x%zx\n", p, size, memid);
      return;
    }
    mi_assert_internal(arena->field_count > mi_bitmap_index_field(bitmap_idx));
    if (arena->field_count <= mi_bitmap_index_field(bitmap_idx)) {
      _mi_error_message(EINVAL, "trying to free from non-existent arena block: %p, size %zu, memid: 0x%zx\n", p, size, memid);
      return;
    }
    const size_t blocks = mi_block_count_of_size(size);
    bool ones = mi_bitmap_unclaim(arena->blocks_inuse, arena->field_count, blocks, bitmap_idx);
    if (!ones) {
      _mi_error_message(EAGAIN, "trying to free an already freed block: %p, size %zu\n", p, size);
      return;
    };
  }
}

/* -----------------------------------------------------------
  Add an arena.
----------------------------------------------------------- */

static bool mi_arena_add(mi_arena_t* arena) {
  mi_assert_internal(arena != NULL);
  mi_assert_internal((uintptr_t)mi_atomic_read_ptr_relaxed(uint8_t,&arena->start) % MI_SEGMENT_ALIGN == 0);
  mi_assert_internal(arena->block_count > 0);

  uintptr_t i = mi_atomic_increment(&mi_arena_count);
  if (i >= MI_MAX_ARENAS) {
    mi_atomic_decrement(&mi_arena_count);
    return false;
  }
  mi_atomic_write_ptr(mi_arena_t,&mi_arenas[i], arena);
  return true;
}


/* -----------------------------------------------------------
  Reserve a huge page arena.
----------------------------------------------------------- */
#include <errno.h> // ENOMEM

// reserve at a specific numa node
int mi_reserve_huge_os_pages_at(size_t pages, int numa_node, size_t timeout_msecs) mi_attr_noexcept {
  if (pages==0) return 0;
  if (numa_node < -1) numa_node = -1;
  if (numa_node >= 0) numa_node = numa_node % _mi_os_numa_node_count();
  size_t hsize = 0;
  size_t pages_reserved = 0;
  void* p = _mi_os_alloc_huge_os_pages(pages, numa_node, timeout_msecs, &pages_reserved, &hsize);
  if (p==NULL || pages_reserved==0) {
    _mi_warning_message("failed to reserve %zu gb huge pages\n", pages);
    return ENOMEM;
  }
  _mi_verbose_message("numa node %i: reserved %zu gb huge pages (of the %zu gb requested)\n", numa_node, pages_reserved, pages);

  size_t bcount = mi_block_count_of_size(hsize);
  size_t fields = _mi_divide_up(bcount, MI_BITMAP_FIELD_BITS);
  size_t asize = sizeof(mi_arena_t) + (2*fields*sizeof(mi_bitmap_field_t));
  mi_arena_t* arena = (mi_arena_t*)_mi_os_alloc(asize, &_mi_stats_main); // TODO: can we avoid allocating from the OS?
  if (arena == NULL) {
    _mi_os_free_huge_pages(p, hsize, &_mi_stats_main);
    return ENOMEM;
  }
  arena->block_count = bcount;
  arena->field_count = fields;
  arena->start = (uint8_t*)p;
  arena->numa_node = numa_node; // TODO: or get the current numa node if -1? (now it allows anyone to allocate on -1)
  arena->is_large = true;
  arena->is_zero_init = true;
  arena->is_committed = true;
  arena->search_idx = 0;
  arena->blocks_dirty = &arena->blocks_inuse[fields]; // just after inuse bitmap
  arena->blocks_committed = NULL;
  // the bitmaps are already zero initialized due to os_alloc
  // just claim leftover blocks if needed
  ptrdiff_t post = (fields * MI_BITMAP_FIELD_BITS) - bcount;
  mi_assert_internal(post >= 0);
  if (post > 0) {
    // don't use leftover bits at the end
    mi_bitmap_index_t postidx = mi_bitmap_index_create(fields - 1, MI_BITMAP_FIELD_BITS - post);
    mi_bitmap_claim(arena->blocks_inuse, fields, post, postidx, NULL);
  }

  mi_arena_add(arena);
  return 0;
}


// reserve huge pages evenly among the given number of numa nodes (or use the available ones as detected)
int mi_reserve_huge_os_pages_interleave(size_t pages, size_t numa_nodes, size_t timeout_msecs) mi_attr_noexcept {
  if (pages == 0) return 0;

  // pages per numa node
  size_t numa_count = (numa_nodes > 0 ? numa_nodes : _mi_os_numa_node_count());
  if (numa_count <= 0) numa_count = 1;
  const size_t pages_per = pages / numa_count;
  const size_t pages_mod = pages % numa_count;
  const size_t timeout_per = (timeout_msecs==0 ? 0 : (timeout_msecs / numa_count) + 50);

  // reserve evenly among numa nodes
  for (size_t numa_node = 0; numa_node < numa_count && pages > 0; numa_node++) {
    size_t node_pages = pages_per;  // can be 0
    if (numa_node < pages_mod) node_pages++;
    int err = mi_reserve_huge_os_pages_at(node_pages, (int)numa_node, timeout_per);
    if (err) return err;
    if (pages < node_pages) {
      pages = 0;
    }
    else {
      pages -= node_pages;
    }
  }

  return 0;
}

int mi_reserve_huge_os_pages(size_t pages, double max_secs, size_t* pages_reserved) mi_attr_noexcept {
  UNUSED(max_secs);
  _mi_warning_message("mi_reserve_huge_os_pages is deprecated: use mi_reserve_huge_os_pages_interleave/at instead\n");
  if (pages_reserved != NULL) *pages_reserved = 0;
  int err = mi_reserve_huge_os_pages_interleave(pages, 0, (size_t)(max_secs * 1000.0));
  if (err==0 && pages_reserved!=NULL) *pages_reserved = pages;
  return err;
}
/**** ended inlining arena.c ****/
/**** start inlining region.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2019, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
This implements a layer between the raw OS memory (VirtualAlloc/mmap/sbrk/..)
and the segment and huge object allocation by mimalloc. There may be multiple
implementations of this (one could be the identity going directly to the OS,
another could be a simple cache etc), but the current one uses large "regions".
In contrast to the rest of mimalloc, the "regions" are shared between threads and
need to be accessed using atomic operations.
We need this memory layer between the raw OS calls because of:
1. on `sbrk` like systems (like WebAssembly) we need our own memory maps in order
   to reuse memory effectively.
2. It turns out that for large objects, between 1MiB and 32MiB (?), the cost of
   an OS allocation/free is still (much) too expensive relative to the accesses 
   in that object :-( (`malloc-large` tests this). This means we need a cheaper 
   way to reuse memory.
3. This layer allows for NUMA aware allocation.

Possible issues:
- (2) can potentially be addressed too with a small cache per thread which is much
  simpler. Generally though that requires shrinking of huge pages, and may overuse
  memory per thread. (and is not compatible with `sbrk`).
- Since the current regions are per-process, we need atomic operations to
  claim blocks which may be contended
- In the worst case, we need to search the whole region map (16KiB for 256GiB)
  linearly. At what point will direct OS calls be faster? Is there a way to
  do this better without adding too much complexity?
-----------------------------------------------------------------------------*/
/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/
/**** skipping file: mimalloc-atomic.h ****/

#include <string.h>  // memset

/**** skipping file: bitmap.inc.c ****/

// Internal raw OS interface
size_t  _mi_os_large_page_size();
bool    _mi_os_protect(void* addr, size_t size);
bool    _mi_os_unprotect(void* addr, size_t size);
bool    _mi_os_commit(void* p, size_t size, bool* is_zero, mi_stats_t* stats);
bool    _mi_os_decommit(void* p, size_t size, mi_stats_t* stats);
bool    _mi_os_reset(void* p, size_t size, mi_stats_t* stats);
bool    _mi_os_unreset(void* p, size_t size, bool* is_zero, mi_stats_t* stats);

// arena.c
void    _mi_arena_free(void* p, size_t size, size_t memid, bool all_committed, mi_stats_t* stats);
void*   _mi_arena_alloc(size_t size, bool* commit, bool* large, bool* is_zero, size_t* memid, mi_os_tld_t* tld);
void*   _mi_arena_alloc_aligned(size_t size, size_t alignment, bool* commit, bool* large, bool* is_zero, size_t* memid, mi_os_tld_t* tld);



// Constants
#if (MI_INTPTR_SIZE==8)
#define MI_HEAP_REGION_MAX_SIZE    (256 * GiB)  // 64KiB for the region map 
#elif (MI_INTPTR_SIZE==4)
#define MI_HEAP_REGION_MAX_SIZE    (3 * GiB)    // ~ KiB for the region map
#else
#error "define the maximum heap space allowed for regions on this platform"
#endif

#define MI_SEGMENT_ALIGN          MI_SEGMENT_SIZE

#define MI_REGION_MAX_BLOCKS      MI_BITMAP_FIELD_BITS
#define MI_REGION_SIZE            (MI_SEGMENT_SIZE * MI_BITMAP_FIELD_BITS)    // 256MiB  (64MiB on 32 bits)
#define MI_REGION_MAX             (MI_HEAP_REGION_MAX_SIZE / MI_REGION_SIZE)  // 1024  (48 on 32 bits)
#define MI_REGION_MAX_OBJ_BLOCKS  (MI_REGION_MAX_BLOCKS/4)                    // 64MiB
#define MI_REGION_MAX_OBJ_SIZE    (MI_REGION_MAX_OBJ_BLOCKS*MI_SEGMENT_SIZE)  

// Region info 
typedef union mi_region_info_u {
  uintptr_t value;      
  struct {
    bool  valid;        // initialized?
    bool  is_large;     // allocated in fixed large/huge OS pages
    short numa_node;    // the associated NUMA node (where -1 means no associated node)
  } x;
} mi_region_info_t;


// A region owns a chunk of REGION_SIZE (256MiB) (virtual) memory with
// a bit map with one bit per MI_SEGMENT_SIZE (4MiB) block.
typedef struct mem_region_s {
  volatile _Atomic(uintptr_t)        info;        // mi_region_info_t.value
  volatile _Atomic(void*)            start;       // start of the memory area 
  mi_bitmap_field_t                  in_use;      // bit per in-use block
  mi_bitmap_field_t                  dirty;       // track if non-zero per block
  mi_bitmap_field_t                  commit;      // track if committed per block
  mi_bitmap_field_t                  reset;       // track if reset per block
  volatile _Atomic(uintptr_t)        arena_memid; // if allocated from a (huge page) arena
  uintptr_t                          padding;     // round to 8 fields
} mem_region_t;

// The region map
static mem_region_t regions[MI_REGION_MAX];

// Allocated regions
static volatile _Atomic(uintptr_t) regions_count; // = 0;        


/* ----------------------------------------------------------------------------
Utility functions
-----------------------------------------------------------------------------*/

// Blocks (of 4MiB) needed for the given size.
static size_t mi_region_block_count(size_t size) {
  return _mi_divide_up(size, MI_SEGMENT_SIZE);
}

/*
// Return a rounded commit/reset size such that we don't fragment large OS pages into small ones.
static size_t mi_good_commit_size(size_t size) {
  if (size > (SIZE_MAX - _mi_os_large_page_size())) return size;
  return _mi_align_up(size, _mi_os_large_page_size());
}
*/

// Return if a pointer points into a region reserved by us.
bool mi_is_in_heap_region(const void* p) mi_attr_noexcept {
  if (p==NULL) return false;
  size_t count = mi_atomic_read_relaxed(&regions_count);
  for (size_t i = 0; i < count; i++) {
    uint8_t* start = mi_atomic_read_ptr_relaxed(uint8_t,&regions[i].start);
    if (start != NULL && (uint8_t*)p >= start && (uint8_t*)p < start + MI_REGION_SIZE) return true;
  }
  return false;
}


static void* mi_region_blocks_start(const mem_region_t* region, mi_bitmap_index_t bit_idx) {
  uint8_t* start = mi_atomic_read_ptr(uint8_t,&region->start);
  mi_assert_internal(start != NULL);
  return (start + (bit_idx * MI_SEGMENT_SIZE));  
}

static size_t mi_memid_create(mem_region_t* region, mi_bitmap_index_t bit_idx) {
  mi_assert_internal(bit_idx < MI_BITMAP_FIELD_BITS);
  size_t idx = region - regions;
  mi_assert_internal(&regions[idx] == region);
  return (idx*MI_BITMAP_FIELD_BITS + bit_idx)<<1;
}

static size_t mi_memid_create_from_arena(size_t arena_memid) {
  return (arena_memid << 1) | 1;
}


static bool mi_memid_is_arena(size_t id, mem_region_t** region, mi_bitmap_index_t* bit_idx, size_t* arena_memid) {
  if ((id&1)==1) {
    if (arena_memid != NULL) *arena_memid = (id>>1);
    return true;
  }
  else {
    size_t idx = (id >> 1) / MI_BITMAP_FIELD_BITS;
    *bit_idx   = (mi_bitmap_index_t)(id>>1) % MI_BITMAP_FIELD_BITS;
    *region    = &regions[idx];
    return false;
  }
}


/* ----------------------------------------------------------------------------
  Allocate a region is allocated from the OS (or an arena)
-----------------------------------------------------------------------------*/

static bool mi_region_try_alloc_os(size_t blocks, bool commit, bool allow_large, mem_region_t** region, mi_bitmap_index_t* bit_idx, mi_os_tld_t* tld)
{
  // not out of regions yet?
  if (mi_atomic_read_relaxed(&regions_count) >= MI_REGION_MAX - 1) return false;

  // try to allocate a fresh region from the OS
  bool region_commit = (commit && mi_option_is_enabled(mi_option_eager_region_commit));
  bool region_large = (commit && allow_large);
  bool is_zero = false;
  size_t arena_memid = 0;
  void* const start = _mi_arena_alloc_aligned(MI_REGION_SIZE, MI_SEGMENT_ALIGN, &region_commit, &region_large, &is_zero, &arena_memid, tld);
  if (start == NULL) return false;
  mi_assert_internal(!(region_large && !allow_large));
  mi_assert_internal(!region_large || region_commit);

  // claim a fresh slot
  const uintptr_t idx = mi_atomic_increment(&regions_count);
  if (idx >= MI_REGION_MAX) {
    mi_atomic_decrement(&regions_count);
    _mi_arena_free(start, MI_REGION_SIZE, arena_memid, region_commit, tld->stats);
    _mi_warning_message("maximum regions used: %zu GiB (perhaps recompile with a larger setting for MI_HEAP_REGION_MAX_SIZE)", _mi_divide_up(MI_HEAP_REGION_MAX_SIZE, GiB));
    return false;
  }

  // allocated, initialize and claim the initial blocks
  mem_region_t* r = &regions[idx];
  r->arena_memid  = arena_memid;
  mi_atomic_write(&r->in_use, 0);
  mi_atomic_write(&r->dirty, (is_zero ? 0 : MI_BITMAP_FIELD_FULL));
  mi_atomic_write(&r->commit, (region_commit ? MI_BITMAP_FIELD_FULL : 0));
  mi_atomic_write(&r->reset, 0);
  *bit_idx = 0;
  mi_bitmap_claim(&r->in_use, 1, blocks, *bit_idx, NULL);
  mi_atomic_write_ptr(uint8_t*,&r->start, start);

  // and share it 
  mi_region_info_t info;
  info.value = 0;                        // initialize the full union to zero
  info.x.valid = true;
  info.x.is_large = region_large;
  info.x.numa_node = (short)_mi_os_numa_node(tld);
  mi_atomic_write(&r->info, info.value); // now make it available to others
  *region = r;
  return true;
}

/* ----------------------------------------------------------------------------
  Try to claim blocks in suitable regions
-----------------------------------------------------------------------------*/

static bool mi_region_is_suitable(const mem_region_t* region, int numa_node, bool allow_large ) {
  // initialized at all?
  mi_region_info_t info;
  info.value = mi_atomic_read_relaxed(&region->info);
  if (info.value==0) return false;

  // numa correct
  if (numa_node >= 0) {  // use negative numa node to always succeed
    int rnode = info.x.numa_node;
    if (rnode >= 0 && rnode != numa_node) return false;
  }

  // check allow-large
  if (!allow_large && info.x.is_large) return false;

  return true;
}


static bool mi_region_try_claim(int numa_node, size_t blocks, bool allow_large, mem_region_t** region, mi_bitmap_index_t* bit_idx, mi_os_tld_t* tld)
{
  // try all regions for a free slot  
  const size_t count = mi_atomic_read(&regions_count);
  size_t idx = tld->region_idx; // Or start at 0 to reuse low addresses? Starting at 0 seems to increase latency though
  for (size_t visited = 0; visited < count; visited++, idx++) {
    if (idx >= count) idx = 0;  // wrap around
    mem_region_t* r = &regions[idx];
    // if this region suits our demand (numa node matches, large OS page matches)
    if (mi_region_is_suitable(r, numa_node, allow_large)) {
      // then try to atomically claim a segment(s) in this region
      if (mi_bitmap_try_find_claim_field(&r->in_use, 0, blocks, bit_idx)) {
        tld->region_idx = idx;    // remember the last found position
        *region = r;
        return true;
      }
    }
  }
  return false;
}


static void* mi_region_try_alloc(size_t blocks, bool* commit, bool* is_large, bool* is_zero, size_t* memid, mi_os_tld_t* tld)
{
  mi_assert_internal(blocks <= MI_BITMAP_FIELD_BITS);
  mem_region_t* region;
  mi_bitmap_index_t bit_idx;
  const int numa_node = (_mi_os_numa_node_count() <= 1 ? -1 : _mi_os_numa_node(tld));
  // try to claim in existing regions
  if (!mi_region_try_claim(numa_node, blocks, *is_large, &region, &bit_idx, tld)) {
    // otherwise try to allocate a fresh region and claim in there
    if (!mi_region_try_alloc_os(blocks, *commit, *is_large, &region, &bit_idx, tld)) {
      // out of regions or memory
      return NULL;
    }
  }
  
  // ------------------------------------------------
  // found a region and claimed `blocks` at `bit_idx`, initialize them now
  mi_assert_internal(region != NULL);
  mi_assert_internal(mi_bitmap_is_claimed(&region->in_use, 1, blocks, bit_idx));

  mi_region_info_t info;
  info.value = mi_atomic_read(&region->info);
  uint8_t* start = mi_atomic_read_ptr(uint8_t,&region->start);
  mi_assert_internal(!(info.x.is_large && !*is_large));
  mi_assert_internal(start != NULL);

  *is_zero = mi_bitmap_claim(&region->dirty, 1, blocks, bit_idx, NULL);  
  *is_large = info.x.is_large;
  *memid = mi_memid_create(region, bit_idx);
  void* p = start + (mi_bitmap_index_bit_in_field(bit_idx) * MI_SEGMENT_SIZE);

  // commit
  if (*commit) {
    // ensure commit
    bool any_uncommitted;
    mi_bitmap_claim(&region->commit, 1, blocks, bit_idx, &any_uncommitted);
    if (any_uncommitted) {
      mi_assert_internal(!info.x.is_large);
      bool commit_zero;
      _mi_mem_commit(p, blocks * MI_SEGMENT_SIZE, &commit_zero, tld);
      if (commit_zero) *is_zero = true;
    }
  }
  else {
    // no need to commit, but check if already fully committed
    *commit = mi_bitmap_is_claimed(&region->commit, 1, blocks, bit_idx);
  }  
  mi_assert_internal(!*commit || mi_bitmap_is_claimed(&region->commit, 1, blocks, bit_idx));

  // unreset reset blocks
  if (mi_bitmap_is_any_claimed(&region->reset, 1, blocks, bit_idx)) {
    // some blocks are still reset
    mi_assert_internal(!info.x.is_large);
    mi_assert_internal(!mi_option_is_enabled(mi_option_eager_commit) || *commit || mi_option_get(mi_option_eager_commit_delay) > 0); 
    mi_bitmap_unclaim(&region->reset, 1, blocks, bit_idx);
    if (*commit || !mi_option_is_enabled(mi_option_reset_decommits)) { // only if needed
      bool reset_zero = false;
      _mi_mem_unreset(p, blocks * MI_SEGMENT_SIZE, &reset_zero, tld);
      if (reset_zero) *is_zero = true;
    }
  }
  mi_assert_internal(!mi_bitmap_is_any_claimed(&region->reset, 1, blocks, bit_idx));
  
  #if (MI_DEBUG>=2)
  if (*commit) { ((uint8_t*)p)[0] = 0; }
  #endif
  
  // and return the allocation  
  mi_assert_internal(p != NULL);  
  return p;
}


/* ----------------------------------------------------------------------------
 Allocation
-----------------------------------------------------------------------------*/

// Allocate `size` memory aligned at `alignment`. Return non NULL on success, with a given memory `id`.
// (`id` is abstract, but `id = idx*MI_REGION_MAP_BITS + bitidx`)
void* _mi_mem_alloc_aligned(size_t size, size_t alignment, bool* commit, bool* large, bool* is_zero, size_t* memid, mi_os_tld_t* tld)
{
  mi_assert_internal(memid != NULL && tld != NULL);
  mi_assert_internal(size > 0);
  *memid = 0;
  *is_zero = false;
  bool default_large = false;
  if (large==NULL) large = &default_large;  // ensure `large != NULL`  
  if (size == 0) return NULL;
  size = _mi_align_up(size, _mi_os_page_size());

  // allocate from regions if possible
  void* p = NULL;
  size_t arena_memid;
  const size_t blocks = mi_region_block_count(size);
  if (blocks <= MI_REGION_MAX_OBJ_BLOCKS && alignment <= MI_SEGMENT_ALIGN) {
    p = mi_region_try_alloc(blocks, commit, large, is_zero, memid, tld);    
    if (p == NULL) {
      _mi_warning_message("unable to allocate from region: size %zu\n", size);
    }
  }
  if (p == NULL) {
    // and otherwise fall back to the OS
    p = _mi_arena_alloc_aligned(size, alignment, commit, large, is_zero, &arena_memid, tld);
    *memid = mi_memid_create_from_arena(arena_memid);
  }

  if (p != NULL) {
    mi_assert_internal((uintptr_t)p % alignment == 0);
#if (MI_DEBUG>=2)
    if (*commit) { ((uint8_t*)p)[0] = 0; } // ensure the memory is committed
#endif
  }
  return p;
}



/* ----------------------------------------------------------------------------
Free
-----------------------------------------------------------------------------*/

// Free previously allocated memory with a given id.
void _mi_mem_free(void* p, size_t size, size_t id, bool full_commit, bool any_reset, mi_os_tld_t* tld) {
  mi_assert_internal(size > 0 && tld != NULL);
  if (p==NULL) return;
  if (size==0) return;
  size = _mi_align_up(size, _mi_os_page_size());
  
  size_t arena_memid = 0;
  mi_bitmap_index_t bit_idx;
  mem_region_t* region;
  if (mi_memid_is_arena(id,&region,&bit_idx,&arena_memid)) {
   // was a direct arena allocation, pass through
    _mi_arena_free(p, size, arena_memid, full_commit, tld->stats);
  }
  else {
    // allocated in a region
    mi_assert_internal(size <= MI_REGION_MAX_OBJ_SIZE); if (size > MI_REGION_MAX_OBJ_SIZE) return;
    const size_t blocks = mi_region_block_count(size);
    mi_assert_internal(blocks + bit_idx <= MI_BITMAP_FIELD_BITS);
    mi_region_info_t info;
    info.value = mi_atomic_read(&region->info);
    mi_assert_internal(info.value != 0);
    void* blocks_start = mi_region_blocks_start(region, bit_idx);
    mi_assert_internal(blocks_start == p); // not a pointer in our area?
    mi_assert_internal(bit_idx + blocks <= MI_BITMAP_FIELD_BITS);
    if (blocks_start != p || bit_idx + blocks > MI_BITMAP_FIELD_BITS) return; // or `abort`?

    // committed?
    if (full_commit && (size % MI_SEGMENT_SIZE) == 0) {
      mi_bitmap_claim(&region->commit, 1, blocks, bit_idx, NULL);
    }

    if (any_reset) {
      // set the is_reset bits if any pages were reset
      mi_bitmap_claim(&region->reset, 1, blocks, bit_idx, NULL);
    }

    // reset the blocks to reduce the working set.
    if (!info.x.is_large && mi_option_is_enabled(mi_option_segment_reset) 
       && (mi_option_is_enabled(mi_option_eager_commit) ||
           mi_option_is_enabled(mi_option_reset_decommits))) // cannot reset halfway committed segments, use only `option_page_reset` instead            
    {
      bool any_unreset;
      mi_bitmap_claim(&region->reset, 1, blocks, bit_idx, &any_unreset);
      if (any_unreset) {
        _mi_abandoned_await_readers(); // ensure no more pending write (in case reset = decommit)
        _mi_mem_reset(p, blocks * MI_SEGMENT_SIZE, tld);
      }
    }    

    // and unclaim
    bool all_unclaimed = mi_bitmap_unclaim(&region->in_use, 1, blocks, bit_idx);
    mi_assert_internal(all_unclaimed); UNUSED(all_unclaimed);
  }
}


/* ----------------------------------------------------------------------------
  collection
-----------------------------------------------------------------------------*/
void _mi_mem_collect(mi_os_tld_t* tld) {
  // free every region that has no segments in use.
  uintptr_t rcount = mi_atomic_read_relaxed(&regions_count);
  for (size_t i = 0; i < rcount; i++) {
    mem_region_t* region = &regions[i];
    if (mi_atomic_read_relaxed(&region->info) != 0) {
      // if no segments used, try to claim the whole region
      uintptr_t m;
      do {
        m = mi_atomic_read_relaxed(&region->in_use);
      } while(m == 0 && !mi_atomic_cas_weak(&region->in_use, MI_BITMAP_FIELD_FULL, 0 ));
      if (m == 0) {
        // on success, free the whole region
        uint8_t* start = mi_atomic_read_ptr(uint8_t,&regions[i].start);
        size_t arena_memid = mi_atomic_read_relaxed(&regions[i].arena_memid);
        uintptr_t commit = mi_atomic_read_relaxed(&regions[i].commit);
        memset(&regions[i], 0, sizeof(mem_region_t));
        // and release the whole region
        mi_atomic_write(&region->info, 0);
        if (start != NULL) { // && !_mi_os_is_huge_reserved(start)) {         
          _mi_abandoned_await_readers(); // ensure no pending reads
          _mi_arena_free(start, MI_REGION_SIZE, arena_memid, (~commit == 0), tld->stats);
        }
      }
    }
  }
}


/* ----------------------------------------------------------------------------
  Other
-----------------------------------------------------------------------------*/

bool _mi_mem_reset(void* p, size_t size, mi_os_tld_t* tld) {
  return _mi_os_reset(p, size, tld->stats);
}

bool _mi_mem_unreset(void* p, size_t size, bool* is_zero, mi_os_tld_t* tld) {
  return _mi_os_unreset(p, size, is_zero, tld->stats);
}

bool _mi_mem_commit(void* p, size_t size, bool* is_zero, mi_os_tld_t* tld) {
  return _mi_os_commit(p, size, is_zero, tld->stats);
}

bool _mi_mem_decommit(void* p, size_t size, mi_os_tld_t* tld) {
  return _mi_os_decommit(p, size, tld->stats);
}

bool _mi_mem_protect(void* p, size_t size) {
  return _mi_os_protect(p, size);
}

bool _mi_mem_unprotect(void* p, size_t size) {
  return _mi_os_unprotect(p, size);
}
/**** ended inlining region.c ****/
/**** start inlining segment.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/
/**** skipping file: mimalloc-atomic.h ****/

#include <string.h>  // memset
#include <stdio.h>

#define MI_PAGE_HUGE_ALIGN  (256*1024)

static uint8_t* mi_segment_raw_page_start(const mi_segment_t* segment, const mi_page_t* page, size_t* page_size);

/* --------------------------------------------------------------------------------
  Segment allocation
  We allocate pages inside bigger "segments" (4mb on 64-bit). This is to avoid
  splitting VMA's on Linux and reduce fragmentation on other OS's.
  Each thread owns its own segments.

  Currently we have:
  - small pages (64kb), 64 in one segment
  - medium pages (512kb), 8 in one segment
  - large pages (4mb), 1 in one segment
  - huge blocks > MI_LARGE_OBJ_SIZE_MAX become large segment with 1 page

  In any case the memory for a segment is virtual and usually committed on demand.
  (i.e. we are careful to not touch the memory until we actually allocate a block there)

  If a  thread ends, it "abandons" pages with used blocks
  and there is an abandoned segment list whose segments can
  be reclaimed by still running threads, much like work-stealing.
-------------------------------------------------------------------------------- */


/* -----------------------------------------------------------
  Queue of segments containing free pages
----------------------------------------------------------- */

#if (MI_DEBUG>=3)
static bool mi_segment_queue_contains(const mi_segment_queue_t* queue, const mi_segment_t* segment) {
  mi_assert_internal(segment != NULL);
  mi_segment_t* list = queue->first;
  while (list != NULL) {
    if (list == segment) break;
    mi_assert_internal(list->next==NULL || list->next->prev == list);
    mi_assert_internal(list->prev==NULL || list->prev->next == list);
    list = list->next;
  }
  return (list == segment);
}
#endif

static bool mi_segment_queue_is_empty(const mi_segment_queue_t* queue) {
  return (queue->first == NULL);
}

static void mi_segment_queue_remove(mi_segment_queue_t* queue, mi_segment_t* segment) {
  mi_assert_expensive(mi_segment_queue_contains(queue, segment));
  if (segment->prev != NULL) segment->prev->next = segment->next;
  if (segment->next != NULL) segment->next->prev = segment->prev;
  if (segment == queue->first) queue->first = segment->next;
  if (segment == queue->last)  queue->last = segment->prev;
  segment->next = NULL;
  segment->prev = NULL;
}

static void mi_segment_enqueue(mi_segment_queue_t* queue, mi_segment_t* segment) {
  mi_assert_expensive(!mi_segment_queue_contains(queue, segment));
  segment->next = NULL;
  segment->prev = queue->last;
  if (queue->last != NULL) {
    mi_assert_internal(queue->last->next == NULL);
    queue->last->next = segment;
    queue->last = segment;
  }
  else {
    queue->last = queue->first = segment;
  }
}

static mi_segment_queue_t* mi_segment_free_queue_of_kind(mi_page_kind_t kind, mi_segments_tld_t* tld) {
  if (kind == MI_PAGE_SMALL) return &tld->small_free;
  else if (kind == MI_PAGE_MEDIUM) return &tld->medium_free;
  else return NULL;
}

static mi_segment_queue_t* mi_segment_free_queue(const mi_segment_t* segment, mi_segments_tld_t* tld) {
  return mi_segment_free_queue_of_kind(segment->page_kind, tld);
}

// remove from free queue if it is in one
static void mi_segment_remove_from_free_queue(mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_segment_queue_t* queue = mi_segment_free_queue(segment, tld); // may be NULL
  bool in_queue = (queue!=NULL && (segment->next != NULL || segment->prev != NULL || queue->first == segment));
  if (in_queue) {
    mi_segment_queue_remove(queue, segment);
  }
}

static void mi_segment_insert_in_free_queue(mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_segment_enqueue(mi_segment_free_queue(segment, tld), segment);
}


/* -----------------------------------------------------------
 Invariant checking
----------------------------------------------------------- */

#if (MI_DEBUG>=2)
static bool mi_segment_is_in_free_queue(const mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_segment_queue_t* queue = mi_segment_free_queue(segment, tld);
  bool in_queue = (queue!=NULL && (segment->next != NULL || segment->prev != NULL || queue->first == segment));
  if (in_queue) {
    mi_assert_expensive(mi_segment_queue_contains(queue, segment));
  }
  return in_queue;
}
#endif

static size_t mi_segment_page_size(const mi_segment_t* segment) {
  if (segment->capacity > 1) {
    mi_assert_internal(segment->page_kind <= MI_PAGE_MEDIUM);
    return ((size_t)1 << segment->page_shift);
  }
  else {
    mi_assert_internal(segment->page_kind >= MI_PAGE_LARGE);
    return segment->segment_size;
  }
}


#if (MI_DEBUG>=2)
static bool mi_pages_reset_contains(const mi_page_t* page, mi_segments_tld_t* tld) {
  mi_page_t* p = tld->pages_reset.first;
  while (p != NULL) {
    if (p == page) return true;
    p = p->next;
  }
  return false;
}
#endif

#if (MI_DEBUG>=3)
static bool mi_segment_is_valid(const mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_assert_internal(segment != NULL);
  mi_assert_internal(_mi_ptr_cookie(segment) == segment->cookie);
  mi_assert_internal(segment->used <= segment->capacity);
  mi_assert_internal(segment->abandoned <= segment->used);
  size_t nfree = 0;
  for (size_t i = 0; i < segment->capacity; i++) {
    const mi_page_t* const page = &segment->pages[i];
    if (!page->segment_in_use) {
      nfree++;
    }
    if (page->segment_in_use || page->is_reset) {
      mi_assert_expensive(!mi_pages_reset_contains(page, tld));
    }
  }
  mi_assert_internal(nfree + segment->used == segment->capacity);
  // mi_assert_internal(segment->thread_id == _mi_thread_id() || (segment->thread_id==0)); // or 0
  mi_assert_internal(segment->page_kind == MI_PAGE_HUGE ||
                     (mi_segment_page_size(segment) * segment->capacity == segment->segment_size));
  return true;
}
#endif

static bool mi_page_not_in_queue(const mi_page_t* page, mi_segments_tld_t* tld) {
  mi_assert_internal(page != NULL);
  if (page->next != NULL || page->prev != NULL) {
    mi_assert_internal(mi_pages_reset_contains(page, tld));
    return false;
  }
  else {
    // both next and prev are NULL, check for singleton list
    return (tld->pages_reset.first != page && tld->pages_reset.last != page);
  }
}


/* -----------------------------------------------------------
  Guard pages
----------------------------------------------------------- */

static void mi_segment_protect_range(void* p, size_t size, bool protect) {
  if (protect) {
    _mi_mem_protect(p, size);
  }
  else {
    _mi_mem_unprotect(p, size);
  }
}

static void mi_segment_protect(mi_segment_t* segment, bool protect, mi_os_tld_t* tld) {
  // add/remove guard pages
  if (MI_SECURE != 0) {
    // in secure mode, we set up a protected page in between the segment info and the page data
    const size_t os_page_size = _mi_os_page_size();
    mi_assert_internal((segment->segment_info_size - os_page_size) >= (sizeof(mi_segment_t) + ((segment->capacity - 1) * sizeof(mi_page_t))));
    mi_assert_internal(((uintptr_t)segment + segment->segment_info_size) % os_page_size == 0);
    mi_segment_protect_range((uint8_t*)segment + segment->segment_info_size - os_page_size, os_page_size, protect);
    if (MI_SECURE <= 1 || segment->capacity == 1) {
      // and protect the last (or only) page too
      mi_assert_internal(MI_SECURE <= 1 || segment->page_kind >= MI_PAGE_LARGE);
      uint8_t* start = (uint8_t*)segment + segment->segment_size - os_page_size;
      if (protect && !segment->mem_is_committed) {
        // ensure secure page is committed
        _mi_mem_commit(start, os_page_size, NULL, tld);
      }
      mi_segment_protect_range(start, os_page_size, protect);
    }
    else {
      // or protect every page
      const size_t page_size = mi_segment_page_size(segment);
      for (size_t i = 0; i < segment->capacity; i++) {
        if (segment->pages[i].is_committed) {
          mi_segment_protect_range((uint8_t*)segment + (i+1)*page_size - os_page_size, os_page_size, protect);
        }
      }
    }
  }
}

/* -----------------------------------------------------------
  Page reset
----------------------------------------------------------- */

static void mi_page_reset(mi_segment_t* segment, mi_page_t* page, size_t size, mi_segments_tld_t* tld) {
  mi_assert_internal(page->is_committed);
  if (!mi_option_is_enabled(mi_option_page_reset)) return;
  if (segment->mem_is_fixed || page->segment_in_use || !page->is_committed || page->is_reset) return;
  size_t psize;
  void* start = mi_segment_raw_page_start(segment, page, &psize);
  page->is_reset = true;
  mi_assert_internal(size <= psize);
  size_t reset_size = ((size == 0 || size > psize) ? psize : size);
  if (reset_size > 0) _mi_mem_reset(start, reset_size, tld->os);
}

static bool mi_page_unreset(mi_segment_t* segment, mi_page_t* page, size_t size, mi_segments_tld_t* tld)
{
  mi_assert_internal(page->is_reset);
  mi_assert_internal(page->is_committed);
  mi_assert_internal(!segment->mem_is_fixed);
  if (segment->mem_is_fixed || !page->is_committed || !page->is_reset) return true;
  page->is_reset = false;
  size_t psize;
  uint8_t* start = mi_segment_raw_page_start(segment, page, &psize);
  size_t unreset_size = (size == 0 || size > psize ? psize : size);
  bool is_zero = false;
  bool ok = true;
  if (unreset_size > 0) {
    ok = _mi_mem_unreset(start, unreset_size, &is_zero, tld->os);
  }
  if (is_zero) page->is_zero_init = true;
  return ok;
}


/* -----------------------------------------------------------
  The free page queue
----------------------------------------------------------- */

// we re-use the `used` field for the expiration counter. Since this is a
// a 32-bit field while the clock is always 64-bit we need to guard
// against overflow, we use substraction to check for expiry which work
// as long as the reset delay is under (2^30 - 1) milliseconds (~12 days)
static void mi_page_reset_set_expire(mi_page_t* page) {
  uint32_t expire = (uint32_t)_mi_clock_now() + mi_option_get(mi_option_reset_delay);
  page->used = expire;
}

static bool mi_page_reset_is_expired(mi_page_t* page, mi_msecs_t now) {
  int32_t expire = (int32_t)(page->used);
  return (((int32_t)now - expire) >= 0);
}

static void mi_pages_reset_add(mi_segment_t* segment, mi_page_t* page, mi_segments_tld_t* tld) {
  mi_assert_internal(!page->segment_in_use || !page->is_committed);
  mi_assert_internal(mi_page_not_in_queue(page,tld));
  mi_assert_expensive(!mi_pages_reset_contains(page, tld));
  mi_assert_internal(_mi_page_segment(page)==segment);
  if (!mi_option_is_enabled(mi_option_page_reset)) return;
  if (segment->mem_is_fixed || page->segment_in_use || !page->is_committed || page->is_reset) return;

  if (mi_option_get(mi_option_reset_delay) == 0) {
    // reset immediately?
    mi_page_reset(segment, page, 0, tld);
  }
  else {
    // otherwise push on the delayed page reset queue
    mi_page_queue_t* pq = &tld->pages_reset;
    // push on top
    mi_page_reset_set_expire(page);
    page->next = pq->first;
    page->prev = NULL;
    if (pq->first == NULL) {
      mi_assert_internal(pq->last == NULL);
      pq->first = pq->last = page;
    }
    else {
      pq->first->prev = page;
      pq->first = page;
    }
  }
}

static void mi_pages_reset_remove(mi_page_t* page, mi_segments_tld_t* tld) {
  if (mi_page_not_in_queue(page,tld)) return;

  mi_page_queue_t* pq = &tld->pages_reset;
  mi_assert_internal(pq!=NULL);
  mi_assert_internal(!page->segment_in_use);
  mi_assert_internal(mi_pages_reset_contains(page, tld));
  if (page->prev != NULL) page->prev->next = page->next;
  if (page->next != NULL) page->next->prev = page->prev;
  if (page == pq->last)  pq->last = page->prev;
  if (page == pq->first) pq->first = page->next;
  page->next = page->prev = NULL;
  page->used = 0;
}

static void mi_pages_reset_remove_all_in_segment(mi_segment_t* segment, bool force_reset, mi_segments_tld_t* tld) {
  if (segment->mem_is_fixed) return; // never reset in huge OS pages
  for (size_t i = 0; i < segment->capacity; i++) {
    mi_page_t* page = &segment->pages[i];
    if (!page->segment_in_use && page->is_committed && !page->is_reset) {
      mi_pages_reset_remove(page, tld);
      if (force_reset) {
        mi_page_reset(segment, page, 0, tld);
      }
    }
    else {
      mi_assert_internal(mi_page_not_in_queue(page,tld));
    }
  }
}

static void mi_reset_delayed(mi_segments_tld_t* tld) {
  if (!mi_option_is_enabled(mi_option_page_reset)) return;
  mi_msecs_t now = _mi_clock_now();
  mi_page_queue_t* pq = &tld->pages_reset;
  // from oldest up to the first that has not expired yet
  mi_page_t* page = pq->last;
  while (page != NULL && mi_page_reset_is_expired(page,now)) {
    mi_page_t* const prev = page->prev; // save previous field
    mi_page_reset(_mi_page_segment(page), page, 0, tld);
    page->used = 0;
    page->prev = page->next = NULL;
    page = prev;
  }
  // discard the reset pages from the queue
  pq->last = page;
  if (page != NULL){
    page->next = NULL;
  }
  else {
    pq->first = NULL;
  }
}


/* -----------------------------------------------------------
 Segment size calculations
----------------------------------------------------------- */

// Raw start of the page available memory; can be used on uninitialized pages (only `segment_idx` must be set)
// The raw start is not taking aligned block allocation into consideration.
static uint8_t* mi_segment_raw_page_start(const mi_segment_t* segment, const mi_page_t* page, size_t* page_size) {
  size_t   psize = (segment->page_kind == MI_PAGE_HUGE ? segment->segment_size : (size_t)1 << segment->page_shift);
  uint8_t* p = (uint8_t*)segment + page->segment_idx * psize;

  if (page->segment_idx == 0) {
    // the first page starts after the segment info (and possible guard page)
    p += segment->segment_info_size;
    psize -= segment->segment_info_size;
  }

  if (MI_SECURE > 1 || (MI_SECURE == 1 && page->segment_idx == segment->capacity - 1)) {
    // secure == 1: the last page has an os guard page at the end
    // secure >  1: every page has an os guard page
    psize -= _mi_os_page_size();
  }

  if (page_size != NULL) *page_size = psize;
  mi_assert_internal(page->xblock_size == 0 || _mi_ptr_page(p) == page);
  mi_assert_internal(_mi_ptr_segment(p) == segment);
  return p;
}

// Start of the page available memory; can be used on uninitialized pages (only `segment_idx` must be set)
uint8_t* _mi_segment_page_start(const mi_segment_t* segment, const mi_page_t* page, size_t block_size, size_t* page_size, size_t* pre_size)
{
  size_t   psize;
  uint8_t* p = mi_segment_raw_page_start(segment, page, &psize);
  if (pre_size != NULL) *pre_size = 0;
  if (page->segment_idx == 0 && block_size > 0 && segment->page_kind <= MI_PAGE_MEDIUM) {
    // for small and medium objects, ensure the page start is aligned with the block size (PR#66 by kickunderscore)
    size_t adjust = block_size - ((uintptr_t)p % block_size);
    if (adjust < block_size) {
      p += adjust;
      psize -= adjust;
      if (pre_size != NULL) *pre_size = adjust;
    }
    mi_assert_internal((uintptr_t)p % block_size == 0);
  }

  if (page_size != NULL) *page_size = psize;
  mi_assert_internal(page->xblock_size==0 || _mi_ptr_page(p) == page);
  mi_assert_internal(_mi_ptr_segment(p) == segment);
  return p;
}

static size_t mi_segment_size(size_t capacity, size_t required, size_t* pre_size, size_t* info_size)
{
  const size_t minsize   = sizeof(mi_segment_t) + ((capacity - 1) * sizeof(mi_page_t)) + 16 /* padding */;
  size_t guardsize = 0;
  size_t isize     = 0;

  if (MI_SECURE == 0) {
    // normally no guard pages
    isize = _mi_align_up(minsize, 16 * MI_MAX_ALIGN_SIZE);
  }
  else {
    // in secure mode, we set up a protected page in between the segment info
    // and the page data (and one at the end of the segment)
    const size_t page_size = _mi_os_page_size();
    isize = _mi_align_up(minsize, page_size);
    guardsize = page_size;
    required = _mi_align_up(required, page_size);
  }

  if (info_size != NULL) *info_size = isize;
  if (pre_size != NULL)  *pre_size  = isize + guardsize;
  return (required==0 ? MI_SEGMENT_SIZE : _mi_align_up( required + isize + 2*guardsize, MI_PAGE_HUGE_ALIGN) );
}


/* ----------------------------------------------------------------------------
Segment caches
We keep a small segment cache per thread to increase local
reuse and avoid setting/clearing guard pages in secure mode.
------------------------------------------------------------------------------- */

static void mi_segments_track_size(long segment_size, mi_segments_tld_t* tld) {
  if (segment_size>=0) _mi_stat_increase(&tld->stats->segments,1);
                  else _mi_stat_decrease(&tld->stats->segments,1);
  tld->count += (segment_size >= 0 ? 1 : -1);
  if (tld->count > tld->peak_count) tld->peak_count = tld->count;
  tld->current_size += segment_size;
  if (tld->current_size > tld->peak_size) tld->peak_size = tld->current_size;
}

static void mi_segment_os_free(mi_segment_t* segment, size_t segment_size, mi_segments_tld_t* tld) {
  segment->thread_id = 0;
  mi_segments_track_size(-((long)segment_size),tld);
  if (MI_SECURE != 0) {
    mi_assert_internal(!segment->mem_is_fixed);
    mi_segment_protect(segment, false, tld->os); // ensure no more guard pages are set
  }

  bool any_reset = false;
  bool fully_committed = true;
  for (size_t i = 0; i < segment->capacity; i++) {
    mi_page_t* page = &segment->pages[i];
    if (!page->is_committed) { fully_committed = false; }
    if (page->is_reset)      { any_reset = true; }
  }
  if (any_reset && mi_option_is_enabled(mi_option_reset_decommits)) {
    fully_committed = false;
  }
  
  _mi_mem_free(segment, segment_size, segment->memid, fully_committed, any_reset, tld->os);
}


// The thread local segment cache is limited to be at most 1/8 of the peak size of segments in use,
#define MI_SEGMENT_CACHE_FRACTION (8)

// note: returned segment may be partially reset
static mi_segment_t* mi_segment_cache_pop(size_t segment_size, mi_segments_tld_t* tld) {
  if (segment_size != 0 && segment_size != MI_SEGMENT_SIZE) return NULL;
  mi_segment_t* segment = tld->cache;
  if (segment == NULL) return NULL;
  tld->cache_count--;
  tld->cache = segment->next;
  segment->next = NULL;
  mi_assert_internal(segment->segment_size == MI_SEGMENT_SIZE);
  _mi_stat_decrease(&tld->stats->segments_cache, 1);
  return segment;
}

static bool mi_segment_cache_full(mi_segments_tld_t* tld)
{
  // if (tld->count == 1 && tld->cache_count==0) return false; // always cache at least the final segment of a thread
  size_t max_cache = mi_option_get(mi_option_segment_cache);
  if (tld->cache_count < max_cache
       && tld->cache_count < (1 + (tld->peak_count / MI_SEGMENT_CACHE_FRACTION)) // at least allow a 1 element cache
     ) {
    return false;
  }
  // take the opportunity to reduce the segment cache if it is too large (now)
  // TODO: this never happens as we check against peak usage, should we use current usage instead?
  while (tld->cache_count > max_cache) { //(1 + (tld->peak_count / MI_SEGMENT_CACHE_FRACTION))) {
    mi_segment_t* segment = mi_segment_cache_pop(0,tld);
    mi_assert_internal(segment != NULL);
    if (segment != NULL) mi_segment_os_free(segment, segment->segment_size, tld);
  }
  return true;
}

static bool mi_segment_cache_push(mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_assert_internal(!mi_segment_is_in_free_queue(segment, tld));
  mi_assert_internal(segment->next == NULL);
  if (segment->segment_size != MI_SEGMENT_SIZE || mi_segment_cache_full(tld)) {
    return false;
  }
  mi_assert_internal(segment->segment_size == MI_SEGMENT_SIZE);
  segment->next = tld->cache;
  tld->cache = segment;
  tld->cache_count++;
  _mi_stat_increase(&tld->stats->segments_cache,1);
  return true;
}

// called by threads that are terminating to free cached segments
void _mi_segment_thread_collect(mi_segments_tld_t* tld) {
  mi_segment_t* segment;
  while ((segment = mi_segment_cache_pop(0,tld)) != NULL) {
    mi_segment_os_free(segment, segment->segment_size, tld);
  }
  mi_assert_internal(tld->cache_count == 0);
  mi_assert_internal(tld->cache == NULL);
#if MI_DEBUG>=2
  if (!_mi_is_main_thread()) {
    mi_assert_internal(tld->pages_reset.first == NULL);
    mi_assert_internal(tld->pages_reset.last == NULL);
  }
#endif
}


/* -----------------------------------------------------------
   Segment allocation
----------------------------------------------------------- */

// Allocate a segment from the OS aligned to `MI_SEGMENT_SIZE` .
static mi_segment_t* mi_segment_init(mi_segment_t* segment, size_t required, mi_page_kind_t page_kind, size_t page_shift, mi_segments_tld_t* tld, mi_os_tld_t* os_tld)
{
  // the segment parameter is non-null if it came from our cache
  mi_assert_internal(segment==NULL || (required==0 && page_kind <= MI_PAGE_LARGE));

  // calculate needed sizes first
  size_t capacity;
  if (page_kind == MI_PAGE_HUGE) {
    mi_assert_internal(page_shift == MI_SEGMENT_SHIFT && required > 0);
    capacity = 1;
  }
  else {
    mi_assert_internal(required == 0);
    size_t page_size = (size_t)1 << page_shift;
    capacity = MI_SEGMENT_SIZE / page_size;
    mi_assert_internal(MI_SEGMENT_SIZE % page_size == 0);
    mi_assert_internal(capacity >= 1 && capacity <= MI_SMALL_PAGES_PER_SEGMENT);
  }
  size_t info_size;
  size_t pre_size;
  size_t segment_size = mi_segment_size(capacity, required, &pre_size, &info_size);
  mi_assert_internal(segment_size >= required);

  // Initialize parameters
  const bool eager_delayed = (page_kind <= MI_PAGE_MEDIUM && tld->count < (size_t)mi_option_get(mi_option_eager_commit_delay));
  const bool eager  = !eager_delayed && mi_option_is_enabled(mi_option_eager_commit);
  bool commit = eager; // || (page_kind >= MI_PAGE_LARGE);
  bool pages_still_good = false;
  bool is_zero = false;

  // Try to get it from our thread local cache first
  if (segment != NULL) {
    // came from cache
    mi_assert_internal(segment->segment_size == segment_size);
    if (page_kind <= MI_PAGE_MEDIUM && segment->page_kind == page_kind && segment->segment_size == segment_size) {
      pages_still_good = true;
    }
    else
    {
      if (MI_SECURE!=0) {
        mi_assert_internal(!segment->mem_is_fixed);
        mi_segment_protect(segment, false, tld->os); // reset protection if the page kind differs
      }
      // different page kinds; unreset any reset pages, and unprotect
      // TODO: optimize cache pop to return fitting pages if possible?
      for (size_t i = 0; i < segment->capacity; i++) {
        mi_page_t* page = &segment->pages[i];
        if (page->is_reset) {
          if (!commit && mi_option_is_enabled(mi_option_reset_decommits)) {
            page->is_reset = false;
          }
          else {
            mi_page_unreset(segment, page, 0, tld);  // todo: only unreset the part that was reset? (instead of the full page)
          }
        }
      }
      // ensure the initial info is committed
      if (segment->capacity < capacity) {
        bool commit_zero = false;
        _mi_mem_commit(segment, pre_size, &commit_zero, tld->os);
        if (commit_zero) is_zero = true;
      }
    }
  }
  else {
    // Allocate the segment from the OS
    size_t memid;
    bool   mem_large = (!eager_delayed && (MI_SECURE==0)); // only allow large OS pages once we are no longer lazy
    segment = (mi_segment_t*)_mi_mem_alloc_aligned(segment_size, MI_SEGMENT_SIZE, &commit, &mem_large, &is_zero, &memid, os_tld);
    if (segment == NULL) return NULL;  // failed to allocate
    if (!commit) {
      // ensure the initial info is committed
      bool commit_zero = false;
      bool ok = _mi_mem_commit(segment, pre_size, &commit_zero, tld->os);
      if (commit_zero) is_zero = true;
      if (!ok) {
        // commit failed; we cannot touch the memory: free the segment directly and return `NULL`
        _mi_mem_free(segment, MI_SEGMENT_SIZE, memid, false, false, os_tld);
        return NULL;  
      }
    }
    segment->memid = memid;
    segment->mem_is_fixed = mem_large;
    segment->mem_is_committed = commit;
    mi_segments_track_size((long)segment_size, tld);
  }
  mi_assert_internal(segment != NULL && (uintptr_t)segment % MI_SEGMENT_SIZE == 0);
  mi_assert_internal(segment->mem_is_fixed ? segment->mem_is_committed : true);  
  if (!pages_still_good) {
    // zero the segment info (but not the `mem` fields)
    ptrdiff_t ofs = offsetof(mi_segment_t, next);
    memset((uint8_t*)segment + ofs, 0, info_size - ofs);

    // initialize pages info
    for (uint8_t i = 0; i < capacity; i++) {
      segment->pages[i].segment_idx = i;
      segment->pages[i].is_reset = false;
      segment->pages[i].is_committed = commit;
      segment->pages[i].is_zero_init = is_zero;
    }
  }
  else {
    // zero the segment info but not the pages info (and mem fields)
    ptrdiff_t ofs = offsetof(mi_segment_t, next);
    memset((uint8_t*)segment + ofs, 0, offsetof(mi_segment_t,pages) - ofs);
  }

  // initialize
  segment->page_kind  = page_kind;
  segment->capacity   = capacity;
  segment->page_shift = page_shift;
  segment->segment_size = segment_size;
  segment->segment_info_size = pre_size;
  segment->thread_id  = _mi_thread_id();
  segment->cookie = _mi_ptr_cookie(segment);
  // _mi_stat_increase(&tld->stats->page_committed, segment->segment_info_size);

  // set protection
  mi_segment_protect(segment, true, tld->os);

  // insert in free lists for small and medium pages
  if (page_kind <= MI_PAGE_MEDIUM) {
    mi_segment_insert_in_free_queue(segment, tld);
  }

  //fprintf(stderr,"mimalloc: alloc segment at %p\n", (void*)segment);
  return segment;
}

static mi_segment_t* mi_segment_alloc(size_t required, mi_page_kind_t page_kind, size_t page_shift, mi_segments_tld_t* tld, mi_os_tld_t* os_tld) {
  return mi_segment_init(NULL, required, page_kind, page_shift, tld, os_tld);
}

static void mi_segment_free(mi_segment_t* segment, bool force, mi_segments_tld_t* tld) {
  UNUSED(force);
  mi_assert(segment != NULL);
  // note: don't reset pages even on abandon as the whole segment is freed? (and ready for reuse)
  bool force_reset = (force && mi_option_is_enabled(mi_option_abandoned_page_reset));
  mi_pages_reset_remove_all_in_segment(segment, force_reset, tld);
  mi_segment_remove_from_free_queue(segment,tld);

  mi_assert_expensive(!mi_segment_queue_contains(&tld->small_free, segment));
  mi_assert_expensive(!mi_segment_queue_contains(&tld->medium_free, segment));
  mi_assert(segment->next == NULL);
  mi_assert(segment->prev == NULL);
  _mi_stat_decrease(&tld->stats->page_committed, segment->segment_info_size);

  if (!force && mi_segment_cache_push(segment, tld)) {
    // it is put in our cache
  }
  else {
    // otherwise return it to the OS
    mi_segment_os_free(segment, segment->segment_size, tld);
  }
}

/* -----------------------------------------------------------
  Free page management inside a segment
----------------------------------------------------------- */


static bool mi_segment_has_free(const mi_segment_t* segment) {
  return (segment->used < segment->capacity);
}

static bool mi_segment_page_claim(mi_segment_t* segment, mi_page_t* page, mi_segments_tld_t* tld) {
  mi_assert_internal(_mi_page_segment(page) == segment);
  mi_assert_internal(!page->segment_in_use);
  mi_pages_reset_remove(page, tld);
  // check commit
  if (!page->is_committed) {
    mi_assert_internal(!segment->mem_is_fixed);
    mi_assert_internal(!page->is_reset);    
    size_t psize;
    uint8_t* start = mi_segment_raw_page_start(segment, page, &psize);
    bool is_zero = false;
    const size_t gsize = (MI_SECURE >= 2 ? _mi_os_page_size() : 0);
    bool ok = _mi_mem_commit(start, psize + gsize, &is_zero, tld->os);
    if (!ok) return false; // failed to commit!
    if (gsize > 0) { mi_segment_protect_range(start + psize, gsize, true); }
    if (is_zero) { page->is_zero_init = true; }
    page->is_committed = true;
  }
  // set in-use before doing unreset to prevent delayed reset
  page->segment_in_use = true;
  segment->used++;
  // check reset
  if (page->is_reset) {
    mi_assert_internal(!segment->mem_is_fixed);
    bool ok = mi_page_unreset(segment, page, 0, tld); 
    if (!ok) {
      page->segment_in_use = false;
      segment->used--;
      return false;
    }
  }
  mi_assert_internal(page->segment_in_use);
  mi_assert_internal(segment->used <= segment->capacity);
  if (segment->used == segment->capacity && segment->page_kind <= MI_PAGE_MEDIUM) {
    // if no more free pages, remove from the queue
    mi_assert_internal(!mi_segment_has_free(segment));
    mi_segment_remove_from_free_queue(segment, tld);
  }
  return true;
}


/* -----------------------------------------------------------
   Free
----------------------------------------------------------- */

static void mi_segment_abandon(mi_segment_t* segment, mi_segments_tld_t* tld);

// clear page data; can be called on abandoned segments
static void mi_segment_page_clear(mi_segment_t* segment, mi_page_t* page, bool allow_reset, mi_segments_tld_t* tld)
{
  mi_assert_internal(page->segment_in_use);
  mi_assert_internal(mi_page_all_free(page));
  mi_assert_internal(page->is_committed);
  mi_assert_internal(mi_page_not_in_queue(page, tld));

  size_t inuse = page->capacity * mi_page_block_size(page);
  _mi_stat_decrease(&tld->stats->page_committed, inuse);
  _mi_stat_decrease(&tld->stats->pages, 1);

  // calculate the used size from the raw (non-aligned) start of the page
  //size_t pre_size;
  //_mi_segment_page_start(segment, page, page->block_size, NULL, &pre_size);
  //size_t used_size = pre_size + (page->capacity * page->block_size);

  page->is_zero_init = false;
  page->segment_in_use = false;

  // reset the page memory to reduce memory pressure?
  // note: must come after setting `segment_in_use` to false but before block_size becomes 0
  //mi_page_reset(segment, page, 0 /*used_size*/, tld);

  // zero the page data, but not the segment fields and capacity, and block_size (for page size calculations)
  uint32_t block_size = page->xblock_size;
  uint16_t capacity = page->capacity;
  uint16_t reserved = page->reserved;
  ptrdiff_t ofs = offsetof(mi_page_t,capacity);
  memset((uint8_t*)page + ofs, 0, sizeof(*page) - ofs);
  page->capacity = capacity;
  page->reserved = reserved;
  page->xblock_size = block_size;
  segment->used--;

  // add to the free page list for reuse/reset
  if (allow_reset) {
    mi_pages_reset_add(segment, page, tld);
  }

  page->capacity = 0;  // after reset there can be zero'd now
  page->reserved = 0;
}

void _mi_segment_page_free(mi_page_t* page, bool force, mi_segments_tld_t* tld)
{
  mi_assert(page != NULL);
  mi_segment_t* segment = _mi_page_segment(page);
  mi_assert_expensive(mi_segment_is_valid(segment,tld));
  mi_reset_delayed(tld);

  // mark it as free now
  mi_segment_page_clear(segment, page, true, tld);

  if (segment->used == 0) {
    // no more used pages; remove from the free list and free the segment
    mi_segment_free(segment, force, tld);
  }
  else {
    if (segment->used == segment->abandoned) {
      // only abandoned pages; remove from free list and abandon
      mi_segment_abandon(segment,tld);
    }
    else if (segment->used + 1 == segment->capacity) {
      mi_assert_internal(segment->page_kind <= MI_PAGE_MEDIUM); // for now we only support small and medium pages
      // move back to segments  free list
      mi_segment_insert_in_free_queue(segment,tld);
    }
  }
}


/* -----------------------------------------------------------
Abandonment

When threads terminate, they can leave segments with
live blocks (reached through other threads). Such segments
are "abandoned" and will be reclaimed by other threads to
reuse their pages and/or free them eventually

We maintain a global list of abandoned segments that are
reclaimed on demand. Since this is shared among threads
the implementation needs to avoid the A-B-A problem on
popping abandoned segments: <https://en.wikipedia.org/wiki/ABA_problem>
We use tagged pointers to avoid accidentially identifying
reused segments, much like stamped references in Java.
Secondly, we maintain a reader counter to avoid resetting
or decommitting segments that have a pending read operation.

Note: the current implementation is one possible design;
another way might be to keep track of abandoned segments
in the regions. This would have the advantage of keeping
all concurrent code in one place and not needing to deal
with ABA issues. The drawback is that it is unclear how to
scan abandoned segments efficiently in that case as they
would be spread among all other segments in the regions.
----------------------------------------------------------- */

// Use the bottom 20-bits (on 64-bit) of the aligned segment pointers
// to put in a tag that increments on update to avoid the A-B-A problem.
#define MI_TAGGED_MASK   MI_SEGMENT_MASK
typedef uintptr_t        mi_tagged_segment_t;

static mi_segment_t* mi_tagged_segment_ptr(mi_tagged_segment_t ts) {
  return (mi_segment_t*)(ts & ~MI_TAGGED_MASK);
}

static mi_tagged_segment_t mi_tagged_segment(mi_segment_t* segment, mi_tagged_segment_t ts) {
  mi_assert_internal(((uintptr_t)segment & MI_TAGGED_MASK) == 0);
  uintptr_t tag = ((ts & MI_TAGGED_MASK) + 1) & MI_TAGGED_MASK;
  return ((uintptr_t)segment | tag);
}

// This is a list of visited abandoned pages that were full at the time.
// this list migrates to `abandoned` when that becomes NULL. The use of
// this list reduces contention and the rate at which segments are visited.
static mi_decl_cache_align volatile _Atomic(mi_segment_t*)       abandoned_visited; // = NULL

// The abandoned page list (tagged as it supports pop)
static mi_decl_cache_align volatile _Atomic(mi_tagged_segment_t) abandoned;         // = NULL

// We also maintain a count of current readers of the abandoned list
// in order to prevent resetting/decommitting segment memory if it might
// still be read.
static mi_decl_cache_align volatile _Atomic(uintptr_t)           abandoned_readers; // = 0

// Push on the visited list
static void mi_abandoned_visited_push(mi_segment_t* segment) {
  mi_assert_internal(segment->thread_id == 0);
  mi_assert_internal(segment->abandoned_next == NULL);
  mi_assert_internal(segment->next == NULL && segment->prev == NULL);
  mi_assert_internal(segment->used > 0);
  mi_segment_t* anext;
  do {
    anext = mi_atomic_read_ptr_relaxed(mi_segment_t, &abandoned_visited);
    segment->abandoned_next = anext;
  } while (!mi_atomic_cas_ptr_weak(mi_segment_t, &abandoned_visited, segment, anext));
}

// Move the visited list to the abandoned list.
static bool mi_abandoned_visited_revisit(void)
{
  // quick check if the visited list is empty
  if (mi_atomic_read_ptr_relaxed(mi_segment_t,&abandoned_visited)==NULL) return false;

  // grab the whole visited list
  mi_segment_t* first = mi_atomic_exchange_ptr(mi_segment_t, &abandoned_visited, NULL);
  if (first == NULL) return false;

  // first try to swap directly if the abandoned list happens to be NULL
  const mi_tagged_segment_t ts = mi_atomic_read_relaxed(&abandoned);
  mi_tagged_segment_t afirst;
  if (mi_tagged_segment_ptr(ts)==NULL) {
    afirst = mi_tagged_segment(first, ts);
    if (mi_atomic_cas_strong(&abandoned, afirst, ts)) return true;
  }

  // find the last element of the visited list: O(n)
  mi_segment_t* last = first;
  while (last->abandoned_next != NULL) {
    last = last->abandoned_next;
  }

  // and atomically prepend to the abandoned list
  // (no need to increase the readers as we don't access the abandoned segments)
  mi_tagged_segment_t anext;
  do {
    anext = mi_atomic_read_relaxed(&abandoned);
    last->abandoned_next = mi_tagged_segment_ptr(anext);
    afirst = mi_tagged_segment(first, anext);
  } while (!mi_atomic_cas_weak(&abandoned, afirst, anext));
  return true;
}

// Push on the abandoned list.
static void mi_abandoned_push(mi_segment_t* segment) {
  mi_assert_internal(segment->thread_id == 0);
  mi_assert_internal(segment->abandoned_next == NULL);
  mi_assert_internal(segment->next == NULL && segment->prev == NULL);
  mi_assert_internal(segment->used > 0);
  mi_tagged_segment_t ts;
  mi_tagged_segment_t next;
  do {
    ts = mi_atomic_read_relaxed(&abandoned);
    segment->abandoned_next = mi_tagged_segment_ptr(ts);
    next = mi_tagged_segment(segment, ts);
  } while (!mi_atomic_cas_weak(&abandoned, next, ts));
}

// Wait until there are no more pending reads on segments that used to be in the abandoned list
void _mi_abandoned_await_readers(void) {
  uintptr_t n;
  do {
    n = mi_atomic_read(&abandoned_readers);
    if (n != 0) mi_atomic_yield();
  } while (n != 0);
}

// Pop from the abandoned list
static mi_segment_t* mi_abandoned_pop(void) {
  mi_segment_t* segment;
  // Check efficiently if it is empty (or if the visited list needs to be moved)
  mi_tagged_segment_t ts = mi_atomic_read_relaxed(&abandoned);
  segment = mi_tagged_segment_ptr(ts);
  if (mi_likely(segment == NULL)) {
    if (mi_likely(!mi_abandoned_visited_revisit())) { // try to swap in the visited list on NULL
      return NULL;
    }
  }

  // Do a pop. We use a reader count to prevent
  // a segment to be decommitted while a read is still pending,
  // and a tagged pointer to prevent A-B-A link corruption.
  // (this is called from `memory.c:_mi_mem_free` for example)
  mi_atomic_increment(&abandoned_readers);  // ensure no segment gets decommitted
  mi_tagged_segment_t next = 0;
  do {
    ts = mi_atomic_read(&abandoned);
    segment = mi_tagged_segment_ptr(ts);
    if (segment != NULL) {
      next = mi_tagged_segment(segment->abandoned_next, ts); // note: reads the segment's `abandoned_next` field so should not be decommitted
    }
  } while (segment != NULL && !mi_atomic_cas_weak(&abandoned, next, ts));
  mi_atomic_decrement(&abandoned_readers);  // release reader lock
  if (segment != NULL) {
    segment->abandoned_next = NULL;
  }
  return segment;
}

/* -----------------------------------------------------------
   Abandon segment/page
----------------------------------------------------------- */

static void mi_segment_abandon(mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_assert_internal(segment->used == segment->abandoned);
  mi_assert_internal(segment->used > 0);
  mi_assert_internal(segment->abandoned_next == NULL);
  mi_assert_expensive(mi_segment_is_valid(segment, tld));

  // remove the segment from the free page queue if needed
  mi_reset_delayed(tld);
  mi_pages_reset_remove_all_in_segment(segment, mi_option_is_enabled(mi_option_abandoned_page_reset), tld);
  mi_segment_remove_from_free_queue(segment, tld);
  mi_assert_internal(segment->next == NULL && segment->prev == NULL);

  // all pages in the segment are abandoned; add it to the abandoned list
  _mi_stat_increase(&tld->stats->segments_abandoned, 1);
  mi_segments_track_size(-((long)segment->segment_size), tld);
  segment->thread_id = 0;
  segment->abandoned_next = NULL;
  segment->abandoned_visits = 0;
  mi_abandoned_push(segment);
}

void _mi_segment_page_abandon(mi_page_t* page, mi_segments_tld_t* tld) {
  mi_assert(page != NULL);
  mi_assert_internal(mi_page_thread_free_flag(page)==MI_NEVER_DELAYED_FREE);
  mi_assert_internal(mi_page_heap(page) == NULL);
  mi_segment_t* segment = _mi_page_segment(page);
  mi_assert_expensive(!mi_pages_reset_contains(page, tld));
  mi_assert_expensive(mi_segment_is_valid(segment, tld));
  segment->abandoned++;
  _mi_stat_increase(&tld->stats->pages_abandoned, 1);
  mi_assert_internal(segment->abandoned <= segment->used);
  if (segment->used == segment->abandoned) {
    // all pages are abandoned, abandon the entire segment
    mi_segment_abandon(segment, tld);
  }
}

/* -----------------------------------------------------------
  Reclaim abandoned pages
----------------------------------------------------------- */

// Possibly clear pages and check if free space is available
static bool mi_segment_check_free(mi_segment_t* segment, size_t block_size, bool* all_pages_free)
{
  mi_assert_internal(block_size < MI_HUGE_BLOCK_SIZE);
  bool has_page = false;
  size_t pages_used = 0;
  size_t pages_used_empty = 0;
  for (size_t i = 0; i < segment->capacity; i++) {
    mi_page_t* page = &segment->pages[i];
    if (page->segment_in_use) {
      pages_used++;
      // ensure used count is up to date and collect potential concurrent frees
      _mi_page_free_collect(page, false);
      if (mi_page_all_free(page)) {
        // if everything free already, page can be reused for some block size
        // note: don't clear the page yet as we can only OS reset it once it is reclaimed
        pages_used_empty++;
        has_page = true;
      }
      else if (page->xblock_size == block_size && mi_page_has_any_available(page)) {
        // a page has available free blocks of the right size
        has_page = true;
      }
    }
    else {
      // whole empty page
      has_page = true;
    }
  }
  mi_assert_internal(pages_used == segment->used && pages_used >= pages_used_empty);
  if (all_pages_free != NULL) {
    *all_pages_free = ((pages_used - pages_used_empty) == 0);
  }
  return has_page;
}


// Reclaim a segment; returns NULL if the segment was freed
// set `right_page_reclaimed` to `true` if it reclaimed a page of the right `block_size` that was not full.
static mi_segment_t* mi_segment_reclaim(mi_segment_t* segment, mi_heap_t* heap, size_t requested_block_size, bool* right_page_reclaimed, mi_segments_tld_t* tld) {
  mi_assert_internal(segment->abandoned_next == NULL);
  if (right_page_reclaimed != NULL) { *right_page_reclaimed = false; }

  segment->thread_id = _mi_thread_id();
  segment->abandoned_visits = 0;
  mi_segments_track_size((long)segment->segment_size, tld);
  mi_assert_internal(segment->next == NULL && segment->prev == NULL);
  mi_assert_expensive(mi_segment_is_valid(segment, tld));
  _mi_stat_decrease(&tld->stats->segments_abandoned, 1);

  for (size_t i = 0; i < segment->capacity; i++) {
    mi_page_t* page = &segment->pages[i];
    if (page->segment_in_use) {
      mi_assert_internal(!page->is_reset);
      mi_assert_internal(page->is_committed);
      mi_assert_internal(mi_page_not_in_queue(page, tld));
      mi_assert_internal(mi_page_thread_free_flag(page)==MI_NEVER_DELAYED_FREE);
      mi_assert_internal(mi_page_heap(page) == NULL);
      segment->abandoned--;
      mi_assert(page->next == NULL);
      _mi_stat_decrease(&tld->stats->pages_abandoned, 1);
      // set the heap again and allow heap thread delayed free again.
      mi_page_set_heap(page, heap);
      _mi_page_use_delayed_free(page, MI_USE_DELAYED_FREE, true); // override never (after heap is set)
      // TODO: should we not collect again given that we just collected in `check_free`?
      _mi_page_free_collect(page, false); // ensure used count is up to date
      if (mi_page_all_free(page)) {
        // if everything free already, clear the page directly
        mi_segment_page_clear(segment, page, true, tld);  // reset is ok now
      }
      else {
        // otherwise reclaim it into the heap
        _mi_page_reclaim(heap, page);
        if (requested_block_size == page->xblock_size && mi_page_has_any_available(page)) {
          if (right_page_reclaimed != NULL) { *right_page_reclaimed = true; }
        }
      }
    }
    else if (page->is_committed && !page->is_reset) {  // not in-use, and not reset yet
      // note: do not reset as this includes pages that were not touched before
      // mi_pages_reset_add(segment, page, tld);
    }
  }
  mi_assert_internal(segment->abandoned == 0);
  if (segment->used == 0) {
    mi_assert_internal(right_page_reclaimed == NULL || !(*right_page_reclaimed));
    mi_segment_free(segment, false, tld);
    return NULL;
  }
  else {
    if (segment->page_kind <= MI_PAGE_MEDIUM && mi_segment_has_free(segment)) {
      mi_segment_insert_in_free_queue(segment, tld);
    }
    return segment;
  }
}


void _mi_abandoned_reclaim_all(mi_heap_t* heap, mi_segments_tld_t* tld) {
  mi_segment_t* segment;
  while ((segment = mi_abandoned_pop()) != NULL) {
    mi_segment_reclaim(segment, heap, 0, NULL, tld);
  }
}

static mi_segment_t* mi_segment_try_reclaim(mi_heap_t* heap, size_t block_size, mi_page_kind_t page_kind, bool* reclaimed, mi_segments_tld_t* tld)
{
  *reclaimed = false;
  mi_segment_t* segment;
  int max_tries = 8;     // limit the work to bound allocation times
  while ((max_tries-- > 0) && ((segment = mi_abandoned_pop()) != NULL)) {
    segment->abandoned_visits++;
    bool all_pages_free;
    bool has_page = mi_segment_check_free(segment,block_size,&all_pages_free); // try to free up pages (due to concurrent frees)
    if (all_pages_free) {
      // free the segment (by forced reclaim) to make it available to other threads.
      // note1: we prefer to free a segment as that might lead to reclaiming another
      // segment that is still partially used.
      // note2: we could in principle optimize this by skipping reclaim and directly
      // freeing but that would violate some invariants temporarily)
      mi_segment_reclaim(segment, heap, 0, NULL, tld);
    }
    else if (has_page && segment->page_kind == page_kind) {
      // found a free page of the right kind, or page of the right block_size with free space
      // we return the result of reclaim (which is usually `segment`) as it might free
      // the segment due to concurrent frees (in which case `NULL` is returned).
      return mi_segment_reclaim(segment, heap, block_size, reclaimed, tld);
    }
    else if (segment->abandoned_visits >= 3) {
      // always reclaim on 3rd visit to limit the list length.
      mi_segment_reclaim(segment, heap, 0, NULL, tld);
    }
    else {
      // otherwise, push on the visited list so it gets not looked at too quickly again
      mi_abandoned_visited_push(segment);
    }
  }
  return NULL;
}


/* -----------------------------------------------------------
   Reclaim or allocate
----------------------------------------------------------- */

static mi_segment_t* mi_segment_reclaim_or_alloc(mi_heap_t* heap, size_t block_size, mi_page_kind_t page_kind, size_t page_shift, mi_segments_tld_t* tld, mi_os_tld_t* os_tld)
{
  mi_assert_internal(page_kind <= MI_PAGE_LARGE);
  mi_assert_internal(block_size < MI_HUGE_BLOCK_SIZE);
  // 1. try to get a segment from our cache
  mi_segment_t* segment = mi_segment_cache_pop(MI_SEGMENT_SIZE, tld);
  if (segment != NULL) {
    mi_segment_init(segment, 0, page_kind, page_shift, tld, os_tld);
    return segment;
  }
  // 2. try to reclaim an abandoned segment
  bool reclaimed;
  segment = mi_segment_try_reclaim(heap, block_size, page_kind, &reclaimed, tld);
  if (reclaimed) {
    // reclaimed the right page right into the heap
    mi_assert_internal(segment != NULL && segment->page_kind == page_kind && page_kind <= MI_PAGE_LARGE);
    return NULL; // pretend out-of-memory as the page will be in the page queue of the heap with available blocks
  }
  else if (segment != NULL) {
    // reclaimed a segment with empty pages (of `page_kind`) in it
    return segment;
  }
  // 3. otherwise allocate a fresh segment
  return mi_segment_alloc(0, page_kind, page_shift, tld, os_tld);
}


/* -----------------------------------------------------------
   Small page allocation
----------------------------------------------------------- */

static mi_page_t* mi_segment_find_free(mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_assert_internal(mi_segment_has_free(segment));
  mi_assert_expensive(mi_segment_is_valid(segment, tld));
  for (size_t i = 0; i < segment->capacity; i++) {  // TODO: use a bitmap instead of search?
    mi_page_t* page = &segment->pages[i];
    if (!page->segment_in_use) {
      bool ok = mi_segment_page_claim(segment, page, tld);
      if (ok) return page;
    }
  }
  mi_assert(false);
  return NULL;
}

// Allocate a page inside a segment. Requires that the page has free pages
static mi_page_t* mi_segment_page_alloc_in(mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_assert_internal(mi_segment_has_free(segment));
  return mi_segment_find_free(segment, tld);
}

static mi_page_t* mi_segment_page_alloc(mi_heap_t* heap, size_t block_size, mi_page_kind_t kind, size_t page_shift, mi_segments_tld_t* tld, mi_os_tld_t* os_tld) {
  // find an available segment the segment free queue
  mi_segment_queue_t* const free_queue = mi_segment_free_queue_of_kind(kind, tld);
  if (mi_segment_queue_is_empty(free_queue)) {
    // possibly allocate or reclaim a fresh segment
    mi_segment_t* const segment = mi_segment_reclaim_or_alloc(heap, block_size, kind, page_shift, tld, os_tld);
    if (segment == NULL) return NULL;  // return NULL if out-of-memory (or reclaimed)
    mi_assert_internal(free_queue->first == segment);
    mi_assert_internal(segment->page_kind==kind);
    mi_assert_internal(segment->used < segment->capacity);
  }
  mi_assert_internal(free_queue->first != NULL);
  mi_page_t* const page = mi_segment_page_alloc_in(free_queue->first, tld);
  mi_assert_internal(page != NULL);
#if MI_DEBUG>=2
  // verify it is committed
  _mi_segment_page_start(_mi_page_segment(page), page, sizeof(void*), NULL, NULL)[0] = 0;
#endif
  return page;
}

static mi_page_t* mi_segment_small_page_alloc(mi_heap_t* heap, size_t block_size, mi_segments_tld_t* tld, mi_os_tld_t* os_tld) {
  return mi_segment_page_alloc(heap, block_size, MI_PAGE_SMALL,MI_SMALL_PAGE_SHIFT,tld,os_tld);
}

static mi_page_t* mi_segment_medium_page_alloc(mi_heap_t* heap, size_t block_size, mi_segments_tld_t* tld, mi_os_tld_t* os_tld) {
  return mi_segment_page_alloc(heap, block_size, MI_PAGE_MEDIUM, MI_MEDIUM_PAGE_SHIFT, tld, os_tld);
}

/* -----------------------------------------------------------
   large page allocation
----------------------------------------------------------- */

static mi_page_t* mi_segment_large_page_alloc(mi_heap_t* heap, size_t block_size, mi_segments_tld_t* tld, mi_os_tld_t* os_tld) {
  mi_segment_t* segment = mi_segment_reclaim_or_alloc(heap,block_size,MI_PAGE_LARGE,MI_LARGE_PAGE_SHIFT,tld,os_tld);
  if (segment == NULL) return NULL;
  mi_page_t* page = mi_segment_find_free(segment, tld);
  mi_assert_internal(page != NULL);
#if MI_DEBUG>=2
  _mi_segment_page_start(segment, page, sizeof(void*), NULL, NULL)[0] = 0;
#endif
  return page;
}

static mi_page_t* mi_segment_huge_page_alloc(size_t size, mi_segments_tld_t* tld, mi_os_tld_t* os_tld)
{
  mi_segment_t* segment = mi_segment_alloc(size, MI_PAGE_HUGE, MI_SEGMENT_SHIFT,tld,os_tld);
  if (segment == NULL) return NULL;
  mi_assert_internal(mi_segment_page_size(segment) - segment->segment_info_size - (2*(MI_SECURE == 0 ? 0 : _mi_os_page_size())) >= size);
  segment->thread_id = 0; // huge pages are immediately abandoned
  mi_segments_track_size(-(long)segment->segment_size, tld);
  mi_page_t* page = mi_segment_find_free(segment, tld);
  mi_assert_internal(page != NULL);
  return page;
}

// free huge block from another thread
void _mi_segment_huge_page_free(mi_segment_t* segment, mi_page_t* page, mi_block_t* block) {
  // huge page segments are always abandoned and can be freed immediately by any thread
  mi_assert_internal(segment->page_kind==MI_PAGE_HUGE);
  mi_assert_internal(segment == _mi_page_segment(page));
  mi_assert_internal(mi_atomic_read_relaxed(&segment->thread_id)==0);

  // claim it and free
  mi_heap_t* heap = mi_heap_get_default(); // issue #221; don't use the internal get_default_heap as we need to ensure the thread is initialized.
  // paranoia: if this it the last reference, the cas should always succeed
  if (mi_atomic_cas_strong(&segment->thread_id, heap->thread_id, 0)) {
    mi_block_set_next(page, block, page->free);
    page->free = block;
    page->used--;
    page->is_zero = false;
    mi_assert(page->used == 0);
    mi_tld_t* tld = heap->tld;
    const size_t bsize = mi_page_usable_block_size(page);
    if (bsize > MI_HUGE_OBJ_SIZE_MAX) {
      _mi_stat_decrease(&tld->stats.giant, bsize); 
    }
    else {
      _mi_stat_decrease(&tld->stats.huge, bsize);
    }
    mi_segments_track_size((long)segment->segment_size, &tld->segments);
    _mi_segment_page_free(page, true, &tld->segments);
  }
}

/* -----------------------------------------------------------
   Page allocation
----------------------------------------------------------- */

mi_page_t* _mi_segment_page_alloc(mi_heap_t* heap, size_t block_size, mi_segments_tld_t* tld, mi_os_tld_t* os_tld) {
  mi_page_t* page;
  if (block_size <= MI_SMALL_OBJ_SIZE_MAX) {
    page = mi_segment_small_page_alloc(heap, block_size, tld, os_tld);
  }
  else if (block_size <= MI_MEDIUM_OBJ_SIZE_MAX) {
    page = mi_segment_medium_page_alloc(heap, block_size, tld, os_tld);
  }
  else if (block_size <= MI_LARGE_OBJ_SIZE_MAX) {
    page = mi_segment_large_page_alloc(heap, block_size, tld, os_tld);
  }
  else {
    page = mi_segment_huge_page_alloc(block_size,tld,os_tld);
  }
  mi_assert_expensive(page == NULL || mi_segment_is_valid(_mi_page_segment(page),tld));
  mi_assert_internal(page == NULL || (mi_segment_page_size(_mi_page_segment(page)) - (MI_SECURE == 0 ? 0 : _mi_os_page_size())) >= block_size);
  mi_reset_delayed(tld);
  mi_assert_internal(page == NULL || mi_page_not_in_queue(page, tld));
  return page;
}
/**** ended inlining segment.c ****/
/**** start inlining page.c ****/
/*----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/* -----------------------------------------------------------
  The core of the allocator. Every segment contains
  pages of a {certain block size. The main function
  exported is `mi_malloc_generic`.
----------------------------------------------------------- */

/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/
/**** skipping file: mimalloc-atomic.h ****/

/* -----------------------------------------------------------
  Definition of page queues for each block size
----------------------------------------------------------- */

#define MI_IN_PAGE_C
/**** start inlining page-queue.c ****/
/*----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/* -----------------------------------------------------------
  Definition of page queues for each block size
----------------------------------------------------------- */

#ifndef MI_IN_PAGE_C
#error "this file should be included from 'page.c'"
#endif

/* -----------------------------------------------------------
  Minimal alignment in machine words (i.e. `sizeof(void*)`)
----------------------------------------------------------- */

#if (MI_MAX_ALIGN_SIZE > 4*MI_INTPTR_SIZE)
  #error "define alignment for more than 4x word size for this platform"
#elif (MI_MAX_ALIGN_SIZE > 2*MI_INTPTR_SIZE)
  #define MI_ALIGN4W   // 4 machine words minimal alignment
#elif (MI_MAX_ALIGN_SIZE > MI_INTPTR_SIZE)
  #define MI_ALIGN2W   // 2 machine words minimal alignment
#else
  // ok, default alignment is 1 word
#endif


/* -----------------------------------------------------------
  Queue query
----------------------------------------------------------- */


static inline bool mi_page_queue_is_huge(const mi_page_queue_t* pq) {
  return (pq->block_size == (MI_LARGE_OBJ_SIZE_MAX+sizeof(uintptr_t)));
}

static inline bool mi_page_queue_is_full(const mi_page_queue_t* pq) {
  return (pq->block_size == (MI_LARGE_OBJ_SIZE_MAX+(2*sizeof(uintptr_t))));
}

static inline bool mi_page_queue_is_special(const mi_page_queue_t* pq) {
  return (pq->block_size > MI_LARGE_OBJ_SIZE_MAX);
}

/* -----------------------------------------------------------
  Bins
----------------------------------------------------------- */

// Bit scan reverse: return the index of the highest bit.
static inline uint8_t mi_bsr32(uint32_t x);

#if defined(_MSC_VER)
#include <intrin.h>
static inline uint8_t mi_bsr32(uint32_t x) {
  uint32_t idx;
  _BitScanReverse((DWORD*)&idx, x);
  return (uint8_t)idx;
}
#elif defined(__GNUC__) || defined(__clang__)
static inline uint8_t mi_bsr32(uint32_t x) {
  return (31 - __builtin_clz(x));
}
#else
static inline uint8_t mi_bsr32(uint32_t x) {
  // de Bruijn multiplication, see <http://supertech.csail.mit.edu/papers/debruijn.pdf>
  static const uint8_t debruijn[32] = {
     31,  0, 22,  1, 28, 23, 18,  2, 29, 26, 24, 10, 19,  7,  3, 12,
     30, 21, 27, 17, 25,  9,  6, 11, 20, 16,  8,  5, 15,  4, 14, 13,
  };
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x++;
  return debruijn[(x*0x076be629) >> 27];
}
#endif

// Bit scan reverse: return the index of the highest bit.
uint8_t _mi_bsr(uintptr_t x) {
  if (x == 0) return 0;
#if MI_INTPTR_SIZE==8
  uint32_t hi = (x >> 32);
  return (hi == 0 ? mi_bsr32((uint32_t)x) : 32 + mi_bsr32(hi));
#elif MI_INTPTR_SIZE==4
  return mi_bsr32(x);
#else
# error "define bsr for non-32 or 64-bit platforms"
#endif
}

// Return the bin for a given field size.
// Returns MI_BIN_HUGE if the size is too large.
// We use `wsize` for the size in "machine word sizes",
// i.e. byte size == `wsize*sizeof(void*)`.
extern inline uint8_t _mi_bin(size_t size) {
  size_t wsize = _mi_wsize_from_size(size);
  uint8_t bin;
  if (wsize <= 1) {
    bin = 1;
  }
  #if defined(MI_ALIGN4W)
  else if (wsize <= 4) {
    bin = (uint8_t)((wsize+1)&~1); // round to double word sizes
  }
  #elif defined(MI_ALIGN2W)
  else if (wsize <= 8) {
    bin = (uint8_t)((wsize+1)&~1); // round to double word sizes
  }
  #else
  else if (wsize <= 8) {
    bin = (uint8_t)wsize;
  }
  #endif
  else if (wsize > MI_LARGE_OBJ_WSIZE_MAX) {
    bin = MI_BIN_HUGE;
  }
  else {
    #if defined(MI_ALIGN4W) 
    if (wsize <= 16) { wsize = (wsize+3)&~3; } // round to 4x word sizes
    #endif
    wsize--;
    // find the highest bit
    uint8_t b = mi_bsr32((uint32_t)wsize);
    // and use the top 3 bits to determine the bin (~12.5% worst internal fragmentation).
    // - adjust with 3 because we use do not round the first 8 sizes
    //   which each get an exact bin
    bin = ((b << 2) + (uint8_t)((wsize >> (b - 2)) & 0x03)) - 3;
    mi_assert_internal(bin < MI_BIN_HUGE);
  }
  mi_assert_internal(bin > 0 && bin <= MI_BIN_HUGE);
  return bin;
}



/* -----------------------------------------------------------
  Queue of pages with free blocks
----------------------------------------------------------- */

size_t _mi_bin_size(uint8_t bin) {
  return _mi_heap_empty.pages[bin].block_size;
}

// Good size for allocation
size_t mi_good_size(size_t size) mi_attr_noexcept {
  if (size <= MI_LARGE_OBJ_SIZE_MAX) {
    return _mi_bin_size(_mi_bin(size));
  }
  else {
    return _mi_align_up(size,_mi_os_page_size());
  }
}

#if (MI_DEBUG>1)
static bool mi_page_queue_contains(mi_page_queue_t* queue, const mi_page_t* page) {
  mi_assert_internal(page != NULL);
  mi_page_t* list = queue->first;
  while (list != NULL) {
    mi_assert_internal(list->next == NULL || list->next->prev == list);
    mi_assert_internal(list->prev == NULL || list->prev->next == list);
    if (list == page) break;
    list = list->next;
  }
  return (list == page);
}

#endif

#if (MI_DEBUG>1)
static bool mi_heap_contains_queue(const mi_heap_t* heap, const mi_page_queue_t* pq) {
  return (pq >= &heap->pages[0] && pq <= &heap->pages[MI_BIN_FULL]);
}
#endif

static mi_page_queue_t* mi_page_queue_of(const mi_page_t* page) {
  uint8_t bin = (mi_page_is_in_full(page) ? MI_BIN_FULL : _mi_bin(page->xblock_size));
  mi_heap_t* heap = mi_page_heap(page);
  mi_assert_internal(heap != NULL && bin <= MI_BIN_FULL);
  mi_page_queue_t* pq = &heap->pages[bin];
  mi_assert_internal(bin >= MI_BIN_HUGE || page->xblock_size == pq->block_size);
  mi_assert_expensive(mi_page_queue_contains(pq, page));
  return pq;
}

static mi_page_queue_t* mi_heap_page_queue_of(mi_heap_t* heap, const mi_page_t* page) {
  uint8_t bin = (mi_page_is_in_full(page) ? MI_BIN_FULL : _mi_bin(page->xblock_size));
  mi_assert_internal(bin <= MI_BIN_FULL);
  mi_page_queue_t* pq = &heap->pages[bin];
  mi_assert_internal(mi_page_is_in_full(page) || page->xblock_size == pq->block_size);
  return pq;
}

// The current small page array is for efficiency and for each
// small size (up to 256) it points directly to the page for that
// size without having to compute the bin. This means when the
// current free page queue is updated for a small bin, we need to update a
// range of entries in `_mi_page_small_free`.
static inline void mi_heap_queue_first_update(mi_heap_t* heap, const mi_page_queue_t* pq) {
  mi_assert_internal(mi_heap_contains_queue(heap,pq));
  size_t size = pq->block_size;
  if (size > MI_SMALL_SIZE_MAX) return;

  mi_page_t* page = pq->first;
  if (pq->first == NULL) page = (mi_page_t*)&_mi_page_empty;

  // find index in the right direct page array
  size_t start;
  size_t idx = _mi_wsize_from_size(size);
  mi_page_t** pages_free = heap->pages_free_direct;

  if (pages_free[idx] == page) return;  // already set

  // find start slot
  if (idx<=1) {
    start = 0;
  }
  else {
    // find previous size; due to minimal alignment upto 3 previous bins may need to be skipped
    uint8_t bin = _mi_bin(size);
    const mi_page_queue_t* prev = pq - 1;
    while( bin == _mi_bin(prev->block_size) && prev > &heap->pages[0]) {
      prev--;
    }
    start = 1 + _mi_wsize_from_size(prev->block_size);
    if (start > idx) start = idx;
  }

  // set size range to the right page
  mi_assert(start <= idx);
  for (size_t sz = start; sz <= idx; sz++) {
    pages_free[sz] = page;
  }
}

/*
static bool mi_page_queue_is_empty(mi_page_queue_t* queue) {
  return (queue->first == NULL);
}
*/

static void mi_page_queue_remove(mi_page_queue_t* queue, mi_page_t* page) {
  mi_assert_internal(page != NULL);
  mi_assert_expensive(mi_page_queue_contains(queue, page));
  mi_assert_internal(page->xblock_size == queue->block_size || (page->xblock_size > MI_LARGE_OBJ_SIZE_MAX && mi_page_queue_is_huge(queue))  || (mi_page_is_in_full(page) && mi_page_queue_is_full(queue)));
  mi_heap_t* heap = mi_page_heap(page);
  if (page->prev != NULL) page->prev->next = page->next;
  if (page->next != NULL) page->next->prev = page->prev;
  if (page == queue->last)  queue->last = page->prev;
  if (page == queue->first) {
    queue->first = page->next;
    // update first
    mi_assert_internal(mi_heap_contains_queue(heap, queue));
    mi_heap_queue_first_update(heap,queue);
  }
  heap->page_count--;
  page->next = NULL;
  page->prev = NULL;
  // mi_atomic_write_ptr(mi_atomic_cast(void*, &page->heap), NULL);
  mi_page_set_in_full(page,false);
}


static void mi_page_queue_push(mi_heap_t* heap, mi_page_queue_t* queue, mi_page_t* page) {
  mi_assert_internal(mi_page_heap(page) == heap);
  mi_assert_internal(!mi_page_queue_contains(queue, page));
  mi_assert_internal(_mi_page_segment(page)->page_kind != MI_PAGE_HUGE);
  mi_assert_internal(page->xblock_size == queue->block_size ||
                      (page->xblock_size > MI_LARGE_OBJ_SIZE_MAX && mi_page_queue_is_huge(queue)) ||
                        (mi_page_is_in_full(page) && mi_page_queue_is_full(queue)));

  mi_page_set_in_full(page, mi_page_queue_is_full(queue));
  // mi_atomic_write_ptr(mi_atomic_cast(void*, &page->heap), heap);
  page->next = queue->first;
  page->prev = NULL;
  if (queue->first != NULL) {
    mi_assert_internal(queue->first->prev == NULL);
    queue->first->prev = page;
    queue->first = page;
  }
  else {
    queue->first = queue->last = page;
  }

  // update direct
  mi_heap_queue_first_update(heap, queue);
  heap->page_count++;
}


static void mi_page_queue_enqueue_from(mi_page_queue_t* to, mi_page_queue_t* from, mi_page_t* page) {
  mi_assert_internal(page != NULL);
  mi_assert_expensive(mi_page_queue_contains(from, page));
  mi_assert_expensive(!mi_page_queue_contains(to, page));
  mi_assert_internal((page->xblock_size == to->block_size && page->xblock_size == from->block_size) ||
                     (page->xblock_size == to->block_size && mi_page_queue_is_full(from)) ||
                     (page->xblock_size == from->block_size && mi_page_queue_is_full(to)) ||
                     (page->xblock_size > MI_LARGE_OBJ_SIZE_MAX && mi_page_queue_is_huge(to)) ||
                     (page->xblock_size > MI_LARGE_OBJ_SIZE_MAX && mi_page_queue_is_full(to)));

  mi_heap_t* heap = mi_page_heap(page);
  if (page->prev != NULL) page->prev->next = page->next;
  if (page->next != NULL) page->next->prev = page->prev;
  if (page == from->last)  from->last = page->prev;
  if (page == from->first) {
    from->first = page->next;
    // update first
    mi_assert_internal(mi_heap_contains_queue(heap, from));
    mi_heap_queue_first_update(heap, from);
  }

  page->prev = to->last;
  page->next = NULL;
  if (to->last != NULL) {
    mi_assert_internal(heap == mi_page_heap(to->last));
    to->last->next = page;
    to->last = page;
  }
  else {
    to->first = page;
    to->last = page;
    mi_heap_queue_first_update(heap, to);
  }

  mi_page_set_in_full(page, mi_page_queue_is_full(to));
}

// Only called from `mi_heap_absorb`.
size_t _mi_page_queue_append(mi_heap_t* heap, mi_page_queue_t* pq, mi_page_queue_t* append) {
  mi_assert_internal(mi_heap_contains_queue(heap,pq));
  mi_assert_internal(pq->block_size == append->block_size);

  if (append->first==NULL) return 0;

  // set append pages to new heap and count
  size_t count = 0;
  for (mi_page_t* page = append->first; page != NULL; page = page->next) {
    // inline `mi_page_set_heap` to avoid wrong assertion during absorption;
    // in this case it is ok to be delayed freeing since both "to" and "from" heap are still alive.
    mi_atomic_write(&page->xheap, (uintptr_t)heap); 
    // set the flag to delayed free (not overriding NEVER_DELAYED_FREE) which has as a
    // side effect that it spins until any DELAYED_FREEING is finished. This ensures
    // that after appending only the new heap will be used for delayed free operations.
    _mi_page_use_delayed_free(page, MI_USE_DELAYED_FREE, false);
    count++;
  }

  if (pq->last==NULL) {
    // take over afresh
    mi_assert_internal(pq->first==NULL);
    pq->first = append->first;
    pq->last = append->last;
    mi_heap_queue_first_update(heap, pq);
  }
  else {
    // append to end
    mi_assert_internal(pq->last!=NULL);
    mi_assert_internal(append->first!=NULL);
    pq->last->next = append->first;
    append->first->prev = pq->last;
    pq->last = append->last;
  }
  return count;
}
/**** ended inlining page-queue.c ****/
#undef MI_IN_PAGE_C


/* -----------------------------------------------------------
  Page helpers
----------------------------------------------------------- */

// Index a block in a page
static inline mi_block_t* mi_page_block_at(const mi_page_t* page, void* page_start, size_t block_size, size_t i) {
  UNUSED(page);
  mi_assert_internal(page != NULL);
  mi_assert_internal(i <= page->reserved);
  return (mi_block_t*)((uint8_t*)page_start + (i * block_size));
}

static void mi_page_init(mi_heap_t* heap, mi_page_t* page, size_t size, mi_tld_t* tld);
static void mi_page_extend_free(mi_heap_t* heap, mi_page_t* page, mi_tld_t* tld);

#if (MI_DEBUG>=3)
static size_t mi_page_list_count(mi_page_t* page, mi_block_t* head) {
  size_t count = 0;
  while (head != NULL) {
    mi_assert_internal(page == _mi_ptr_page(head));
    count++;
    head = mi_block_next(page, head);
  }
  return count;
}

/*
// Start of the page available memory
static inline uint8_t* mi_page_area(const mi_page_t* page) {
  return _mi_page_start(_mi_page_segment(page), page, NULL);
}
*/

static bool mi_page_list_is_valid(mi_page_t* page, mi_block_t* p) {
  size_t psize;
  uint8_t* page_area = _mi_page_start(_mi_page_segment(page), page, &psize);
  mi_block_t* start = (mi_block_t*)page_area;
  mi_block_t* end   = (mi_block_t*)(page_area + psize);
  while(p != NULL) {
    if (p < start || p >= end) return false;
    p = mi_block_next(page, p);
  }
  return true;
}

static bool mi_page_is_valid_init(mi_page_t* page) {
  mi_assert_internal(page->xblock_size > 0);
  mi_assert_internal(page->used <= page->capacity);
  mi_assert_internal(page->capacity <= page->reserved);

  const size_t bsize = mi_page_block_size(page);
  mi_segment_t* segment = _mi_page_segment(page);
  uint8_t* start = _mi_page_start(segment,page,NULL);
  mi_assert_internal(start == _mi_segment_page_start(segment,page,bsize,NULL,NULL));
  //mi_assert_internal(start + page->capacity*page->block_size == page->top);

  mi_assert_internal(mi_page_list_is_valid(page,page->free));
  mi_assert_internal(mi_page_list_is_valid(page,page->local_free));

  #if MI_DEBUG>3 // generally too expensive to check this
  if (page->flags.is_zero) {
    for(mi_block_t* block = page->free; block != NULL; mi_block_next(page,block)) {
      mi_assert_expensive(mi_mem_is_zero(block + 1, page->block_size - sizeof(mi_block_t)));
    }
  }
  #endif

  mi_block_t* tfree = mi_page_thread_free(page);
  mi_assert_internal(mi_page_list_is_valid(page, tfree));
  //size_t tfree_count = mi_page_list_count(page, tfree);
  //mi_assert_internal(tfree_count <= page->thread_freed + 1);

  size_t free_count = mi_page_list_count(page, page->free) + mi_page_list_count(page, page->local_free);
  mi_assert_internal(page->used + free_count == page->capacity);

  return true;
}

bool _mi_page_is_valid(mi_page_t* page) {
  mi_assert_internal(mi_page_is_valid_init(page));
  #if MI_SECURE
  mi_assert_internal(page->keys[0] != 0);
  #endif
  if (mi_page_heap(page)!=NULL) {
    mi_segment_t* segment = _mi_page_segment(page);
    mi_assert_internal(!_mi_process_is_initialized || segment->thread_id == mi_page_heap(page)->thread_id || segment->thread_id==0);
    if (segment->page_kind != MI_PAGE_HUGE) {
      mi_page_queue_t* pq = mi_page_queue_of(page);
      mi_assert_internal(mi_page_queue_contains(pq, page));
      mi_assert_internal(pq->block_size==mi_page_block_size(page) || mi_page_block_size(page) > MI_LARGE_OBJ_SIZE_MAX || mi_page_is_in_full(page));
      mi_assert_internal(mi_heap_contains_queue(mi_page_heap(page),pq));
    }
  }
  return true;
}
#endif

void _mi_page_use_delayed_free(mi_page_t* page, mi_delayed_t delay, bool override_never) {
  mi_thread_free_t tfree;
  mi_thread_free_t tfreex;
  mi_delayed_t     old_delay;
  do {
    tfree = mi_atomic_read(&page->xthread_free);  // note: must acquire as we can break this loop and not do a CAS
    tfreex = mi_tf_set_delayed(tfree, delay);
    old_delay = mi_tf_delayed(tfree);
    if (mi_unlikely(old_delay == MI_DELAYED_FREEING)) {
      mi_atomic_yield(); // delay until outstanding MI_DELAYED_FREEING are done.
      // tfree = mi_tf_set_delayed(tfree, MI_NO_DELAYED_FREE); // will cause CAS to busy fail
    }
    else if (delay == old_delay) {
      break; // avoid atomic operation if already equal
    }
    else if (!override_never && old_delay == MI_NEVER_DELAYED_FREE) {
      break; // leave never-delayed flag set
    }
  } while ((old_delay == MI_DELAYED_FREEING) ||
           !mi_atomic_cas_weak(&page->xthread_free, tfreex, tfree));
}

/* -----------------------------------------------------------
  Page collect the `local_free` and `thread_free` lists
----------------------------------------------------------- */

// Collect the local `thread_free` list using an atomic exchange.
// Note: The exchange must be done atomically as this is used right after
// moving to the full list in `mi_page_collect_ex` and we need to
// ensure that there was no race where the page became unfull just before the move.
static void _mi_page_thread_free_collect(mi_page_t* page)
{
  mi_block_t* head;
  mi_thread_free_t tfree;
  mi_thread_free_t tfreex;
  do {
    tfree = mi_atomic_read_relaxed(&page->xthread_free);
    head = mi_tf_block(tfree);
    tfreex = mi_tf_set_block(tfree,NULL);
  } while (!mi_atomic_cas_weak(&page->xthread_free, tfreex, tfree));

  // return if the list is empty
  if (head == NULL) return;

  // find the tail -- also to get a proper count (without data races)
  uint32_t max_count = page->capacity; // cannot collect more than capacity
  uint32_t count = 1;
  mi_block_t* tail = head;
  mi_block_t* next;
  while ((next = mi_block_next(page,tail)) != NULL && count <= max_count) {
    count++;
    tail = next;
  }
  // if `count > max_count` there was a memory corruption (possibly infinite list due to double multi-threaded free)
  if (count > max_count) {
    _mi_error_message(EFAULT, "corrupted thread-free list\n");
    return; // the thread-free items cannot be freed
  }

  // and append the current local free list
  mi_block_set_next(page,tail, page->local_free);
  page->local_free = head;

  // update counts now
  page->used -= count;
}

void _mi_page_free_collect(mi_page_t* page, bool force) {
  mi_assert_internal(page!=NULL);

  // collect the thread free list
  if (force || mi_page_thread_free(page) != NULL) {  // quick test to avoid an atomic operation
    _mi_page_thread_free_collect(page);
  }

  // and the local free list
  if (page->local_free != NULL) {
    if (mi_likely(page->free == NULL)) {
      // usual case
      page->free = page->local_free;
      page->local_free = NULL;
      page->is_zero = false;
    }
    else if (force) {
      // append -- only on shutdown (force) as this is a linear operation
      mi_block_t* tail = page->local_free;
      mi_block_t* next;
      while ((next = mi_block_next(page, tail)) != NULL) {
        tail = next;
      }
      mi_block_set_next(page, tail, page->free);
      page->free = page->local_free;
      page->local_free = NULL;
      page->is_zero = false;
    }
  }

  mi_assert_internal(!force || page->local_free == NULL);
}



/* -----------------------------------------------------------
  Page fresh and retire
----------------------------------------------------------- */

// called from segments when reclaiming abandoned pages
void _mi_page_reclaim(mi_heap_t* heap, mi_page_t* page) {
  mi_assert_expensive(mi_page_is_valid_init(page));
  mi_assert_internal(mi_page_heap(page) == heap);
  mi_assert_internal(mi_page_thread_free_flag(page) != MI_NEVER_DELAYED_FREE);
  mi_assert_internal(_mi_page_segment(page)->page_kind != MI_PAGE_HUGE);
  mi_assert_internal(!page->is_reset);
  // TODO: push on full queue immediately if it is full?
  mi_page_queue_t* pq = mi_page_queue(heap, mi_page_block_size(page));
  mi_page_queue_push(heap, pq, page);
  mi_assert_expensive(_mi_page_is_valid(page));
}

// allocate a fresh page from a segment
static mi_page_t* mi_page_fresh_alloc(mi_heap_t* heap, mi_page_queue_t* pq, size_t block_size) {
  mi_assert_internal(pq==NULL||mi_heap_contains_queue(heap, pq));
  mi_assert_internal(pq==NULL||block_size == pq->block_size);
  mi_page_t* page = _mi_segment_page_alloc(heap, block_size, &heap->tld->segments, &heap->tld->os);
  if (page == NULL) {
    // this may be out-of-memory, or an abandoned page was reclaimed (and in our queue)
    return NULL;
  }
  // a fresh page was found, initialize it
  mi_assert_internal(pq==NULL || _mi_page_segment(page)->page_kind != MI_PAGE_HUGE);
  mi_page_init(heap, page, block_size, heap->tld);
  _mi_stat_increase(&heap->tld->stats.pages, 1);
  if (pq!=NULL) mi_page_queue_push(heap, pq, page); // huge pages use pq==NULL
  mi_assert_expensive(_mi_page_is_valid(page));
  return page;
}

// Get a fresh page to use
static mi_page_t* mi_page_fresh(mi_heap_t* heap, mi_page_queue_t* pq) {
  mi_assert_internal(mi_heap_contains_queue(heap, pq));
  mi_page_t* page = mi_page_fresh_alloc(heap, pq, pq->block_size);
  if (page==NULL) return NULL;
  mi_assert_internal(pq->block_size==mi_page_block_size(page));
  mi_assert_internal(pq==mi_page_queue(heap, mi_page_block_size(page)));
  return page;
}

/* -----------------------------------------------------------
   Do any delayed frees
   (put there by other threads if they deallocated in a full page)
----------------------------------------------------------- */
void _mi_heap_delayed_free(mi_heap_t* heap) {
  // take over the list (note: no atomic exchange is it is often NULL)
  mi_block_t* block;
  do {
    block = mi_atomic_read_ptr_relaxed(mi_block_t,&heap->thread_delayed_free);
  } while (block != NULL && !mi_atomic_cas_ptr_weak(mi_block_t,&heap->thread_delayed_free, NULL, block));

  // and free them all
  while(block != NULL) {
    mi_block_t* next = mi_block_nextx(heap,block, heap->keys);
    // use internal free instead of regular one to keep stats etc correct
    if (!_mi_free_delayed_block(block)) {
      // we might already start delayed freeing while another thread has not yet
      // reset the delayed_freeing flag; in that case delay it further by reinserting.
      mi_block_t* dfree;
      do {
        dfree = mi_atomic_read_ptr_relaxed(mi_block_t,&heap->thread_delayed_free);
        mi_block_set_nextx(heap, block, dfree, heap->keys);
      } while (!mi_atomic_cas_ptr_weak(mi_block_t,&heap->thread_delayed_free, block, dfree));
    }
    block = next;
  }
}

/* -----------------------------------------------------------
  Unfull, abandon, free and retire
----------------------------------------------------------- */

// Move a page from the full list back to a regular list
void _mi_page_unfull(mi_page_t* page) {
  mi_assert_internal(page != NULL);
  mi_assert_expensive(_mi_page_is_valid(page));
  mi_assert_internal(mi_page_is_in_full(page));
  if (!mi_page_is_in_full(page)) return;

  mi_heap_t* heap = mi_page_heap(page);
  mi_page_queue_t* pqfull = &heap->pages[MI_BIN_FULL];
  mi_page_set_in_full(page, false); // to get the right queue
  mi_page_queue_t* pq = mi_heap_page_queue_of(heap, page);
  mi_page_set_in_full(page, true);
  mi_page_queue_enqueue_from(pq, pqfull, page);
}

static void mi_page_to_full(mi_page_t* page, mi_page_queue_t* pq) {
  mi_assert_internal(pq == mi_page_queue_of(page));
  mi_assert_internal(!mi_page_immediate_available(page));
  mi_assert_internal(!mi_page_is_in_full(page));

  if (mi_page_is_in_full(page)) return;
  mi_page_queue_enqueue_from(&mi_page_heap(page)->pages[MI_BIN_FULL], pq, page);
  _mi_page_free_collect(page,false);  // try to collect right away in case another thread freed just before MI_USE_DELAYED_FREE was set
}


// Abandon a page with used blocks at the end of a thread.
// Note: only call if it is ensured that no references exist from
// the `page->heap->thread_delayed_free` into this page.
// Currently only called through `mi_heap_collect_ex` which ensures this.
void _mi_page_abandon(mi_page_t* page, mi_page_queue_t* pq) {
  mi_assert_internal(page != NULL);
  mi_assert_expensive(_mi_page_is_valid(page));
  mi_assert_internal(pq == mi_page_queue_of(page));
  mi_assert_internal(mi_page_heap(page) != NULL);

  mi_heap_t* pheap = mi_page_heap(page);

  // remove from our page list
  mi_segments_tld_t* segments_tld = &pheap->tld->segments;
  mi_page_queue_remove(pq, page);

  // page is no longer associated with our heap
  mi_assert_internal(mi_page_thread_free_flag(page)==MI_NEVER_DELAYED_FREE);
  mi_page_set_heap(page, NULL);

#if MI_DEBUG>1
  // check there are no references left..
  for (mi_block_t* block = (mi_block_t*)pheap->thread_delayed_free; block != NULL; block = mi_block_nextx(pheap, block, pheap->keys)) {
    mi_assert_internal(_mi_ptr_page(block) != page);
  }
#endif

  // and abandon it
  mi_assert_internal(mi_page_heap(page) == NULL);
  _mi_segment_page_abandon(page,segments_tld);
}


// Free a page with no more free blocks
void _mi_page_free(mi_page_t* page, mi_page_queue_t* pq, bool force) {
  mi_assert_internal(page != NULL);
  mi_assert_expensive(_mi_page_is_valid(page));
  mi_assert_internal(pq == mi_page_queue_of(page));
  mi_assert_internal(mi_page_all_free(page));
  mi_assert_internal(mi_page_thread_free_flag(page)!=MI_DELAYED_FREEING);

  // no more aligned blocks in here
  mi_page_set_has_aligned(page, false);

  // remove from the page list
  // (no need to do _mi_heap_delayed_free first as all blocks are already free)
  mi_segments_tld_t* segments_tld = &mi_page_heap(page)->tld->segments;
  mi_page_queue_remove(pq, page);

  // and free it
  mi_page_set_heap(page,NULL);
  _mi_segment_page_free(page, force, segments_tld);
}

#define MI_MAX_RETIRE_SIZE    MI_LARGE_OBJ_SIZE_MAX  
#define MI_RETIRE_CYCLES      (8)

// Retire a page with no more used blocks
// Important to not retire too quickly though as new
// allocations might coming.
// Note: called from `mi_free` and benchmarks often
// trigger this due to freeing everything and then
// allocating again so careful when changing this.
void _mi_page_retire(mi_page_t* page) {
  mi_assert_internal(page != NULL);
  mi_assert_expensive(_mi_page_is_valid(page));
  mi_assert_internal(mi_page_all_free(page));

  mi_page_set_has_aligned(page, false);

  // don't retire too often..
  // (or we end up retiring and re-allocating most of the time)
  // NOTE: refine this more: we should not retire if this
  // is the only page left with free blocks. It is not clear
  // how to check this efficiently though...
  // for now, we don't retire if it is the only page left of this size class.
  mi_page_queue_t* pq = mi_page_queue_of(page);
  if (mi_likely(page->xblock_size <= MI_MAX_RETIRE_SIZE && !mi_page_is_in_full(page))) {
    if (pq->last==page && pq->first==page) { // the only page in the queue?
      mi_stat_counter_increase(_mi_stats_main.page_no_retire,1);
      page->retire_expire = (page->xblock_size <= MI_SMALL_OBJ_SIZE_MAX ? MI_RETIRE_CYCLES : MI_RETIRE_CYCLES/4);
      mi_heap_t* heap = mi_page_heap(page);
      mi_assert_internal(pq >= heap->pages);
      const size_t index = pq - heap->pages;
      mi_assert_internal(index < MI_BIN_FULL && index < MI_BIN_HUGE);
      if (index < heap->page_retired_min) heap->page_retired_min = index;
      if (index > heap->page_retired_max) heap->page_retired_max = index;
      mi_assert_internal(mi_page_all_free(page));
      return; // dont't free after all
    }
  }

  _mi_page_free(page, pq, false);
}

// free retired pages: we don't need to look at the entire queues
// since we only retire pages that are at the head position in a queue.
void _mi_heap_collect_retired(mi_heap_t* heap, bool force) {
  size_t min = MI_BIN_FULL;
  size_t max = 0;
  for(size_t bin = heap->page_retired_min; bin <= heap->page_retired_max; bin++) {
    mi_page_queue_t* pq   = &heap->pages[bin];
    mi_page_t*       page = pq->first;
    if (page != NULL && page->retire_expire != 0) {
      if (mi_page_all_free(page)) {
        page->retire_expire--;
        if (force || page->retire_expire == 0) {
          _mi_page_free(pq->first, pq, force);
        }
        else {
          // keep retired, update min/max
          if (bin < min) min = bin;
          if (bin > max) max = bin;
        }
      }
      else {
        page->retire_expire = 0;
      }
    }
  }
  heap->page_retired_min = min;
  heap->page_retired_max = max;
}


/* -----------------------------------------------------------
  Initialize the initial free list in a page.
  In secure mode we initialize a randomized list by
  alternating between slices.
----------------------------------------------------------- */

#define MI_MAX_SLICE_SHIFT  (6)   // at most 64 slices
#define MI_MAX_SLICES       (1UL << MI_MAX_SLICE_SHIFT)
#define MI_MIN_SLICES       (2)

static void mi_page_free_list_extend_secure(mi_heap_t* const heap, mi_page_t* const page, const size_t bsize, const size_t extend, mi_stats_t* const stats) {
  UNUSED(stats);
  #if (MI_SECURE<=2)
  mi_assert_internal(page->free == NULL);
  mi_assert_internal(page->local_free == NULL);
  #endif
  mi_assert_internal(page->capacity + extend <= page->reserved);
  mi_assert_internal(bsize == mi_page_block_size(page));
  void* const page_area = _mi_page_start(_mi_page_segment(page), page, NULL);

  // initialize a randomized free list
  // set up `slice_count` slices to alternate between
  size_t shift = MI_MAX_SLICE_SHIFT;
  while ((extend >> shift) == 0) {
    shift--;
  }
  const size_t slice_count = (size_t)1U << shift;
  const size_t slice_extend = extend / slice_count;
  mi_assert_internal(slice_extend >= 1);
  mi_block_t* blocks[MI_MAX_SLICES];   // current start of the slice
  size_t      counts[MI_MAX_SLICES];   // available objects in the slice
  for (size_t i = 0; i < slice_count; i++) {
    blocks[i] = mi_page_block_at(page, page_area, bsize, page->capacity + i*slice_extend);
    counts[i] = slice_extend;
  }
  counts[slice_count-1] += (extend % slice_count);  // final slice holds the modulus too (todo: distribute evenly?)

  // and initialize the free list by randomly threading through them
  // set up first element
  const uintptr_t r = _mi_heap_random_next(heap);
  size_t current = r % slice_count;
  counts[current]--;
  mi_block_t* const free_start = blocks[current];
  // and iterate through the rest; use `random_shuffle` for performance
  uintptr_t rnd = _mi_random_shuffle(r|1); // ensure not 0
  for (size_t i = 1; i < extend; i++) {
    // call random_shuffle only every INTPTR_SIZE rounds
    const size_t round = i%MI_INTPTR_SIZE;
    if (round == 0) rnd = _mi_random_shuffle(rnd);
    // select a random next slice index
    size_t next = ((rnd >> 8*round) & (slice_count-1));
    while (counts[next]==0) {                            // ensure it still has space
      next++;
      if (next==slice_count) next = 0;
    }
    // and link the current block to it
    counts[next]--;
    mi_block_t* const block = blocks[current];
    blocks[current] = (mi_block_t*)((uint8_t*)block + bsize);  // bump to the following block
    mi_block_set_next(page, block, blocks[next]);   // and set next; note: we may have `current == next`
    current = next;
  }
  // prepend to the free list (usually NULL)
  mi_block_set_next(page, blocks[current], page->free);  // end of the list
  page->free = free_start;
}

static mi_decl_noinline void mi_page_free_list_extend( mi_page_t* const page, const size_t bsize, const size_t extend, mi_stats_t* const stats)
{
  UNUSED(stats);
  #if (MI_SECURE <= 2)
  mi_assert_internal(page->free == NULL);
  mi_assert_internal(page->local_free == NULL);
  #endif
  mi_assert_internal(page->capacity + extend <= page->reserved);
  mi_assert_internal(bsize == mi_page_block_size(page));
  void* const page_area = _mi_page_start(_mi_page_segment(page), page, NULL );

  mi_block_t* const start = mi_page_block_at(page, page_area, bsize, page->capacity);

  // initialize a sequential free list
  mi_block_t* const last = mi_page_block_at(page, page_area, bsize, page->capacity + extend - 1);
  mi_block_t* block = start;
  while(block <= last) {
    mi_block_t* next = (mi_block_t*)((uint8_t*)block + bsize);
    mi_block_set_next(page,block,next);
    block = next;
  }
  // prepend to free list (usually `NULL`)
  mi_block_set_next(page, last, page->free);
  page->free = start;
}

/* -----------------------------------------------------------
  Page initialize and extend the capacity
----------------------------------------------------------- */

#define MI_MAX_EXTEND_SIZE    (4*1024)      // heuristic, one OS page seems to work well.
#if (MI_SECURE>0)
#define MI_MIN_EXTEND         (8*MI_SECURE) // extend at least by this many
#else
#define MI_MIN_EXTEND         (1)
#endif

// Extend the capacity (up to reserved) by initializing a free list
// We do at most `MI_MAX_EXTEND` to avoid touching too much memory
// Note: we also experimented with "bump" allocation on the first
// allocations but this did not speed up any benchmark (due to an
// extra test in malloc? or cache effects?)
static void mi_page_extend_free(mi_heap_t* heap, mi_page_t* page, mi_tld_t* tld) {
  mi_assert_expensive(mi_page_is_valid_init(page));
  #if (MI_SECURE<=2)
  mi_assert(page->free == NULL);
  mi_assert(page->local_free == NULL);
  if (page->free != NULL) return;
  #endif
  if (page->capacity >= page->reserved) return;

  size_t page_size;
  //uint8_t* page_start = 
  _mi_page_start(_mi_page_segment(page), page, &page_size);
  mi_stat_counter_increase(tld->stats.pages_extended, 1);

  // calculate the extend count
  const size_t bsize = (page->xblock_size < MI_HUGE_BLOCK_SIZE ? page->xblock_size : page_size);
  size_t extend = page->reserved - page->capacity;
  size_t max_extend = (bsize >= MI_MAX_EXTEND_SIZE ? MI_MIN_EXTEND : MI_MAX_EXTEND_SIZE/(uint32_t)bsize);
  if (max_extend < MI_MIN_EXTEND) max_extend = MI_MIN_EXTEND;

  if (extend > max_extend) {
    // ensure we don't touch memory beyond the page to reduce page commit.
    // the `lean` benchmark tests this. Going from 1 to 8 increases rss by 50%.
    extend = (max_extend==0 ? 1 : max_extend);
  }

  mi_assert_internal(extend > 0 && extend + page->capacity <= page->reserved);
  mi_assert_internal(extend < (1UL<<16));

  // and append the extend the free list
  if (extend < MI_MIN_SLICES || MI_SECURE==0) { //!mi_option_is_enabled(mi_option_secure)) {
    mi_page_free_list_extend(page, bsize, extend, &tld->stats );
  }
  else {
    mi_page_free_list_extend_secure(heap, page, bsize, extend, &tld->stats);
  }
  // enable the new free list
  page->capacity += (uint16_t)extend;
  mi_stat_increase(tld->stats.page_committed, extend * bsize);

  // extension into zero initialized memory preserves the zero'd free list
  if (!page->is_zero_init) {
    page->is_zero = false;
  }
  mi_assert_expensive(mi_page_is_valid_init(page));
}

// Initialize a fresh page
static void mi_page_init(mi_heap_t* heap, mi_page_t* page, size_t block_size, mi_tld_t* tld) {
  mi_assert(page != NULL);
  mi_segment_t* segment = _mi_page_segment(page);
  mi_assert(segment != NULL);
  mi_assert_internal(block_size > 0);
  // set fields
  mi_page_set_heap(page, heap);
  size_t page_size;
  _mi_segment_page_start(segment, page, block_size, &page_size, NULL);
  page->xblock_size = (block_size < MI_HUGE_BLOCK_SIZE ? (uint32_t)block_size : MI_HUGE_BLOCK_SIZE);
  mi_assert_internal(page_size / block_size < (1L<<16));
  page->reserved = (uint16_t)(page_size / block_size);
  #ifdef MI_ENCODE_FREELIST
  page->keys[0] = _mi_heap_random_next(heap);
  page->keys[1] = _mi_heap_random_next(heap);
  #endif
  page->is_zero = page->is_zero_init;

  mi_assert_internal(page->capacity == 0);
  mi_assert_internal(page->free == NULL);
  mi_assert_internal(page->used == 0);
  mi_assert_internal(page->xthread_free == 0);
  mi_assert_internal(page->next == NULL);
  mi_assert_internal(page->prev == NULL);
  mi_assert_internal(page->retire_expire == 0);
  mi_assert_internal(!mi_page_has_aligned(page));
  #if (MI_ENCODE_FREELIST)
  mi_assert_internal(page->keys[0] != 0);
  mi_assert_internal(page->keys[1] != 0);
  #endif
  mi_assert_expensive(mi_page_is_valid_init(page));

  // initialize an initial free list
  mi_page_extend_free(heap,page,tld);
  mi_assert(mi_page_immediate_available(page));
}


/* -----------------------------------------------------------
  Find pages with free blocks
-------------------------------------------------------------*/

// Find a page with free blocks of `page->block_size`.
static mi_page_t* mi_page_queue_find_free_ex(mi_heap_t* heap, mi_page_queue_t* pq, bool first_try)
{
  // search through the pages in "next fit" order
  size_t count = 0;
  mi_page_t* page = pq->first;
  while (page != NULL)
  {
    mi_page_t* next = page->next; // remember next
    count++;

    // 0. collect freed blocks by us and other threads
    _mi_page_free_collect(page, false);

    // 1. if the page contains free blocks, we are done
    if (mi_page_immediate_available(page)) {
      break;  // pick this one
    }

    // 2. Try to extend
    if (page->capacity < page->reserved) {
      mi_page_extend_free(heap, page, heap->tld);
      mi_assert_internal(mi_page_immediate_available(page));
      break;
    }

    // 3. If the page is completely full, move it to the `mi_pages_full`
    // queue so we don't visit long-lived pages too often.
    mi_assert_internal(!mi_page_is_in_full(page) && !mi_page_immediate_available(page));
    mi_page_to_full(page, pq);

    page = next;
  } // for each page

  mi_stat_counter_increase(heap->tld->stats.searches, count);

  if (page == NULL) {
    _mi_heap_collect_retired(heap, false); // perhaps make a page available
    page = mi_page_fresh(heap, pq);
    if (page == NULL && first_try) {
      // out-of-memory _or_ an abandoned page with free blocks was reclaimed, try once again
      page = mi_page_queue_find_free_ex(heap, pq, false);      
    }
  }
  else {
    mi_assert(pq->first == page);
    page->retire_expire = 0;
  }
  mi_assert_internal(page == NULL || mi_page_immediate_available(page));
  return page;
}



// Find a page with free blocks of `size`.
static inline mi_page_t* mi_find_free_page(mi_heap_t* heap, size_t size) {
  mi_page_queue_t* pq = mi_page_queue(heap,size);
  mi_page_t* page = pq->first;
  if (page != NULL) {
    if ((MI_SECURE >= 3) && page->capacity < page->reserved && ((_mi_heap_random_next(heap) & 1) == 1)) {
      // in secure mode, we extend half the time to increase randomness
      mi_page_extend_free(heap, page, heap->tld);
      mi_assert_internal(mi_page_immediate_available(page));
    }
    else {
      _mi_page_free_collect(page,false);
    }
    if (mi_page_immediate_available(page)) {
      page->retire_expire = 0;
      return page; // fast path
    }
  }
  return mi_page_queue_find_free_ex(heap, pq, true);
}


/* -----------------------------------------------------------
  Users can register a deferred free function called
  when the `free` list is empty. Since the `local_free`
  is separate this is deterministically called after
  a certain number of allocations.
----------------------------------------------------------- */

static mi_deferred_free_fun* volatile deferred_free = NULL;
static volatile _Atomic(void*) deferred_arg; // = NULL

void _mi_deferred_free(mi_heap_t* heap, bool force) {
  heap->tld->heartbeat++;
  if (deferred_free != NULL && !heap->tld->recurse) {
    heap->tld->recurse = true;
    deferred_free(force, heap->tld->heartbeat, mi_atomic_read_ptr_relaxed(void,&deferred_arg));
    heap->tld->recurse = false;
  }
}

void mi_register_deferred_free(mi_deferred_free_fun* fn, void* arg) mi_attr_noexcept {
  deferred_free = fn;
  mi_atomic_write_ptr(void,&deferred_arg, arg);
}


/* -----------------------------------------------------------
  General allocation
----------------------------------------------------------- */

// A huge page is allocated directly without being in a queue.
// Because huge pages contain just one block, and the segment contains
// just that page, we always treat them as abandoned and any thread
// that frees the block can free the whole page and segment directly.
static mi_page_t* mi_huge_page_alloc(mi_heap_t* heap, size_t size) {
  size_t block_size = _mi_os_good_alloc_size(size);
  mi_assert_internal(_mi_bin(block_size) == MI_BIN_HUGE);
  mi_page_t* page = mi_page_fresh_alloc(heap,NULL,block_size);
  if (page != NULL) {
    const size_t bsize = mi_page_block_size(page);  // note: not `mi_page_usable_block_size` as `size` includes padding already
    mi_assert_internal(bsize >= size);
    mi_assert_internal(mi_page_immediate_available(page));
    mi_assert_internal(_mi_page_segment(page)->page_kind==MI_PAGE_HUGE);
    mi_assert_internal(_mi_page_segment(page)->used==1);
    mi_assert_internal(_mi_page_segment(page)->thread_id==0); // abandoned, not in the huge queue
    mi_page_set_heap(page, NULL);

    if (bsize > MI_HUGE_OBJ_SIZE_MAX) {
      _mi_stat_increase(&heap->tld->stats.giant, bsize);
      _mi_stat_counter_increase(&heap->tld->stats.giant_count, 1);
    }
    else {
      _mi_stat_increase(&heap->tld->stats.huge, bsize);
      _mi_stat_counter_increase(&heap->tld->stats.huge_count, 1);
    }
  }
  return page;
}


// Allocate a page
// Note: in debug mode the size includes MI_PADDING_SIZE and might have overflowed.
static mi_page_t* mi_find_page(mi_heap_t* heap, size_t size) mi_attr_noexcept {
  // huge allocation?
  const size_t req_size = size - MI_PADDING_SIZE;  // correct for padding_size in case of an overflow on `size`  
  if (mi_unlikely(req_size > (MI_LARGE_OBJ_SIZE_MAX - MI_PADDING_SIZE) )) {
    if (mi_unlikely(req_size > PTRDIFF_MAX)) {  // we don't allocate more than PTRDIFF_MAX (see <https://sourceware.org/ml/libc-announce/2019/msg00001.html>)
      _mi_error_message(EOVERFLOW, "allocation request is too large (%zu bytes)\n", req_size);
      return NULL;
    }
    else {
      return mi_huge_page_alloc(heap,size);
    }
  }
  else {
    // otherwise find a page with free blocks in our size segregated queues
    mi_assert_internal(size >= MI_PADDING_SIZE);
    return mi_find_free_page(heap, size);
  }
}

// Generic allocation routine if the fast path (`alloc.c:mi_page_malloc`) does not succeed.
// Note: in debug mode the size includes MI_PADDING_SIZE and might have overflowed.
void* _mi_malloc_generic(mi_heap_t* heap, size_t size) mi_attr_noexcept
{
  mi_assert_internal(heap != NULL);

  // initialize if necessary
  if (mi_unlikely(!mi_heap_is_initialized(heap))) {
    mi_thread_init(); // calls `_mi_heap_init` in turn
    heap = mi_get_default_heap();
    if (mi_unlikely(!mi_heap_is_initialized(heap))) { return NULL; }
  }
  mi_assert_internal(mi_heap_is_initialized(heap));

  // call potential deferred free routines
  _mi_deferred_free(heap, false);

  // free delayed frees from other threads
  _mi_heap_delayed_free(heap);

  // find (or allocate) a page of the right size
  mi_page_t* page = mi_find_page(heap, size);
  if (mi_unlikely(page == NULL)) { // first time out of memory, try to collect and retry the allocation once more
    mi_heap_collect(heap, true /* force */);
    page = mi_find_page(heap, size);
  }

  if (mi_unlikely(page == NULL)) { // out of memory
    const size_t req_size = size - MI_PADDING_SIZE;  // correct for padding_size in case of an overflow on `size`  
    _mi_error_message(ENOMEM, "unable to allocate memory (%zu bytes)\n", req_size);
    return NULL;
  }

  mi_assert_internal(mi_page_immediate_available(page));
  mi_assert_internal(mi_page_block_size(page) >= size);

  // and try again, this time succeeding! (i.e. this should never recurse)
  return _mi_page_malloc(heap, page, size);
}
/**** ended inlining page.c ****/
/**** start inlining heap.c ****/
/*----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/
/**** skipping file: mimalloc-atomic.h ****/

#include <string.h>  // memset, memcpy


/* -----------------------------------------------------------
  Helpers
----------------------------------------------------------- */

// return `true` if ok, `false` to break
typedef bool (heap_page_visitor_fun)(mi_heap_t* heap, mi_page_queue_t* pq, mi_page_t* page, void* arg1, void* arg2);

// Visit all pages in a heap; returns `false` if break was called.
static bool mi_heap_visit_pages(mi_heap_t* heap, heap_page_visitor_fun* fn, void* arg1, void* arg2)
{
  if (heap==NULL || heap->page_count==0) return 0;

  // visit all pages
  #if MI_DEBUG>1
  size_t total = heap->page_count;
  #endif
  size_t count = 0;
  for (size_t i = 0; i <= MI_BIN_FULL; i++) {
    mi_page_queue_t* pq = &heap->pages[i];
    mi_page_t* page = pq->first;
    while(page != NULL) {
      mi_page_t* next = page->next; // save next in case the page gets removed from the queue
      mi_assert_internal(mi_page_heap(page) == heap);
      count++;
      if (!fn(heap, pq, page, arg1, arg2)) return false;
      page = next; // and continue
    }
  }
  mi_assert_internal(count == total);
  return true;
}


#if MI_DEBUG>=2
static bool mi_heap_page_is_valid(mi_heap_t* heap, mi_page_queue_t* pq, mi_page_t* page, void* arg1, void* arg2) {
  UNUSED(arg1);
  UNUSED(arg2);
  UNUSED(pq);
  mi_assert_internal(mi_page_heap(page) == heap);
  mi_segment_t* segment = _mi_page_segment(page);
  mi_assert_internal(segment->thread_id == heap->thread_id);
  mi_assert_expensive(_mi_page_is_valid(page));
  return true;
}
#endif
#if MI_DEBUG>=3
static bool mi_heap_is_valid(mi_heap_t* heap) {
  mi_assert_internal(heap!=NULL);
  mi_heap_visit_pages(heap, &mi_heap_page_is_valid, NULL, NULL);
  return true;
}
#endif




/* -----------------------------------------------------------
  "Collect" pages by migrating `local_free` and `thread_free`
  lists and freeing empty pages. This is done when a thread
  stops (and in that case abandons pages if there are still
  blocks alive)
----------------------------------------------------------- */

typedef enum mi_collect_e {
  MI_NORMAL,
  MI_FORCE,
  MI_ABANDON
} mi_collect_t;


static bool mi_heap_page_collect(mi_heap_t* heap, mi_page_queue_t* pq, mi_page_t* page, void* arg_collect, void* arg2 ) {
  UNUSED(arg2);
  UNUSED(heap);
  mi_assert_internal(mi_heap_page_is_valid(heap, pq, page, NULL, NULL));
  mi_collect_t collect = *((mi_collect_t*)arg_collect);
  _mi_page_free_collect(page, collect >= MI_FORCE);
  if (mi_page_all_free(page)) {
    // no more used blocks, free the page. 
    // note: this will free retired pages as well.
    _mi_page_free(page, pq, collect >= MI_FORCE);
  }
  else if (collect == MI_ABANDON) {
    // still used blocks but the thread is done; abandon the page
    _mi_page_abandon(page, pq);
  }
  return true; // don't break
}

static bool mi_heap_page_never_delayed_free(mi_heap_t* heap, mi_page_queue_t* pq, mi_page_t* page, void* arg1, void* arg2) {
  UNUSED(arg1);
  UNUSED(arg2);
  UNUSED(heap);
  UNUSED(pq);
  _mi_page_use_delayed_free(page, MI_NEVER_DELAYED_FREE, false);
  return true; // don't break
}

static void mi_heap_collect_ex(mi_heap_t* heap, mi_collect_t collect)
{
  if (!mi_heap_is_initialized(heap)) return;
  _mi_deferred_free(heap, collect >= MI_FORCE);

  // note: never reclaim on collect but leave it to threads that need storage to reclaim 
  if (
  #ifdef NDEBUG
      collect == MI_FORCE
  #else
      collect >= MI_FORCE
  #endif
    && _mi_is_main_thread() && mi_heap_is_backing(heap) && !heap->no_reclaim)
  {
    // the main thread is abandoned (end-of-program), try to reclaim all abandoned segments.
    // if all memory is freed by now, all segments should be freed.
    _mi_abandoned_reclaim_all(heap, &heap->tld->segments);
  }
  
  // if abandoning, mark all pages to no longer add to delayed_free
  if (collect == MI_ABANDON) {
    mi_heap_visit_pages(heap, &mi_heap_page_never_delayed_free, NULL, NULL);
  }

  // free thread delayed blocks.
  // (if abandoning, after this there are no more thread-delayed references into the pages.)
  _mi_heap_delayed_free(heap);

  // collect retired pages
  _mi_heap_collect_retired(heap, collect >= MI_FORCE);

  // collect all pages owned by this thread
  mi_heap_visit_pages(heap, &mi_heap_page_collect, &collect, NULL);
  mi_assert_internal( collect != MI_ABANDON || mi_atomic_read_ptr(mi_block_t,&heap->thread_delayed_free) == NULL );

  // collect segment caches
  if (collect >= MI_FORCE) {
    _mi_segment_thread_collect(&heap->tld->segments);
  }

  // collect regions on program-exit (or shared library unload)
  if (collect >= MI_FORCE && _mi_is_main_thread() && mi_heap_is_backing(heap)) {
    _mi_mem_collect(&heap->tld->os);
  }
}

void _mi_heap_collect_abandon(mi_heap_t* heap) {
  mi_heap_collect_ex(heap, MI_ABANDON);
}

void mi_heap_collect(mi_heap_t* heap, bool force) mi_attr_noexcept {
  mi_heap_collect_ex(heap, (force ? MI_FORCE : MI_NORMAL));
}

void mi_collect(bool force) mi_attr_noexcept {
  mi_heap_collect(mi_get_default_heap(), force);
}


/* -----------------------------------------------------------
  Heap new
----------------------------------------------------------- */

mi_heap_t* mi_heap_get_default(void) {
  mi_thread_init();
  return mi_get_default_heap();
}

mi_heap_t* mi_heap_get_backing(void) {
  mi_heap_t* heap = mi_heap_get_default();
  mi_assert_internal(heap!=NULL);
  mi_heap_t* bheap = heap->tld->heap_backing;
  mi_assert_internal(bheap!=NULL);
  mi_assert_internal(bheap->thread_id == _mi_thread_id());
  return bheap;
}

mi_heap_t* mi_heap_new(void) {
  mi_heap_t* bheap = mi_heap_get_backing();
  mi_heap_t* heap = mi_heap_malloc_tp(bheap, mi_heap_t);  // todo: OS allocate in secure mode?
  if (heap==NULL) return NULL;
  memcpy(heap, &_mi_heap_empty, sizeof(mi_heap_t));
  heap->tld = bheap->tld;
  heap->thread_id = _mi_thread_id();
  _mi_random_split(&bheap->random, &heap->random);
  heap->cookie  = _mi_heap_random_next(heap) | 1;
  heap->keys[0] = _mi_heap_random_next(heap);
  heap->keys[1] = _mi_heap_random_next(heap);
  heap->no_reclaim = true;  // don't reclaim abandoned pages or otherwise destroy is unsafe
  // push on the thread local heaps list
  heap->next = heap->tld->heaps;
  heap->tld->heaps = heap;
  return heap;
}

uintptr_t _mi_heap_random_next(mi_heap_t* heap) {
  return _mi_random_next(&heap->random);
}

// zero out the page queues
static void mi_heap_reset_pages(mi_heap_t* heap) {
  mi_assert_internal(mi_heap_is_initialized(heap));
  // TODO: copy full empty heap instead?
  memset(&heap->pages_free_direct, 0, sizeof(heap->pages_free_direct));
#ifdef MI_MEDIUM_DIRECT
  memset(&heap->pages_free_medium, 0, sizeof(heap->pages_free_medium));
#endif
  memcpy(&heap->pages, &_mi_heap_empty.pages, sizeof(heap->pages));
  heap->thread_delayed_free = NULL;
  heap->page_count = 0;
}

// called from `mi_heap_destroy` and `mi_heap_delete` to free the internal heap resources.
static void mi_heap_free(mi_heap_t* heap) {
  mi_assert(heap != NULL);
  mi_assert_internal(mi_heap_is_initialized(heap));
  if (mi_heap_is_backing(heap)) return; // dont free the backing heap

  // reset default
  if (mi_heap_is_default(heap)) {
    _mi_heap_set_default_direct(heap->tld->heap_backing);
  }

  // remove ourselves from the thread local heaps list
  // linear search but we expect the number of heaps to be relatively small
  mi_heap_t* prev = NULL;
  mi_heap_t* curr = heap->tld->heaps; 
  while (curr != heap && curr != NULL) {
    prev = curr;
    curr = curr->next;
  }
  mi_assert_internal(curr == heap);
  if (curr == heap) {
    if (prev != NULL) { prev->next = heap->next; }
                 else { heap->tld->heaps = heap->next; }
  }
  mi_assert_internal(heap->tld->heaps != NULL);

  // and free the used memory
  mi_free(heap);
}


/* -----------------------------------------------------------
  Heap destroy
----------------------------------------------------------- */

static bool _mi_heap_page_destroy(mi_heap_t* heap, mi_page_queue_t* pq, mi_page_t* page, void* arg1, void* arg2) {
  UNUSED(arg1);
  UNUSED(arg2);
  UNUSED(heap);
  UNUSED(pq);

  // ensure no more thread_delayed_free will be added
  _mi_page_use_delayed_free(page, MI_NEVER_DELAYED_FREE, false);

  // stats
  const size_t bsize = mi_page_block_size(page);
  if (bsize > MI_LARGE_OBJ_SIZE_MAX) {
    if (bsize > MI_HUGE_OBJ_SIZE_MAX) {
      _mi_stat_decrease(&heap->tld->stats.giant, bsize);
    }
    else {
      _mi_stat_decrease(&heap->tld->stats.huge, bsize);
    }
  }
#if (MI_STAT>1)
  _mi_page_free_collect(page, false);  // update used count
  const size_t inuse = page->used;
  if (bsize <= MI_LARGE_OBJ_SIZE_MAX) {
    mi_heap_stat_decrease(heap, normal[_mi_bin(bsize)], inuse);
  }
  mi_heap_stat_decrease(heap, malloc, bsize * inuse);  // todo: off for aligned blocks...
#endif

  /// pretend it is all free now
  mi_assert_internal(mi_page_thread_free(page) == NULL);
  page->used = 0;

  // and free the page
  // mi_page_free(page,false);
  page->next = NULL;
  page->prev = NULL;
  _mi_segment_page_free(page,false /* no force? */, &heap->tld->segments);

  return true; // keep going
}

void _mi_heap_destroy_pages(mi_heap_t* heap) {
  mi_heap_visit_pages(heap, &_mi_heap_page_destroy, NULL, NULL);
  mi_heap_reset_pages(heap);
}

void mi_heap_destroy(mi_heap_t* heap) {
  mi_assert(heap != NULL);
  mi_assert(mi_heap_is_initialized(heap));
  mi_assert(heap->no_reclaim);
  mi_assert_expensive(mi_heap_is_valid(heap));
  if (!mi_heap_is_initialized(heap)) return;
  if (!heap->no_reclaim) {
    // don't free in case it may contain reclaimed pages
    mi_heap_delete(heap);
  }
  else {
    // free all pages
    _mi_heap_destroy_pages(heap);
    mi_heap_free(heap);
  }
}



/* -----------------------------------------------------------
  Safe Heap delete
----------------------------------------------------------- */

// Tranfer the pages from one heap to the other
static void mi_heap_absorb(mi_heap_t* heap, mi_heap_t* from) {
  mi_assert_internal(heap!=NULL);
  if (from==NULL || from->page_count == 0) return;

  // reduce the size of the delayed frees
  _mi_heap_delayed_free(from);
  
  // transfer all pages by appending the queues; this will set a new heap field 
  // so threads may do delayed frees in either heap for a while.
  // note: appending waits for each page to not be in the `MI_DELAYED_FREEING` state
  // so after this only the new heap will get delayed frees
  for (size_t i = 0; i <= MI_BIN_FULL; i++) {
    mi_page_queue_t* pq = &heap->pages[i];
    mi_page_queue_t* append = &from->pages[i];
    size_t pcount = _mi_page_queue_append(heap, pq, append);
    heap->page_count += pcount;
    from->page_count -= pcount;
  }
  mi_assert_internal(from->page_count == 0);

  // and do outstanding delayed frees in the `from` heap  
  // note: be careful here as the `heap` field in all those pages no longer point to `from`,
  // turns out to be ok as `_mi_heap_delayed_free` only visits the list and calls a 
  // the regular `_mi_free_delayed_block` which is safe.
  _mi_heap_delayed_free(from);  
  mi_assert_internal(from->thread_delayed_free == NULL);

  // and reset the `from` heap
  mi_heap_reset_pages(from);  
}

// Safe delete a heap without freeing any still allocated blocks in that heap.
void mi_heap_delete(mi_heap_t* heap)
{
  mi_assert(heap != NULL);
  mi_assert(mi_heap_is_initialized(heap));
  mi_assert_expensive(mi_heap_is_valid(heap));
  if (!mi_heap_is_initialized(heap)) return;

  if (!mi_heap_is_backing(heap)) {
    // tranfer still used pages to the backing heap
    mi_heap_absorb(heap->tld->heap_backing, heap);
  }
  else {
    // the backing heap abandons its pages
    _mi_heap_collect_abandon(heap);
  }
  mi_assert_internal(heap->page_count==0);
  mi_heap_free(heap);
}

mi_heap_t* mi_heap_set_default(mi_heap_t* heap) {
  mi_assert(mi_heap_is_initialized(heap));
  if (!mi_heap_is_initialized(heap)) return NULL;
  mi_assert_expensive(mi_heap_is_valid(heap));
  mi_heap_t* old = mi_get_default_heap();
  _mi_heap_set_default_direct(heap);
  return old;
}




/* -----------------------------------------------------------
  Analysis
----------------------------------------------------------- */

// static since it is not thread safe to access heaps from other threads.
static mi_heap_t* mi_heap_of_block(const void* p) {
  if (p == NULL) return NULL;
  mi_segment_t* segment = _mi_ptr_segment(p);
  bool valid = (_mi_ptr_cookie(segment) == segment->cookie);
  mi_assert_internal(valid);
  if (mi_unlikely(!valid)) return NULL;
  return mi_page_heap(_mi_segment_page_of(segment,p));
}

bool mi_heap_contains_block(mi_heap_t* heap, const void* p) {
  mi_assert(heap != NULL);
  if (!mi_heap_is_initialized(heap)) return false;
  return (heap == mi_heap_of_block(p));
}


static bool mi_heap_page_check_owned(mi_heap_t* heap, mi_page_queue_t* pq, mi_page_t* page, void* p, void* vfound) {
  UNUSED(heap);
  UNUSED(pq);
  bool* found = (bool*)vfound;
  mi_segment_t* segment = _mi_page_segment(page);
  void* start = _mi_page_start(segment, page, NULL);
  void* end   = (uint8_t*)start + (page->capacity * mi_page_block_size(page));
  *found = (p >= start && p < end);
  return (!*found); // continue if not found
}

bool mi_heap_check_owned(mi_heap_t* heap, const void* p) {
  mi_assert(heap != NULL);
  if (!mi_heap_is_initialized(heap)) return false;
  if (((uintptr_t)p & (MI_INTPTR_SIZE - 1)) != 0) return false;  // only aligned pointers
  bool found = false;
  mi_heap_visit_pages(heap, &mi_heap_page_check_owned, (void*)p, &found);
  return found;
}

bool mi_check_owned(const void* p) {
  return mi_heap_check_owned(mi_get_default_heap(), p);
}

/* -----------------------------------------------------------
  Visit all heap blocks and areas
  Todo: enable visiting abandoned pages, and
        enable visiting all blocks of all heaps across threads
----------------------------------------------------------- */

// Separate struct to keep `mi_page_t` out of the public interface
typedef struct mi_heap_area_ex_s {
  mi_heap_area_t area;
  mi_page_t*     page;
} mi_heap_area_ex_t;

static bool mi_heap_area_visit_blocks(const mi_heap_area_ex_t* xarea, mi_block_visit_fun* visitor, void* arg) {
  mi_assert(xarea != NULL);
  if (xarea==NULL) return true;
  const mi_heap_area_t* area = &xarea->area;
  mi_page_t* page = xarea->page;
  mi_assert(page != NULL);
  if (page == NULL) return true;

  _mi_page_free_collect(page,true);
  mi_assert_internal(page->local_free == NULL);
  if (page->used == 0) return true;

  const size_t bsize = mi_page_block_size(page);
  size_t   psize;
  uint8_t* pstart = _mi_page_start(_mi_page_segment(page), page, &psize);

  if (page->capacity == 1) {
    // optimize page with one block
    mi_assert_internal(page->used == 1 && page->free == NULL);
    return visitor(mi_page_heap(page), area, pstart, bsize, arg);
  }

  // create a bitmap of free blocks.
  #define MI_MAX_BLOCKS   (MI_SMALL_PAGE_SIZE / sizeof(void*))
  uintptr_t free_map[MI_MAX_BLOCKS / sizeof(uintptr_t)];
  memset(free_map, 0, sizeof(free_map));

  size_t free_count = 0;
  for (mi_block_t* block = page->free; block != NULL; block = mi_block_next(page,block)) {
    free_count++;
    mi_assert_internal((uint8_t*)block >= pstart && (uint8_t*)block < (pstart + psize));
    size_t offset = (uint8_t*)block - pstart;
    mi_assert_internal(offset % bsize == 0);
    size_t blockidx = offset / bsize;  // Todo: avoid division?
    mi_assert_internal( blockidx < MI_MAX_BLOCKS);
    size_t bitidx = (blockidx / sizeof(uintptr_t));
    size_t bit = blockidx - (bitidx * sizeof(uintptr_t));
    free_map[bitidx] |= ((uintptr_t)1 << bit);
  }
  mi_assert_internal(page->capacity == (free_count + page->used));

  // walk through all blocks skipping the free ones
  size_t used_count = 0;
  for (size_t i = 0; i < page->capacity; i++) {
    size_t bitidx = (i / sizeof(uintptr_t));
    size_t bit = i - (bitidx * sizeof(uintptr_t));
    uintptr_t m = free_map[bitidx];
    if (bit == 0 && m == UINTPTR_MAX) {
      i += (sizeof(uintptr_t) - 1); // skip a run of free blocks
    }
    else if ((m & ((uintptr_t)1 << bit)) == 0) {
      used_count++;
      uint8_t* block = pstart + (i * bsize);
      if (!visitor(mi_page_heap(page), area, block, bsize, arg)) return false;
    }
  }
  mi_assert_internal(page->used == used_count);
  return true;
}

typedef bool (mi_heap_area_visit_fun)(const mi_heap_t* heap, const mi_heap_area_ex_t* area, void* arg);


static bool mi_heap_visit_areas_page(mi_heap_t* heap, mi_page_queue_t* pq, mi_page_t* page, void* vfun, void* arg) {
  UNUSED(heap);
  UNUSED(pq);
  mi_heap_area_visit_fun* fun = (mi_heap_area_visit_fun*)vfun;
  mi_heap_area_ex_t xarea;
  const size_t bsize = mi_page_block_size(page);
  xarea.page = page;
  xarea.area.reserved = page->reserved * bsize;
  xarea.area.committed = page->capacity * bsize;
  xarea.area.blocks = _mi_page_start(_mi_page_segment(page), page, NULL);
  xarea.area.used = page->used;
  xarea.area.block_size = bsize;
  return fun(heap, &xarea, arg);
}

// Visit all heap pages as areas
static bool mi_heap_visit_areas(const mi_heap_t* heap, mi_heap_area_visit_fun* visitor, void* arg) {
  if (visitor == NULL) return false;
  return mi_heap_visit_pages((mi_heap_t*)heap, &mi_heap_visit_areas_page, (void*)(visitor), arg); // note: function pointer to void* :-{
}

// Just to pass arguments
typedef struct mi_visit_blocks_args_s {
  bool  visit_blocks;
  mi_block_visit_fun* visitor;
  void* arg;
} mi_visit_blocks_args_t;

static bool mi_heap_area_visitor(const mi_heap_t* heap, const mi_heap_area_ex_t* xarea, void* arg) {
  mi_visit_blocks_args_t* args = (mi_visit_blocks_args_t*)arg;
  if (!args->visitor(heap, &xarea->area, NULL, xarea->area.block_size, args->arg)) return false;
  if (args->visit_blocks) {
    return mi_heap_area_visit_blocks(xarea, args->visitor, args->arg);
  }
  else {
    return true;
  }
}

// Visit all blocks in a heap
bool mi_heap_visit_blocks(const mi_heap_t* heap, bool visit_blocks, mi_block_visit_fun* visitor, void* arg) {
  mi_visit_blocks_args_t args = { visit_blocks, visitor, arg };
  return mi_heap_visit_areas(heap, &mi_heap_area_visitor, &args);
}
/**** ended inlining heap.c ****/
/**** start inlining alloc.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/
/**** skipping file: mimalloc-atomic.h ****/

#include <string.h>  // memset, memcpy, strlen
#include <stdlib.h>  // malloc, exit

#define MI_IN_ALLOC_C
/**** start inlining alloc-override.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

#if !defined(MI_IN_ALLOC_C)
#error "this file should be included from 'alloc.c' (so aliases can work)"
#endif

#if defined(MI_MALLOC_OVERRIDE) && defined(_WIN32) && !(defined(MI_SHARED_LIB) && defined(_DLL))
#error "It is only possible to override "malloc" on Windows when building as a DLL (and linking the C runtime as a DLL)"
#endif

#if defined(MI_MALLOC_OVERRIDE) && !(defined(_WIN32)) // || (defined(__MACH__) && !defined(MI_INTERPOSE)))

// ------------------------------------------------------
// Override system malloc
// ------------------------------------------------------

#if (defined(__GNUC__) || defined(__clang__)) && !defined(__MACH__)
  // use aliasing to alias the exported function to one of our `mi_` functions
  #if (defined(__GNUC__) && __GNUC__ >= 9)
    #define MI_FORWARD(fun)      __attribute__((alias(#fun), used, visibility("default"), copy(fun)))
  #else
    #define MI_FORWARD(fun)      __attribute__((alias(#fun), used, visibility("default")))
  #endif
  #define MI_FORWARD1(fun,x)      MI_FORWARD(fun)
  #define MI_FORWARD2(fun,x,y)    MI_FORWARD(fun)
  #define MI_FORWARD3(fun,x,y,z)  MI_FORWARD(fun)
  #define MI_FORWARD0(fun,x)      MI_FORWARD(fun)
  #define MI_FORWARD02(fun,x,y)   MI_FORWARD(fun)
#else
  // use forwarding by calling our `mi_` function
  #define MI_FORWARD1(fun,x)      { return fun(x); }
  #define MI_FORWARD2(fun,x,y)    { return fun(x,y); }
  #define MI_FORWARD3(fun,x,y,z)  { return fun(x,y,z); }
  #define MI_FORWARD0(fun,x)      { fun(x); }
  #define MI_FORWARD02(fun,x,y)   { fun(x,y); }
#endif

#if defined(__APPLE__) && defined(MI_SHARED_LIB_EXPORT) && defined(MI_INTERPOSE)
  // use interposing so `DYLD_INSERT_LIBRARIES` works without `DYLD_FORCE_FLAT_NAMESPACE=1`
  // See: <https://books.google.com/books?id=K8vUkpOXhN4C&pg=PA73>
  struct mi_interpose_s {
    const void* replacement;
    const void* target;
  };
  #define MI_INTERPOSE_FUN(oldfun,newfun) { (const void*)&newfun, (const void*)&oldfun }
  #define MI_INTERPOSE_MI(fun)            MI_INTERPOSE_FUN(fun,mi_##fun)
  __attribute__((used)) static struct mi_interpose_s _mi_interposes[]  __attribute__((section("__DATA, __interpose"))) =
  {
    MI_INTERPOSE_MI(malloc),
    MI_INTERPOSE_MI(calloc),
    MI_INTERPOSE_MI(realloc),
    MI_INTERPOSE_MI(strdup),
    MI_INTERPOSE_MI(strndup),
    MI_INTERPOSE_MI(realpath),
    MI_INTERPOSE_MI(posix_memalign),
    MI_INTERPOSE_MI(reallocf),
    MI_INTERPOSE_MI(valloc),
    // some code allocates from a zone but deallocates using plain free :-( (like NxHashResizeToCapacity <https://github.com/nneonneo/osx-10.9-opensource/blob/master/objc4-551.1/runtime/hashtable2.mm>)
    MI_INTERPOSE_FUN(free,mi_cfree), // use safe free that checks if pointers are from us
  };
#elif defined(_MSC_VER)
  // cannot override malloc unless using a dll.
  // we just override new/delete which does work in a static library.
#else
  // On all other systems forward to our API
  void* malloc(size_t size)              MI_FORWARD1(mi_malloc, size);
  void* calloc(size_t size, size_t n)    MI_FORWARD2(mi_calloc, size, n);
  void* realloc(void* p, size_t newsize) MI_FORWARD2(mi_realloc, p, newsize);
  void  free(void* p)                    MI_FORWARD0(mi_free, p);
#endif

#if (defined(__GNUC__) || defined(__clang__)) && !defined(__MACH__)
#pragma GCC visibility push(default)
#endif

// ------------------------------------------------------
// Override new/delete
// This is not really necessary as they usually call
// malloc/free anyway, but it improves performance.
// ------------------------------------------------------
#ifdef __cplusplus
  // ------------------------------------------------------
  // With a C++ compiler we override the new/delete operators.
  // see <https://en.cppreference.com/w/cpp/memory/new/operator_new>
  // ------------------------------------------------------
  #include <new>
  void operator delete(void* p) noexcept              MI_FORWARD0(mi_free,p);
  void operator delete[](void* p) noexcept            MI_FORWARD0(mi_free,p);

  void* operator new(std::size_t n) noexcept(false)   MI_FORWARD1(mi_new,n);
  void* operator new[](std::size_t n) noexcept(false) MI_FORWARD1(mi_new,n);

  void* operator new  (std::size_t n, const std::nothrow_t& tag) noexcept { UNUSED(tag); return mi_new_nothrow(n); }
  void* operator new[](std::size_t n, const std::nothrow_t& tag) noexcept { UNUSED(tag); return mi_new_nothrow(n); }

  #if (__cplusplus >= 201402L || _MSC_VER >= 1916)
  void operator delete  (void* p, std::size_t n) noexcept MI_FORWARD02(mi_free_size,p,n);
  void operator delete[](void* p, std::size_t n) noexcept MI_FORWARD02(mi_free_size,p,n);
  #endif

  #if (__cplusplus > 201402L && defined(__cpp_aligned_new)) && (!defined(__GNUC__) || (__GNUC__ > 5))
  void operator delete  (void* p, std::align_val_t al) noexcept { mi_free_aligned(p, static_cast<size_t>(al)); }
  void operator delete[](void* p, std::align_val_t al) noexcept { mi_free_aligned(p, static_cast<size_t>(al)); }
  void operator delete  (void* p, std::size_t n, std::align_val_t al) noexcept { mi_free_size_aligned(p, n, static_cast<size_t>(al)); };
  void operator delete[](void* p, std::size_t n, std::align_val_t al) noexcept { mi_free_size_aligned(p, n, static_cast<size_t>(al)); };

  void* operator new( std::size_t n, std::align_val_t al)   noexcept(false) { return mi_new_aligned(n, static_cast<size_t>(al)); }
  void* operator new[]( std::size_t n, std::align_val_t al) noexcept(false) { return mi_new_aligned(n, static_cast<size_t>(al)); }
  void* operator new  (std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept { return mi_new_aligned_nothrow(n, static_cast<size_t>(al)); }
  void* operator new[](std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept { return mi_new_aligned_nothrow(n, static_cast<size_t>(al)); }
  #endif

#elif (defined(__GNUC__) || defined(__clang__))
  // ------------------------------------------------------
  // Override by defining the mangled C++ names of the operators (as
  // used by GCC and CLang).
  // See <https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling>
  // ------------------------------------------------------
  void _ZdlPv(void* p)            MI_FORWARD0(mi_free,p); // delete
  void _ZdaPv(void* p)            MI_FORWARD0(mi_free,p); // delete[]
  void _ZdlPvm(void* p, size_t n) MI_FORWARD02(mi_free_size,p,n);
  void _ZdaPvm(void* p, size_t n) MI_FORWARD02(mi_free_size,p,n);
  void _ZdlPvSt11align_val_t(void* p, size_t al)            { mi_free_aligned(p,al); }
  void _ZdaPvSt11align_val_t(void* p, size_t al)            { mi_free_aligned(p,al); }
  void _ZdlPvmSt11align_val_t(void* p, size_t n, size_t al) { mi_free_size_aligned(p,n,al); }
  void _ZdaPvmSt11align_val_t(void* p, size_t n, size_t al) { mi_free_size_aligned(p,n,al); }

  typedef struct mi_nothrow_s {  } mi_nothrow_t;
  #if (MI_INTPTR_SIZE==8)
    void* _Znwm(size_t n)                             MI_FORWARD1(mi_new,n);  // new 64-bit
    void* _Znam(size_t n)                             MI_FORWARD1(mi_new,n);  // new[] 64-bit
    void* _ZnwmSt11align_val_t(size_t n, size_t al)   MI_FORWARD2(mi_new_aligned, n, al);
    void* _ZnamSt11align_val_t(size_t n, size_t al)   MI_FORWARD2(mi_new_aligned, n, al);
    void* _ZnwmRKSt9nothrow_t(size_t n, mi_nothrow_t tag) { UNUSED(tag); return mi_new_nothrow(n); }
    void* _ZnamRKSt9nothrow_t(size_t n, mi_nothrow_t tag) { UNUSED(tag); return mi_new_nothrow(n); }
    void* _ZnwmSt11align_val_tRKSt9nothrow_t(size_t n, size_t al, mi_nothrow_t tag) { UNUSED(tag); return mi_new_aligned_nothrow(n,al); }
    void* _ZnamSt11align_val_tRKSt9nothrow_t(size_t n, size_t al, mi_nothrow_t tag) { UNUSED(tag); return mi_new_aligned_nothrow(n,al); }
  #elif (MI_INTPTR_SIZE==4)
    void* _Znwj(size_t n)                             MI_FORWARD1(mi_new,n);  // new 64-bit
    void* _Znaj(size_t n)                             MI_FORWARD1(mi_new,n);  // new[] 64-bit
    void* _ZnwjSt11align_val_t(size_t n, size_t al)   MI_FORWARD2(mi_new_aligned, n, al);
    void* _ZnajSt11align_val_t(size_t n, size_t al)   MI_FORWARD2(mi_new_aligned, n, al);
    void* _ZnwjRKSt9nothrow_t(size_t n, mi_nothrow_t tag) { UNUSED(tag); return mi_new_nothrow(n); }
    void* _ZnajRKSt9nothrow_t(size_t n, mi_nothrow_t tag) { UNUSED(tag); return mi_new_nothrow(n); }
    void* _ZnwjSt11align_val_tRKSt9nothrow_t(size_t n, size_t al, mi_nothrow_t tag) { UNUSED(tag); return mi_new_aligned_nothrow(n,al); }
    void* _ZnajSt11align_val_tRKSt9nothrow_t(size_t n, size_t al, mi_nothrow_t tag) { UNUSED(tag); return mi_new_aligned_nothrow(n,al); }
  #else
  #error "define overloads for new/delete for this platform (just for performance, can be skipped)"
  #endif
#endif // __cplusplus


#ifdef __cplusplus
extern "C" {
#endif

// ------------------------------------------------------
// Posix & Unix functions definitions
// ------------------------------------------------------

void   cfree(void* p)                    MI_FORWARD0(mi_free, p);
void*  reallocf(void* p, size_t newsize) MI_FORWARD2(mi_reallocf,p,newsize);
size_t malloc_size(const void* p)        MI_FORWARD1(mi_usable_size,p);
#if !defined(__ANDROID__)
size_t malloc_usable_size(void *p)       MI_FORWARD1(mi_usable_size,p);
#else
size_t malloc_usable_size(const void *p) MI_FORWARD1(mi_usable_size,p);
#endif

// no forwarding here due to aliasing/name mangling issues
void* valloc(size_t size)                                     { return mi_valloc(size); }
void* pvalloc(size_t size)                                    { return mi_pvalloc(size); }
void* reallocarray(void* p, size_t count, size_t size)        { return mi_reallocarray(p, count, size); }
void* memalign(size_t alignment, size_t size)                 { return mi_memalign(alignment, size); }
int   posix_memalign(void** p, size_t alignment, size_t size) { return mi_posix_memalign(p, alignment, size); }
void* _aligned_malloc(size_t alignment, size_t size)          { return mi_aligned_alloc(alignment, size); }

// on some glibc `aligned_alloc` is declared `static inline` so we cannot override it (e.g. Conda). This happens
// when _GLIBCXX_HAVE_ALIGNED_ALLOC is not defined. However, in those cases it will use `memalign`, `posix_memalign`, 
// or `_aligned_malloc` and we can avoid overriding it ourselves.
// We should always override if using C compilation. (issue #276)
#if _GLIBCXX_HAVE_ALIGNED_ALLOC || !defined(__cplusplus)
void* aligned_alloc(size_t alignment, size_t size)   { return mi_aligned_alloc(alignment, size); }
#endif


#if defined(__GLIBC__) && defined(__linux__)
  // forward __libc interface (needed for glibc-based Linux distributions)
  void* __libc_malloc(size_t size)                  MI_FORWARD1(mi_malloc,size);
  void* __libc_calloc(size_t count, size_t size)    MI_FORWARD2(mi_calloc,count,size);
  void* __libc_realloc(void* p, size_t size)        MI_FORWARD2(mi_realloc,p,size);
  void  __libc_free(void* p)                        MI_FORWARD0(mi_free,p);
  void  __libc_cfree(void* p)                       MI_FORWARD0(mi_free,p);

  void* __libc_valloc(size_t size)                                { return mi_valloc(size); }
  void* __libc_pvalloc(size_t size)                               { return mi_pvalloc(size); }
  void* __libc_memalign(size_t alignment, size_t size)            { return mi_memalign(alignment,size); }
  int   __posix_memalign(void** p, size_t alignment, size_t size) { return mi_posix_memalign(p,alignment,size); }
#endif

#ifdef __cplusplus
}
#endif

#if (defined(__GNUC__) || defined(__clang__)) && !defined(__MACH__)
#pragma GCC visibility pop
#endif

#endif // MI_MALLOC_OVERRIDE && !_WIN32
/**** ended inlining alloc-override.c ****/
#undef MI_IN_ALLOC_C

// ------------------------------------------------------
// Allocation
// ------------------------------------------------------

// Fast allocation in a page: just pop from the free list.
// Fall back to generic allocation only if the list is empty.
extern inline void* _mi_page_malloc(mi_heap_t* heap, mi_page_t* page, size_t size) mi_attr_noexcept {
  mi_assert_internal(page->xblock_size==0||mi_page_block_size(page) >= size);
  mi_block_t* block = page->free;
  if (mi_unlikely(block == NULL)) {
    return _mi_malloc_generic(heap, size); 
  }
  mi_assert_internal(block != NULL && _mi_ptr_page(block) == page);
  // pop from the free list
  page->free = mi_block_next(page, block);
  page->used++;
  mi_assert_internal(page->free == NULL || _mi_ptr_page(page->free) == page);
#if (MI_DEBUG>0)
  if (!page->is_zero) { memset(block, MI_DEBUG_UNINIT, size); }
#elif (MI_SECURE!=0)
  block->next = 0;  // don't leak internal data
#endif
#if (MI_STAT>1)
  const size_t bsize = mi_page_usable_block_size(page);
  if (bsize <= MI_LARGE_OBJ_SIZE_MAX) {
    const size_t bin = _mi_bin(bsize);
    mi_heap_stat_increase(heap, normal[bin], 1);
  }
#endif
#if (MI_PADDING > 0) && defined(MI_ENCODE_FREELIST)
  mi_padding_t* const padding = (mi_padding_t*)((uint8_t*)block + mi_page_usable_block_size(page));
  ptrdiff_t delta = ((uint8_t*)padding - (uint8_t*)block - (size - MI_PADDING_SIZE));
  mi_assert_internal(delta >= 0 && mi_page_usable_block_size(page) >= (size - MI_PADDING_SIZE + delta));
  padding->canary = (uint32_t)(mi_ptr_encode(page,block,page->keys));
  padding->delta  = (uint32_t)(delta);
  uint8_t* fill = (uint8_t*)padding - delta;
  const size_t maxpad = (delta > MI_MAX_ALIGN_SIZE ? MI_MAX_ALIGN_SIZE : delta); // set at most N initial padding bytes
  for (size_t i = 0; i < maxpad; i++) { fill[i] = MI_DEBUG_PADDING; }
#endif
  return block;
}

// allocate a small block
extern inline mi_decl_restrict void* mi_heap_malloc_small(mi_heap_t* heap, size_t size) mi_attr_noexcept {
  mi_assert(heap!=NULL);
  mi_assert(heap->thread_id == 0 || heap->thread_id == _mi_thread_id()); // heaps are thread local
  mi_assert(size <= MI_SMALL_SIZE_MAX);
  #if (MI_PADDING)
  if (size == 0) {
    size = sizeof(void*);
  }
  #endif
  mi_page_t* page = _mi_heap_get_free_small_page(heap,size + MI_PADDING_SIZE);
  void* p = _mi_page_malloc(heap, page, size + MI_PADDING_SIZE);
  mi_assert_internal(p==NULL || mi_usable_size(p) >= size);
  #if MI_STAT>1
  if (p != NULL) {
    if (!mi_heap_is_initialized(heap)) { heap = mi_get_default_heap(); }
    mi_heap_stat_increase(heap, malloc, mi_usable_size(p));
  }
  #endif
  return p;
}

extern inline mi_decl_restrict void* mi_malloc_small(size_t size) mi_attr_noexcept {
  return mi_heap_malloc_small(mi_get_default_heap(), size);
}

// The main allocation function
extern inline mi_decl_restrict void* mi_heap_malloc(mi_heap_t* heap, size_t size) mi_attr_noexcept {
  if (mi_likely(size <= MI_SMALL_SIZE_MAX)) {
    return mi_heap_malloc_small(heap, size);
  }
  else {
    mi_assert(heap!=NULL);
    mi_assert(heap->thread_id == 0 || heap->thread_id == _mi_thread_id()); // heaps are thread local
    void* const p = _mi_malloc_generic(heap, size + MI_PADDING_SIZE);      // note: size can overflow but it is detected in malloc_generic
    mi_assert_internal(p == NULL || mi_usable_size(p) >= size);
    #if MI_STAT>1
    if (p != NULL) {
      if (!mi_heap_is_initialized(heap)) { heap = mi_get_default_heap(); }
      mi_heap_stat_increase(heap, malloc, mi_usable_size(p));
    }
    #endif
    return p;
  }
}

extern inline mi_decl_restrict void* mi_malloc(size_t size) mi_attr_noexcept {
  return mi_heap_malloc(mi_get_default_heap(), size);
}


void _mi_block_zero_init(const mi_page_t* page, void* p, size_t size) {
  // note: we need to initialize the whole usable block size to zero, not just the requested size,
  // or the recalloc/rezalloc functions cannot safely expand in place (see issue #63)
  UNUSED(size);
  mi_assert_internal(p != NULL);
  mi_assert_internal(mi_usable_size(p) >= size); // size can be zero
  mi_assert_internal(_mi_ptr_page(p)==page);
  if (page->is_zero && size > sizeof(mi_block_t)) {
    // already zero initialized memory
    ((mi_block_t*)p)->next = 0;  // clear the free list pointer
    mi_assert_expensive(mi_mem_is_zero(p, mi_usable_size(p)));
  }
  else {
    // otherwise memset
    memset(p, 0, mi_usable_size(p));
  }
}

// zero initialized small block
mi_decl_restrict void* mi_zalloc_small(size_t size) mi_attr_noexcept {
  void* p = mi_malloc_small(size);
  if (p != NULL) {
    _mi_block_zero_init(_mi_ptr_page(p), p, size);  // todo: can we avoid getting the page again?
  }
  return p;
}

void* _mi_heap_malloc_zero(mi_heap_t* heap, size_t size, bool zero) {
  void* p = mi_heap_malloc(heap,size);
  if (zero && p != NULL) {
    _mi_block_zero_init(_mi_ptr_page(p),p,size);  // todo: can we avoid getting the page again?
  }
  return p;
}

extern inline mi_decl_restrict void* mi_heap_zalloc(mi_heap_t* heap, size_t size) mi_attr_noexcept {
  return _mi_heap_malloc_zero(heap, size, true);
}

mi_decl_restrict void* mi_zalloc(size_t size) mi_attr_noexcept {
  return mi_heap_zalloc(mi_get_default_heap(),size);
}


// ------------------------------------------------------
// Check for double free in secure and debug mode
// This is somewhat expensive so only enabled for secure mode 4
// ------------------------------------------------------

#if (MI_ENCODE_FREELIST && (MI_SECURE>=4 || MI_DEBUG!=0))
// linear check if the free list contains a specific element
static bool mi_list_contains(const mi_page_t* page, const mi_block_t* list, const mi_block_t* elem) {
  while (list != NULL) {
    if (elem==list) return true;
    list = mi_block_next(page, list);
  }
  return false;
}

static mi_decl_noinline bool mi_check_is_double_freex(const mi_page_t* page, const mi_block_t* block) {
  // The decoded value is in the same page (or NULL).
  // Walk the free lists to verify positively if it is already freed
  if (mi_list_contains(page, page->free, block) ||
      mi_list_contains(page, page->local_free, block) ||
      mi_list_contains(page, mi_page_thread_free(page), block))
  {
    _mi_error_message(EAGAIN, "double free detected of block %p with size %zu\n", block, mi_page_block_size(page));
    return true;
  }
  return false;
}

static inline bool mi_check_is_double_free(const mi_page_t* page, const mi_block_t* block) {
  mi_block_t* n = mi_block_nextx(page, block, page->keys); // pretend it is freed, and get the decoded first field
  if (((uintptr_t)n & (MI_INTPTR_SIZE-1))==0 &&  // quick check: aligned pointer?
      (n==NULL || mi_is_in_same_page(block, n))) // quick check: in same page or NULL?
  {
    // Suspicous: decoded value a in block is in the same page (or NULL) -- maybe a double free?
    // (continue in separate function to improve code generation)
    return mi_check_is_double_freex(page, block);
  }
  return false;
}
#else
static inline bool mi_check_is_double_free(const mi_page_t* page, const mi_block_t* block) {
  UNUSED(page);
  UNUSED(block);
  return false;
}
#endif

// ---------------------------------------------------------------------------
// Check for heap block overflow by setting up padding at the end of the block
// ---------------------------------------------------------------------------

#if (MI_PADDING>0) && defined(MI_ENCODE_FREELIST)
static bool mi_page_decode_padding(const mi_page_t* page, const mi_block_t* block, size_t* delta, size_t* bsize) {
  *bsize = mi_page_usable_block_size(page);
  const mi_padding_t* const padding = (mi_padding_t*)((uint8_t*)block + *bsize);
  *delta = padding->delta;
  return ((uint32_t)mi_ptr_encode(page,block,page->keys) == padding->canary && *delta <= *bsize);
}

// Return the exact usable size of a block.
static size_t mi_page_usable_size_of(const mi_page_t* page, const mi_block_t* block) {
  size_t bsize;
  size_t delta;
  bool ok = mi_page_decode_padding(page, block, &delta, &bsize);
  mi_assert_internal(ok); mi_assert_internal(delta <= bsize);
  return (ok ? bsize - delta : 0);
}

static bool mi_verify_padding(const mi_page_t* page, const mi_block_t* block, size_t* size, size_t* wrong) {
  size_t bsize;
  size_t delta;
  bool ok = mi_page_decode_padding(page, block, &delta, &bsize);
  *size = *wrong = bsize;
  if (!ok) return false;
  mi_assert_internal(bsize >= delta);
  *size = bsize - delta;
  uint8_t* fill = (uint8_t*)block + bsize - delta;
  const size_t maxpad = (delta > MI_MAX_ALIGN_SIZE ? MI_MAX_ALIGN_SIZE : delta); // check at most the first N padding bytes
  for (size_t i = 0; i < maxpad; i++) {
    if (fill[i] != MI_DEBUG_PADDING) {
      *wrong = bsize - delta + i;
      return false;
    }
  }
  return true;
}

static void mi_check_padding(const mi_page_t* page, const mi_block_t* block) {
  size_t size;
  size_t wrong;
  if (!mi_verify_padding(page,block,&size,&wrong)) {
    _mi_error_message(EFAULT, "buffer overflow in heap block %p of size %zu: write after %zu bytes\n", block, size, wrong );
  }
}

// When a non-thread-local block is freed, it becomes part of the thread delayed free
// list that is freed later by the owning heap. If the exact usable size is too small to
// contain the pointer for the delayed list, then shrink the padding (by decreasing delta)
// so it will later not trigger an overflow error in `mi_free_block`.
static void mi_padding_shrink(const mi_page_t* page, const mi_block_t* block, const size_t min_size) {
  size_t bsize;
  size_t delta;
  bool ok = mi_page_decode_padding(page, block, &delta, &bsize);
  mi_assert_internal(ok);
  if (!ok || (bsize - delta) >= min_size) return;  // usually already enough space
  mi_assert_internal(bsize >= min_size);
  if (bsize < min_size) return;  // should never happen
  size_t new_delta = (bsize - min_size);
  mi_assert_internal(new_delta < bsize);
  mi_padding_t* padding = (mi_padding_t*)((uint8_t*)block + bsize);
  padding->delta = (uint32_t)new_delta;
}
#else
static void mi_check_padding(const mi_page_t* page, const mi_block_t* block) {
  UNUSED(page);
  UNUSED(block);
}

static size_t mi_page_usable_size_of(const mi_page_t* page, const mi_block_t* block) {
  UNUSED(block);
  return mi_page_usable_block_size(page);
}

static void mi_padding_shrink(const mi_page_t* page, const mi_block_t* block, const size_t min_size) {
  UNUSED(page);
  UNUSED(block);
  UNUSED(min_size);
}
#endif

// ------------------------------------------------------
// Free
// ------------------------------------------------------

// multi-threaded free
static mi_decl_noinline void _mi_free_block_mt(mi_page_t* page, mi_block_t* block)
{
  // The padding check may access the non-thread-owned page for the key values.
  // that is safe as these are constant and the page won't be freed (as the block is not freed yet).
  mi_check_padding(page, block);
  mi_padding_shrink(page, block, sizeof(mi_block_t)); // for small size, ensure we can fit the delayed thread pointers without triggering overflow detection
  #if (MI_DEBUG!=0)
  memset(block, MI_DEBUG_FREED, mi_usable_size(block));
  #endif

  // huge page segments are always abandoned and can be freed immediately
  mi_segment_t* const segment = _mi_page_segment(page);
  if (segment->page_kind==MI_PAGE_HUGE) {
    _mi_segment_huge_page_free(segment, page, block);
    return;
  }

  // Try to put the block on either the page-local thread free list, or the heap delayed free list.
  mi_thread_free_t tfree;
  mi_thread_free_t tfreex;
  bool use_delayed;
  do {
    tfree = mi_atomic_read_relaxed(&page->xthread_free);
    use_delayed = (mi_tf_delayed(tfree) == MI_USE_DELAYED_FREE);
    if (mi_unlikely(use_delayed)) {
      // unlikely: this only happens on the first concurrent free in a page that is in the full list
      tfreex = mi_tf_set_delayed(tfree,MI_DELAYED_FREEING);
    }
    else {
      // usual: directly add to page thread_free list
      mi_block_set_next(page, block, mi_tf_block(tfree));
      tfreex = mi_tf_set_block(tfree,block);
    }
  } while (!mi_atomic_cas_weak(&page->xthread_free, tfreex, tfree));

  if (mi_unlikely(use_delayed)) {
    // racy read on `heap`, but ok because MI_DELAYED_FREEING is set (see `mi_heap_delete` and `mi_heap_collect_abandon`)
    mi_heap_t* const heap = mi_page_heap(page);
    mi_assert_internal(heap != NULL);
    if (heap != NULL) {
      // add to the delayed free list of this heap. (do this atomically as the lock only protects heap memory validity)
      mi_block_t* dfree;
      do {
        dfree = mi_atomic_read_ptr_relaxed(mi_block_t,&heap->thread_delayed_free);
        mi_block_set_nextx(heap,block,dfree, heap->keys);
      } while (!mi_atomic_cas_ptr_weak(mi_block_t,&heap->thread_delayed_free, block, dfree));
    }

    // and reset the MI_DELAYED_FREEING flag
    do {
      tfreex = tfree = mi_atomic_read_relaxed(&page->xthread_free);
      mi_assert_internal(mi_tf_delayed(tfree) == MI_DELAYED_FREEING);
      tfreex = mi_tf_set_delayed(tfree,MI_NO_DELAYED_FREE);
    } while (!mi_atomic_cas_weak(&page->xthread_free, tfreex, tfree));
  }
}


// regular free
static inline void _mi_free_block(mi_page_t* page, bool local, mi_block_t* block)
{
  // and push it on the free list
  if (mi_likely(local)) {
    // owning thread can free a block directly
    if (mi_unlikely(mi_check_is_double_free(page, block))) return;
    mi_check_padding(page, block);
    #if (MI_DEBUG!=0)
    memset(block, MI_DEBUG_FREED, mi_page_block_size(page));
    #endif
    mi_block_set_next(page, block, page->local_free);
    page->local_free = block;
    page->used--;
    if (mi_unlikely(mi_page_all_free(page))) {
      _mi_page_retire(page);
    }
    else if (mi_unlikely(mi_page_is_in_full(page))) {
      _mi_page_unfull(page);
    }
  }
  else {
    _mi_free_block_mt(page,block);
  }
}


// Adjust a block that was allocated aligned, to the actual start of the block in the page.
mi_block_t* _mi_page_ptr_unalign(const mi_segment_t* segment, const mi_page_t* page, const void* p) {
  mi_assert_internal(page!=NULL && p!=NULL);
  const size_t diff   = (uint8_t*)p - _mi_page_start(segment, page, NULL);
  const size_t adjust = (diff % mi_page_block_size(page));
  return (mi_block_t*)((uintptr_t)p - adjust);
}


static void mi_decl_noinline mi_free_generic(const mi_segment_t* segment, bool local, void* p) {
  mi_page_t* const page = _mi_segment_page_of(segment, p);
  mi_block_t* const block = (mi_page_has_aligned(page) ? _mi_page_ptr_unalign(segment, page, p) : (mi_block_t*)p);
  _mi_free_block(page, local, block);
}

// Get the segment data belonging to a pointer
// This is just a single `and` in assembly but does further checks in debug mode
// (and secure mode) if this was a valid pointer.
static inline mi_segment_t* mi_checked_ptr_segment(const void* p, const char* msg) 
{
  UNUSED(msg);
#if (MI_DEBUG>0)
  if (mi_unlikely(((uintptr_t)p & (MI_INTPTR_SIZE - 1)) != 0)) {
    _mi_error_message(EINVAL, "%s: invalid (unaligned) pointer: %p\n", msg, p);
    return NULL;
  }
#endif

  mi_segment_t* const segment = _mi_ptr_segment(p);

#if (MI_DEBUG>0 || MI_SECURE>=4)
  if (mi_unlikely(segment == NULL)) return NULL;  // checks also for (p==NULL)
#endif

#if (MI_DEBUG>0)
  if (mi_unlikely(!mi_is_in_heap_region(p))) {
    _mi_warning_message("%s: pointer might not point to a valid heap region: %p\n"
      "(this may still be a valid very large allocation (over 64MiB))\n", msg, p);
    if (mi_likely(_mi_ptr_cookie(segment) == segment->cookie)) {
      _mi_warning_message("(yes, the previous pointer %p was valid after all)\n", p);
    }
  }
#endif
#if (MI_DEBUG>0 || MI_SECURE>=4)
  if (mi_unlikely(_mi_ptr_cookie(segment) != segment->cookie)) {
    _mi_error_message(EINVAL, "%s: pointer does not point to a valid heap space: %p\n", p);
  }
#endif
  return segment;
}


// Free a block
void mi_free(void* p) mi_attr_noexcept
{
  const mi_segment_t* const segment = mi_checked_ptr_segment(p,"mi_free");
  if (mi_unlikely(segment == NULL)) return; 

  const uintptr_t tid = _mi_thread_id();
  mi_page_t* const page = _mi_segment_page_of(segment, p);
  mi_block_t* const block = (mi_block_t*)p;

#if (MI_STAT>1)
  mi_heap_t* const heap = mi_heap_get_default();
  const size_t bsize = mi_page_usable_block_size(page);
  mi_heap_stat_decrease(heap, malloc, bsize);
  if (bsize <= MI_LARGE_OBJ_SIZE_MAX) { // huge page stats are accounted for in `_mi_page_retire`
    mi_heap_stat_decrease(heap, normal[_mi_bin(bsize)], 1);
  }
#endif

  if (mi_likely(tid == segment->thread_id && page->flags.full_aligned == 0)) {  // the thread id matches and it is not a full page, nor has aligned blocks
    // local, and not full or aligned
    if (mi_unlikely(mi_check_is_double_free(page,block))) return;
    mi_check_padding(page, block);
    #if (MI_DEBUG!=0)
    memset(block, MI_DEBUG_FREED, mi_page_block_size(page));
    #endif
    mi_block_set_next(page, block, page->local_free);
    page->local_free = block;
    page->used--;
    if (mi_unlikely(mi_page_all_free(page))) {
      _mi_page_retire(page);
    }
  }
  else {
    // non-local, aligned blocks, or a full page; use the more generic path
    // note: recalc page in generic to improve code generation
    mi_free_generic(segment, tid == segment->thread_id, p);
  }
}

bool _mi_free_delayed_block(mi_block_t* block) {
  // get segment and page
  const mi_segment_t* const segment = _mi_ptr_segment(block);
  mi_assert_internal(_mi_ptr_cookie(segment) == segment->cookie);
  mi_assert_internal(_mi_thread_id() == segment->thread_id);
  mi_page_t* const page = _mi_segment_page_of(segment, block);

  // Clear the no-delayed flag so delayed freeing is used again for this page.
  // This must be done before collecting the free lists on this page -- otherwise
  // some blocks may end up in the page `thread_free` list with no blocks in the
  // heap `thread_delayed_free` list which may cause the page to be never freed!
  // (it would only be freed if we happen to scan it in `mi_page_queue_find_free_ex`)
  _mi_page_use_delayed_free(page, MI_USE_DELAYED_FREE, false /* dont overwrite never delayed */);

  // collect all other non-local frees to ensure up-to-date `used` count
  _mi_page_free_collect(page, false);

  // and free the block (possibly freeing the page as well since used is updated)
  _mi_free_block(page, true, block);
  return true;
}

// Bytes available in a block
static size_t _mi_usable_size(const void* p, const char* msg) mi_attr_noexcept {
  const mi_segment_t* const segment = mi_checked_ptr_segment(p,msg);
  if (segment==NULL) return 0;
  const mi_page_t* const page = _mi_segment_page_of(segment, p);
  const mi_block_t* block = (const mi_block_t*)p;
  if (mi_unlikely(mi_page_has_aligned(page))) {
    block = _mi_page_ptr_unalign(segment, page, p);
    size_t size = mi_page_usable_size_of(page, block);
    ptrdiff_t const adjust = (uint8_t*)p - (uint8_t*)block;
    mi_assert_internal(adjust >= 0 && (size_t)adjust <= size);
    return (size - adjust);
  }
  else {
    return mi_page_usable_size_of(page, block);
  }
}

size_t mi_usable_size(const void* p) mi_attr_noexcept {
  return _mi_usable_size(p, "mi_usable_size");
}


// ------------------------------------------------------
// ensure explicit external inline definitions are emitted!
// ------------------------------------------------------

#ifdef __cplusplus
void* _mi_externs[] = {
  (void*)&_mi_page_malloc,
  (void*)&mi_malloc,
  (void*)&mi_malloc_small,
  (void*)&mi_heap_malloc,
  (void*)&mi_heap_zalloc,
  (void*)&mi_heap_malloc_small
};
#endif


// ------------------------------------------------------
// Allocation extensions
// ------------------------------------------------------

void mi_free_size(void* p, size_t size) mi_attr_noexcept {
  UNUSED_RELEASE(size);
  mi_assert(p == NULL || size <= _mi_usable_size(p,"mi_free_size"));
  mi_free(p);
}

void mi_free_size_aligned(void* p, size_t size, size_t alignment) mi_attr_noexcept {
  UNUSED_RELEASE(alignment);
  mi_assert(((uintptr_t)p % alignment) == 0);
  mi_free_size(p,size);
}

void mi_free_aligned(void* p, size_t alignment) mi_attr_noexcept {
  UNUSED_RELEASE(alignment);
  mi_assert(((uintptr_t)p % alignment) == 0);
  mi_free(p);
}

extern inline mi_decl_restrict void* mi_heap_calloc(mi_heap_t* heap, size_t count, size_t size) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(count,size,&total)) return NULL;
  return mi_heap_zalloc(heap,total);
}

mi_decl_restrict void* mi_calloc(size_t count, size_t size) mi_attr_noexcept {
  return mi_heap_calloc(mi_get_default_heap(),count,size);
}

// Uninitialized `calloc`
extern mi_decl_restrict void* mi_heap_mallocn(mi_heap_t* heap, size_t count, size_t size) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(count, size, &total)) return NULL;
  return mi_heap_malloc(heap, total);
}

mi_decl_restrict void* mi_mallocn(size_t count, size_t size) mi_attr_noexcept {
  return mi_heap_mallocn(mi_get_default_heap(),count,size);
}

// Expand in place or fail
void* mi_expand(void* p, size_t newsize) mi_attr_noexcept {
  if (p == NULL) return NULL;
  size_t size = _mi_usable_size(p,"mi_expand");
  if (newsize > size) return NULL;
  return p; // it fits
}

void* _mi_heap_realloc_zero(mi_heap_t* heap, void* p, size_t newsize, bool zero) {
  if (p == NULL) return _mi_heap_malloc_zero(heap,newsize,zero);
  size_t size = _mi_usable_size(p,"mi_realloc");
  if (newsize <= size && newsize >= (size / 2)) {
    return p;  // reallocation still fits and not more than 50% waste
  }
  void* newp = mi_heap_malloc(heap,newsize);
  if (mi_likely(newp != NULL)) {
    if (zero && newsize > size) {
      // also set last word in the previous allocation to zero to ensure any padding is zero-initialized
      size_t start = (size >= sizeof(intptr_t) ? size - sizeof(intptr_t) : 0);
      memset((uint8_t*)newp + start, 0, newsize - start);
    }
    memcpy(newp, p, (newsize > size ? size : newsize));
    mi_free(p); // only free if successful
  }
  return newp;
}

void* mi_heap_realloc(mi_heap_t* heap, void* p, size_t newsize) mi_attr_noexcept {
  return _mi_heap_realloc_zero(heap, p, newsize, false);
}

void* mi_heap_reallocn(mi_heap_t* heap, void* p, size_t count, size_t size) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(count, size, &total)) return NULL;
  return mi_heap_realloc(heap, p, total);
}


// Reallocate but free `p` on errors
void* mi_heap_reallocf(mi_heap_t* heap, void* p, size_t newsize) mi_attr_noexcept {
  void* newp = mi_heap_realloc(heap, p, newsize);
  if (newp==NULL && p!=NULL) mi_free(p);
  return newp;
}

void* mi_heap_rezalloc(mi_heap_t* heap, void* p, size_t newsize) mi_attr_noexcept {
  return _mi_heap_realloc_zero(heap, p, newsize, true);
}

void* mi_heap_recalloc(mi_heap_t* heap, void* p, size_t count, size_t size) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(count, size, &total)) return NULL;
  return mi_heap_rezalloc(heap, p, total);
}


void* mi_realloc(void* p, size_t newsize) mi_attr_noexcept {
  return mi_heap_realloc(mi_get_default_heap(),p,newsize);
}

void* mi_reallocn(void* p, size_t count, size_t size) mi_attr_noexcept {
  return mi_heap_reallocn(mi_get_default_heap(),p,count,size);
}

// Reallocate but free `p` on errors
void* mi_reallocf(void* p, size_t newsize) mi_attr_noexcept {
  return mi_heap_reallocf(mi_get_default_heap(),p,newsize);
}

void* mi_rezalloc(void* p, size_t newsize) mi_attr_noexcept {
  return mi_heap_rezalloc(mi_get_default_heap(), p, newsize);
}

void* mi_recalloc(void* p, size_t count, size_t size) mi_attr_noexcept {
  return mi_heap_recalloc(mi_get_default_heap(), p, count, size);
}



// ------------------------------------------------------
// strdup, strndup, and realpath
// ------------------------------------------------------

// `strdup` using mi_malloc
mi_decl_restrict char* mi_heap_strdup(mi_heap_t* heap, const char* s) mi_attr_noexcept {
  if (s == NULL) return NULL;
  size_t n = strlen(s);
  char* t = (char*)mi_heap_malloc(heap,n+1);
  if (t != NULL) memcpy(t, s, n + 1);
  return t;
}

mi_decl_restrict char* mi_strdup(const char* s) mi_attr_noexcept {
  return mi_heap_strdup(mi_get_default_heap(), s);
}

// `strndup` using mi_malloc
mi_decl_restrict char* mi_heap_strndup(mi_heap_t* heap, const char* s, size_t n) mi_attr_noexcept {
  if (s == NULL) return NULL;
  const char* end = (const char*)memchr(s, 0, n);  // find end of string in the first `n` characters (returns NULL if not found)
  const size_t m = (end != NULL ? (size_t)(end - s) : n);  // `m` is the minimum of `n` or the end-of-string
  mi_assert_internal(m <= n);
  char* t = (char*)mi_heap_malloc(heap, m+1);
  if (t == NULL) return NULL;
  memcpy(t, s, m);
  t[m] = 0;
  return t;
}

mi_decl_restrict char* mi_strndup(const char* s, size_t n) mi_attr_noexcept {
  return mi_heap_strndup(mi_get_default_heap(),s,n);
}

#if !defined(__wasi__) && !defined(__EMSCRIPTEN__)
// `realpath` using mi_malloc
#ifdef _WIN32
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#include <windows.h>
mi_decl_restrict char* mi_heap_realpath(mi_heap_t* heap, const char* fname, char* resolved_name) mi_attr_noexcept {
  // todo: use GetFullPathNameW to allow longer file names
  char buf[PATH_MAX];
  DWORD res = GetFullPathNameA(fname, PATH_MAX, (resolved_name == NULL ? buf : resolved_name), NULL);
  if (res == 0) {
    errno = GetLastError(); return NULL;
  }
  else if (res > PATH_MAX) {
    errno = EINVAL; return NULL;
  }
  else if (resolved_name != NULL) {
    return resolved_name;
  }
  else {
    return mi_heap_strndup(heap, buf, PATH_MAX);
  }
}
#else
#include <unistd.h>  // pathconf
static size_t mi_path_max() {
  static size_t path_max = 0;
  if (path_max <= 0) {
    long m = pathconf("/",_PC_PATH_MAX);
    if (m <= 0) path_max = 4096;      // guess
    else if (m < 256) path_max = 256; // at least 256
    else path_max = m;
  }
  return path_max;
}

char* mi_heap_realpath(mi_heap_t* heap, const char* fname, char* resolved_name) mi_attr_noexcept {
  if (resolved_name != NULL) {
    return realpath(fname,resolved_name);
  }
  else {
    size_t n  = mi_path_max();
    char* buf = (char*)mi_malloc(n+1);
    if (buf==NULL) return NULL;
    char* rname  = realpath(fname,buf);
    char* result = mi_heap_strndup(heap,rname,n); // ok if `rname==NULL`
    mi_free(buf);
    return result;
  }
}
#endif

mi_decl_restrict char* mi_realpath(const char* fname, char* resolved_name) mi_attr_noexcept {
  return mi_heap_realpath(mi_get_default_heap(),fname,resolved_name);
}
#endif

/*-------------------------------------------------------
C++ new and new_aligned
The standard requires calling into `get_new_handler` and
throwing the bad_alloc exception on failure. If we compile
with a C++ compiler we can implement this precisely. If we
use a C compiler we cannot throw a `bad_alloc` exception
but we call `exit` instead (i.e. not returning).
-------------------------------------------------------*/

#ifdef __cplusplus
#include <new>
static bool mi_try_new_handler(bool nothrow) {
  std::new_handler h = std::get_new_handler();
  if (h==NULL) {
    if (!nothrow) throw std::bad_alloc();
    return false;
  }
  else {
    h();
    return true;
  }
}
#else
typedef void (*std_new_handler_t)();

#if (defined(__GNUC__) || defined(__clang__))
std_new_handler_t __attribute((weak)) _ZSt15get_new_handlerv() {
  return NULL;
}
std_new_handler_t mi_get_new_handler() {
  return _ZSt15get_new_handlerv();
}
#else
// note: on windows we could dynamically link to `?get_new_handler@std@@YAP6AXXZXZ`.
std_new_handler_t mi_get_new_handler() {
  return NULL;
}
#endif

static bool mi_try_new_handler(bool nothrow) {
  std_new_handler_t h = mi_get_new_handler();
  if (h==NULL) {
    if (!nothrow) exit(ENOMEM);  // cannot throw in plain C, use exit as we are out of memory anyway.
    return false;
  }
  else {
    h();
    return true;
  }
}
#endif

static mi_decl_noinline void* mi_try_new(size_t size, bool nothrow ) {
  void* p = NULL;
  while(p == NULL && mi_try_new_handler(nothrow)) {
    p = mi_malloc(size);
  }
  return p;
}

mi_decl_restrict void* mi_new(size_t size) {
  void* p = mi_malloc(size);
  if (mi_unlikely(p == NULL)) return mi_try_new(size,false);
  return p;
}

mi_decl_restrict void* mi_new_nothrow(size_t size) mi_attr_noexcept {
  void* p = mi_malloc(size);
  if (mi_unlikely(p == NULL)) return mi_try_new(size, true);
  return p;
}

mi_decl_restrict void* mi_new_aligned(size_t size, size_t alignment) {
  void* p;
  do {
    p = mi_malloc_aligned(size, alignment);
  }
  while(p == NULL && mi_try_new_handler(false));
  return p;
}

mi_decl_restrict void* mi_new_aligned_nothrow(size_t size, size_t alignment) mi_attr_noexcept {
  void* p;
  do {
    p = mi_malloc_aligned(size, alignment);
  }
  while(p == NULL && mi_try_new_handler(true));
  return p;
}

mi_decl_restrict void* mi_new_n(size_t count, size_t size) {
  size_t total;
  if (mi_unlikely(mi_count_size_overflow(count, size, &total))) {
    mi_try_new_handler(false);  // on overflow we invoke the try_new_handler once to potentially throw std::bad_alloc
    return NULL;
  }
  else {
    return mi_new(total);
  }
}

void* mi_new_realloc(void* p, size_t newsize) {
  void* q;
  do {
    q = mi_realloc(p, newsize);
  } while (q == NULL && mi_try_new_handler(false));
  return q;
}

void* mi_new_reallocn(void* p, size_t newcount, size_t size) {
  size_t total;
  if (mi_unlikely(mi_count_size_overflow(newcount, size, &total))) {
    mi_try_new_handler(false);  // on overflow we invoke the try_new_handler once to potentially throw std::bad_alloc
    return NULL;
  }
  else {
    return mi_new_realloc(p, total);
  }
}
/**** ended inlining alloc.c ****/
/**** start inlining alloc-aligned.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/

#include <string.h>  // memset, memcpy

// ------------------------------------------------------
// Aligned Allocation
// ------------------------------------------------------

static void* mi_heap_malloc_zero_aligned_at(mi_heap_t* const heap, const size_t size, const size_t alignment, const size_t offset, const bool zero) mi_attr_noexcept {
  // note: we don't require `size > offset`, we just guarantee that
  // the address at offset is aligned regardless of the allocated size.
  mi_assert(alignment > 0);
  if (mi_unlikely(size > PTRDIFF_MAX)) return NULL;   // we don't allocate more than PTRDIFF_MAX (see <https://sourceware.org/ml/libc-announce/2019/msg00001.html>)
  if (mi_unlikely(alignment==0 || !_mi_is_power_of_two(alignment))) return NULL; // require power-of-two (see <https://en.cppreference.com/w/c/memory/aligned_alloc>)
  const uintptr_t align_mask = alignment-1;  // for any x, `(x & align_mask) == (x % alignment)`
  
  // try if there is a small block available with just the right alignment
  const size_t padsize = size + MI_PADDING_SIZE;
  if (mi_likely(padsize <= MI_SMALL_SIZE_MAX)) {
    mi_page_t* page = _mi_heap_get_free_small_page(heap,padsize);
    const bool is_aligned = (((uintptr_t)page->free+offset) & align_mask)==0;
    if (mi_likely(page->free != NULL && is_aligned))
    {
      #if MI_STAT>1
      mi_heap_stat_increase( heap, malloc, size);
      #endif
      void* p = _mi_page_malloc(heap,page,padsize); // TODO: inline _mi_page_malloc
      mi_assert_internal(p != NULL);
      mi_assert_internal(((uintptr_t)p + offset) % alignment == 0);
      if (zero) _mi_block_zero_init(page,p,size);
      return p;
    }
  }

  // use regular allocation if it is guaranteed to fit the alignment constraints
  if (offset==0 && alignment<=padsize && padsize<=MI_MEDIUM_OBJ_SIZE_MAX && (padsize&align_mask)==0) {
    void* p = _mi_heap_malloc_zero(heap, size, zero);
    mi_assert_internal(p == NULL || ((uintptr_t)p % alignment) == 0);
    return p;
  }
  
  // otherwise over-allocate
  void* p = _mi_heap_malloc_zero(heap, size + alignment - 1, zero);
  if (p == NULL) return NULL;

  // .. and align within the allocation
  uintptr_t adjust = alignment - (((uintptr_t)p + offset) & align_mask);
  mi_assert_internal(adjust <= alignment);
  void* aligned_p = (adjust == alignment ? p : (void*)((uintptr_t)p + adjust));
  if (aligned_p != p) mi_page_set_has_aligned(_mi_ptr_page(p), true); 
  mi_assert_internal(((uintptr_t)aligned_p + offset) % alignment == 0);
  mi_assert_internal( p == _mi_page_ptr_unalign(_mi_ptr_segment(aligned_p),_mi_ptr_page(aligned_p),aligned_p) );
  return aligned_p;
}


mi_decl_restrict void* mi_heap_malloc_aligned_at(mi_heap_t* heap, size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_malloc_zero_aligned_at(heap, size, alignment, offset, false);
}

mi_decl_restrict void* mi_heap_malloc_aligned(mi_heap_t* heap, size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_malloc_aligned_at(heap, size, alignment, 0);
}

mi_decl_restrict void* mi_heap_zalloc_aligned_at(mi_heap_t* heap, size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_malloc_zero_aligned_at(heap, size, alignment, offset, true);
}

mi_decl_restrict void* mi_heap_zalloc_aligned(mi_heap_t* heap, size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_zalloc_aligned_at(heap, size, alignment, 0);
}

mi_decl_restrict void* mi_heap_calloc_aligned_at(mi_heap_t* heap, size_t count, size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(count, size, &total)) return NULL;
  return mi_heap_zalloc_aligned_at(heap, total, alignment, offset);
}

mi_decl_restrict void* mi_heap_calloc_aligned(mi_heap_t* heap, size_t count, size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_calloc_aligned_at(heap,count,size,alignment,0);
}

mi_decl_restrict void* mi_malloc_aligned_at(size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_malloc_aligned_at(mi_get_default_heap(), size, alignment, offset);
}

mi_decl_restrict void* mi_malloc_aligned(size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_malloc_aligned(mi_get_default_heap(), size, alignment);
}

mi_decl_restrict void* mi_zalloc_aligned_at(size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_zalloc_aligned_at(mi_get_default_heap(), size, alignment, offset);
}

mi_decl_restrict void* mi_zalloc_aligned(size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_zalloc_aligned(mi_get_default_heap(), size, alignment);
}

mi_decl_restrict void* mi_calloc_aligned_at(size_t count, size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_calloc_aligned_at(mi_get_default_heap(), count, size, alignment, offset);
}

mi_decl_restrict void* mi_calloc_aligned(size_t count, size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_calloc_aligned(mi_get_default_heap(), count, size, alignment);
}


static void* mi_heap_realloc_zero_aligned_at(mi_heap_t* heap, void* p, size_t newsize, size_t alignment, size_t offset, bool zero) mi_attr_noexcept {
  mi_assert(alignment > 0);
  if (alignment <= sizeof(uintptr_t)) return _mi_heap_realloc_zero(heap,p,newsize,zero);
  if (p == NULL) return mi_heap_malloc_zero_aligned_at(heap,newsize,alignment,offset,zero);
  size_t size = mi_usable_size(p);
  if (newsize <= size && newsize >= (size - (size / 2))
      && (((uintptr_t)p + offset) % alignment) == 0) {
    return p;  // reallocation still fits, is aligned and not more than 50% waste
  }
  else {
    void* newp = mi_heap_malloc_aligned_at(heap,newsize,alignment,offset);
    if (newp != NULL) {
      if (zero && newsize > size) {
        const mi_page_t* page = _mi_ptr_page(newp);
        if (page->is_zero) {
          // already zero initialized
          mi_assert_expensive(mi_mem_is_zero(newp,newsize));
        }
        else {
          // also set last word in the previous allocation to zero to ensure any padding is zero-initialized
          size_t start = (size >= sizeof(intptr_t) ? size - sizeof(intptr_t) : 0);
          memset((uint8_t*)newp + start, 0, newsize - start);
        }
      }
      memcpy(newp, p, (newsize > size ? size : newsize));
      mi_free(p); // only free if successful
    }
    return newp;
  }
}

static void* mi_heap_realloc_zero_aligned(mi_heap_t* heap, void* p, size_t newsize, size_t alignment, bool zero) mi_attr_noexcept {
  mi_assert(alignment > 0);
  if (alignment <= sizeof(uintptr_t)) return _mi_heap_realloc_zero(heap,p,newsize,zero);
  size_t offset = ((uintptr_t)p % alignment); // use offset of previous allocation (p can be NULL)
  return mi_heap_realloc_zero_aligned_at(heap,p,newsize,alignment,offset,zero);
}

void* mi_heap_realloc_aligned_at(mi_heap_t* heap, void* p, size_t newsize, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_realloc_zero_aligned_at(heap,p,newsize,alignment,offset,false);
}

void* mi_heap_realloc_aligned(mi_heap_t* heap, void* p, size_t newsize, size_t alignment) mi_attr_noexcept {
  return mi_heap_realloc_zero_aligned(heap,p,newsize,alignment,false);
}

void* mi_heap_rezalloc_aligned_at(mi_heap_t* heap, void* p, size_t newsize, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_realloc_zero_aligned_at(heap, p, newsize, alignment, offset, true);
}

void* mi_heap_rezalloc_aligned(mi_heap_t* heap, void* p, size_t newsize, size_t alignment) mi_attr_noexcept {
  return mi_heap_realloc_zero_aligned(heap, p, newsize, alignment, true);
}

void* mi_heap_recalloc_aligned_at(mi_heap_t* heap, void* p, size_t newcount, size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(newcount, size, &total)) return NULL;
  return mi_heap_rezalloc_aligned_at(heap, p, total, alignment, offset);
}

void* mi_heap_recalloc_aligned(mi_heap_t* heap, void* p, size_t newcount, size_t size, size_t alignment) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(newcount, size, &total)) return NULL;
  return mi_heap_rezalloc_aligned(heap, p, total, alignment);
}

void* mi_realloc_aligned_at(void* p, size_t newsize, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_realloc_aligned_at(mi_get_default_heap(), p, newsize, alignment, offset);
}

void* mi_realloc_aligned(void* p, size_t newsize, size_t alignment) mi_attr_noexcept {
  return mi_heap_realloc_aligned(mi_get_default_heap(), p, newsize, alignment);
}

void* mi_rezalloc_aligned_at(void* p, size_t newsize, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_rezalloc_aligned_at(mi_get_default_heap(), p, newsize, alignment, offset);
}

void* mi_rezalloc_aligned(void* p, size_t newsize, size_t alignment) mi_attr_noexcept {
  return mi_heap_rezalloc_aligned(mi_get_default_heap(), p, newsize, alignment);
}

void* mi_recalloc_aligned_at(void* p, size_t newcount, size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_recalloc_aligned_at(mi_get_default_heap(), p, newcount, size, alignment, offset);
}

void* mi_recalloc_aligned(void* p, size_t newcount, size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_recalloc_aligned(mi_get_default_heap(), p, newcount, size, alignment);
}

/**** ended inlining alloc-aligned.c ****/
/**** start inlining alloc-posix.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018,2019, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

// ------------------------------------------------------------------------
// mi prefixed publi definitions of various Posix, Unix, and C++ functions
// for convenience and used when overriding these functions.
// ------------------------------------------------------------------------
/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/

// ------------------------------------------------------
// Posix & Unix functions definitions
// ------------------------------------------------------

#include <errno.h>
#include <string.h>  // memcpy
#include <stdlib.h>  // getenv

#ifndef EINVAL
#define EINVAL 22
#endif
#ifndef ENOMEM
#define ENOMEM 12
#endif


size_t mi_malloc_size(const void* p) mi_attr_noexcept {
  return mi_usable_size(p);
}

size_t mi_malloc_usable_size(const void *p) mi_attr_noexcept {
  return mi_usable_size(p);
}

void mi_cfree(void* p) mi_attr_noexcept {
  if (mi_is_in_heap_region(p)) {
    mi_free(p);
  }
}

int mi_posix_memalign(void** p, size_t alignment, size_t size) mi_attr_noexcept {
  // Note: The spec dictates we should not modify `*p` on an error. (issue#27)
  // <http://man7.org/linux/man-pages/man3/posix_memalign.3.html>
  if (p == NULL) return EINVAL;
  if (alignment % sizeof(void*) != 0) return EINVAL;   // natural alignment
  if (!_mi_is_power_of_two(alignment)) return EINVAL;  // not a power of 2
  void* q = (mi_malloc_satisfies_alignment(alignment, size) ? mi_malloc(size) : mi_malloc_aligned(size, alignment));
  if (q==NULL && size != 0) return ENOMEM;
  mi_assert_internal(((uintptr_t)q % alignment) == 0);
  *p = q;
  return 0;
}

mi_decl_restrict void* mi_memalign(size_t alignment, size_t size) mi_attr_noexcept {
  void* p = (mi_malloc_satisfies_alignment(alignment,size) ? mi_malloc(size) : mi_malloc_aligned(size, alignment));
  mi_assert_internal(((uintptr_t)p % alignment) == 0);
  return p;
}

mi_decl_restrict void* mi_valloc(size_t size) mi_attr_noexcept {
  return mi_memalign( _mi_os_page_size(), size );
}

mi_decl_restrict void* mi_pvalloc(size_t size) mi_attr_noexcept {
  size_t psize = _mi_os_page_size();
  if (size >= SIZE_MAX - psize) return NULL; // overflow
  size_t asize = _mi_align_up(size, psize);
  return mi_malloc_aligned(asize, psize);
}

mi_decl_restrict void* mi_aligned_alloc(size_t alignment, size_t size) mi_attr_noexcept {
  if (alignment==0 || !_mi_is_power_of_two(alignment)) return NULL; 
  if ((size&(alignment-1)) != 0) return NULL; // C11 requires integral multiple, see <https://en.cppreference.com/w/c/memory/aligned_alloc>
  void* p = (mi_malloc_satisfies_alignment(alignment, size) ? mi_malloc(size) : mi_malloc_aligned(size, alignment));
  mi_assert_internal(((uintptr_t)p % alignment) == 0);
  return p;
}

void* mi_reallocarray( void* p, size_t count, size_t size ) mi_attr_noexcept {  // BSD
  void* newp = mi_reallocn(p,count,size);
  if (newp==NULL) errno = ENOMEM;
  return newp;
}

void* mi__expand(void* p, size_t newsize) mi_attr_noexcept {  // Microsoft
  void* res = mi_expand(p, newsize);
  if (res == NULL) errno = ENOMEM;
  return res;
}

mi_decl_restrict unsigned short* mi_wcsdup(const unsigned short* s) mi_attr_noexcept {
  if (s==NULL) return NULL;
  size_t len;
  for(len = 0; s[len] != 0; len++) { }
  size_t size = (len+1)*sizeof(unsigned short);
  unsigned short* p = (unsigned short*)mi_malloc(size);
  if (p != NULL) {
    memcpy(p,s,size);
  }
  return p;
}

mi_decl_restrict unsigned char* mi_mbsdup(const unsigned char* s)  mi_attr_noexcept {
  return (unsigned char*)mi_strdup((const char*)s);
}

int mi_dupenv_s(char** buf, size_t* size, const char* name) mi_attr_noexcept {
  if (buf==NULL || name==NULL) return EINVAL;
  if (size != NULL) *size = 0;
  #pragma warning(suppress:4996)
  char* p = getenv(name);
  if (p==NULL) {
    *buf = NULL;
  }
  else {
    *buf = mi_strdup(p);
    if (*buf==NULL) return ENOMEM;
    if (size != NULL) *size = strlen(p);
  }
  return 0;
}

int mi_wdupenv_s(unsigned short** buf, size_t* size, const unsigned short* name) mi_attr_noexcept {
  if (buf==NULL || name==NULL) return EINVAL;
  if (size != NULL) *size = 0;
#if !defined(_WIN32) || (defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP))
  // not supported
  *buf = NULL;
  return EINVAL;
#else
  #pragma warning(suppress:4996)
  unsigned short* p = (unsigned short*)_wgetenv((const wchar_t*)name);
  if (p==NULL) {
    *buf = NULL;
  }
  else {
    *buf = mi_wcsdup(p);
    if (*buf==NULL) return ENOMEM;
    if (size != NULL) *size = wcslen((const wchar_t*)p);
  }
  return 0;
#endif
}

void* mi_aligned_offset_recalloc(void* p, size_t newcount, size_t size, size_t alignment, size_t offset) mi_attr_noexcept { // Microsoft
  return mi_recalloc_aligned_at(p, newcount, size, alignment, offset);
}

void* mi_aligned_recalloc(void* p, size_t newcount, size_t size, size_t alignment) mi_attr_noexcept { // Microsoft
  return mi_recalloc_aligned(p, newcount, size, alignment);
}
/**** ended inlining alloc-posix.c ****/
#if MI_OSX_ZONE
/**** start inlining alloc-override-osx.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/

#if defined(MI_MALLOC_OVERRIDE)

#if !defined(__APPLE__)
#error "this file should only be included on macOS"
#endif

/* ------------------------------------------------------
   Override system malloc on macOS
   This is done through the malloc zone interface.
   It seems we also need to interpose (see `alloc-override.c`)
   or otherwise we get zone errors as there are usually 
   already allocations done by the time we take over the 
   zone. Unfortunately, that means we need to replace
   the `free` with a checked free (`cfree`) impacting 
   performance.
------------------------------------------------------ */

#include <AvailabilityMacros.h>
#include <malloc/malloc.h>
#include <string.h>  // memset

#if defined(MAC_OS_X_VERSION_10_6) && \
    MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
// only available from OSX 10.6
extern malloc_zone_t* malloc_default_purgeable_zone(void) __attribute__((weak_import));
#endif


/* ------------------------------------------------------
   malloc zone members
------------------------------------------------------ */

static size_t zone_size(malloc_zone_t* zone, const void* p) {
  UNUSED(zone);
  if (!mi_is_in_heap_region(p))
    return 0; // not our pointer, bail out
  
  return mi_usable_size(p);
}

static void* zone_malloc(malloc_zone_t* zone, size_t size) {
  UNUSED(zone);
  return mi_malloc(size);
}

static void* zone_calloc(malloc_zone_t* zone, size_t count, size_t size) {
  UNUSED(zone);
  return mi_calloc(count, size);
}

static void* zone_valloc(malloc_zone_t* zone, size_t size) {
  UNUSED(zone);
  return mi_malloc_aligned(size, _mi_os_page_size());
}

static void zone_free(malloc_zone_t* zone, void* p) {
  UNUSED(zone);
  return mi_free(p);
}

static void* zone_realloc(malloc_zone_t* zone, void* p, size_t newsize) {
  UNUSED(zone);
  return mi_realloc(p, newsize);
}

static void* zone_memalign(malloc_zone_t* zone, size_t alignment, size_t size) {
  UNUSED(zone);
  return mi_malloc_aligned(size,alignment);
}

static void zone_destroy(malloc_zone_t* zone) {
  UNUSED(zone);
  // todo: ignore for now?
}

static unsigned zone_batch_malloc(malloc_zone_t* zone, size_t size, void** ps, unsigned count) {
  size_t i;
  for (i = 0; i < count; i++) {
    ps[i] = zone_malloc(zone, size);
    if (ps[i] == NULL) break;
  }
  return i;
}

static void zone_batch_free(malloc_zone_t* zone, void** ps, unsigned count) {
  for(size_t i = 0; i < count; i++) {
    zone_free(zone, ps[i]);
    ps[i] = NULL;
  }
}

static size_t zone_pressure_relief(malloc_zone_t* zone, size_t size) {
  UNUSED(zone); UNUSED(size);
  mi_collect(false);
  return 0;
}

static void zone_free_definite_size(malloc_zone_t* zone, void* p, size_t size) {
  UNUSED(size);
  zone_free(zone,p);
}


/* ------------------------------------------------------
   Introspection members
------------------------------------------------------ */

static kern_return_t intro_enumerator(task_t task, void* p,
                            unsigned type_mask, vm_address_t zone_address,
                            memory_reader_t reader,
                            vm_range_recorder_t recorder)
{
  // todo: enumerate all memory
  UNUSED(task); UNUSED(p); UNUSED(type_mask); UNUSED(zone_address);
  UNUSED(reader); UNUSED(recorder);
  return KERN_SUCCESS;
}

static size_t intro_good_size(malloc_zone_t* zone, size_t size) {
  UNUSED(zone);
  return mi_good_size(size);
}

static boolean_t intro_check(malloc_zone_t* zone) {
  UNUSED(zone);
  return true;
}

static void intro_print(malloc_zone_t* zone, boolean_t verbose) {
  UNUSED(zone); UNUSED(verbose);
  mi_stats_print(NULL);
}

static void intro_log(malloc_zone_t* zone, void* p) {
  UNUSED(zone); UNUSED(p);
  // todo?
}

static void intro_force_lock(malloc_zone_t* zone) {
  UNUSED(zone);
  // todo?
}

static void intro_force_unlock(malloc_zone_t* zone) {
  UNUSED(zone);
  // todo?
}

static void intro_statistics(malloc_zone_t* zone, malloc_statistics_t* stats) {
  UNUSED(zone);
  // todo...
  stats->blocks_in_use = 0;
  stats->size_in_use = 0;
  stats->max_size_in_use = 0;
  stats->size_allocated = 0;
}

static boolean_t intro_zone_locked(malloc_zone_t* zone) {
  UNUSED(zone);
  return false;
}


/* ------------------------------------------------------
  At process start, override the default allocator
------------------------------------------------------ */

static malloc_zone_t* mi_get_default_zone()
{
  // The first returned zone is the real default
  malloc_zone_t** zones = NULL;
  unsigned count = 0;
  kern_return_t ret = malloc_get_all_zones(0, NULL, (vm_address_t**)&zones, &count);
  if (ret == KERN_SUCCESS && count > 0) {
    return zones[0];
  }
  else {
    // fallback
    return malloc_default_zone();
  }
}

static void __attribute__((constructor)) _mi_macos_override_malloc()
{
  static malloc_introspection_t intro;
  memset(&intro, 0, sizeof(intro));

  intro.enumerator = &intro_enumerator;
  intro.good_size = &intro_good_size;
  intro.check = &intro_check;
  intro.print = &intro_print;
  intro.log = &intro_log;
  intro.force_lock = &intro_force_lock;
  intro.force_unlock = &intro_force_unlock;

  static malloc_zone_t zone;
  memset(&zone, 0, sizeof(zone));

  zone.version = 4;
  zone.zone_name = "mimalloc";
  zone.size = &zone_size;
  zone.introspect = &intro;
  zone.malloc = &zone_malloc;
  zone.calloc = &zone_calloc;
  zone.valloc = &zone_valloc;
  zone.free = &zone_free;
  zone.realloc = &zone_realloc;
  zone.destroy = &zone_destroy;
  zone.batch_malloc = &zone_batch_malloc;
  zone.batch_free = &zone_batch_free;

  malloc_zone_t* purgeable_zone = NULL;

#if defined(MAC_OS_X_VERSION_10_6) && \
    MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
  // switch to version 9 on OSX 10.6 to support memalign.
  zone.version = 9;
  zone.memalign = &zone_memalign;
  zone.free_definite_size = &zone_free_definite_size;
  zone.pressure_relief = &zone_pressure_relief;
  intro.zone_locked = &intro_zone_locked;
  intro.statistics = &intro_statistics;

  // force the purgeable zone to exist to avoid strange bugs
  if (malloc_default_purgeable_zone) {
    purgeable_zone = malloc_default_purgeable_zone();
  }
#endif

  // Register our zone
  malloc_zone_register(&zone);

  // Unregister the default zone, this makes our zone the new default
  // as that was the last registered.
  malloc_zone_t *default_zone = mi_get_default_zone();
  malloc_zone_unregister(default_zone);

  // Reregister the default zone so free and realloc in that zone keep working.
  malloc_zone_register(default_zone);

  // Unregister, and re-register the purgeable_zone to avoid bugs if it occurs
  // earlier than the default zone.
  if (purgeable_zone != NULL) {
    malloc_zone_unregister(purgeable_zone);
    malloc_zone_register(purgeable_zone);
  }

}

#endif // MI_MALLOC_OVERRIDE
/**** ended inlining alloc-override-osx.c ****/
#endif
/**** start inlining init.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/

#include <string.h>  // memcpy, memset
#include <stdlib.h>  // atexit

// Empty page used to initialize the small free pages array
const mi_page_t _mi_page_empty = {
  0, false, false, false, false,
  0,       // capacity
  0,       // reserved capacity
  { 0 },   // flags
  false,   // is_zero
  0,       // retire_expire
  NULL,    // free
  #if MI_ENCODE_FREELIST
  { 0, 0 },
  #endif
  0,       // used
  0,       // xblock_size
  NULL,    // local_free
  ATOMIC_VAR_INIT(0), // xthread_free
  ATOMIC_VAR_INIT(0), // xheap
  NULL, NULL
};

#define MI_PAGE_EMPTY() ((mi_page_t*)&_mi_page_empty)

#if (MI_PADDING>0) && (MI_INTPTR_SIZE >= 8)
#define MI_SMALL_PAGES_EMPTY  { MI_INIT128(MI_PAGE_EMPTY), MI_PAGE_EMPTY(), MI_PAGE_EMPTY() }
#elif (MI_PADDING>0)
#define MI_SMALL_PAGES_EMPTY  { MI_INIT128(MI_PAGE_EMPTY), MI_PAGE_EMPTY(), MI_PAGE_EMPTY(), MI_PAGE_EMPTY() }
#else
#define MI_SMALL_PAGES_EMPTY  { MI_INIT128(MI_PAGE_EMPTY), MI_PAGE_EMPTY() }
#endif


// Empty page queues for every bin
#define QNULL(sz)  { NULL, NULL, (sz)*sizeof(uintptr_t) }
#define MI_PAGE_QUEUES_EMPTY \
  { QNULL(1), \
    QNULL(     1), QNULL(     2), QNULL(     3), QNULL(     4), QNULL(     5), QNULL(     6), QNULL(     7), QNULL(     8), /* 8 */ \
    QNULL(    10), QNULL(    12), QNULL(    14), QNULL(    16), QNULL(    20), QNULL(    24), QNULL(    28), QNULL(    32), /* 16 */ \
    QNULL(    40), QNULL(    48), QNULL(    56), QNULL(    64), QNULL(    80), QNULL(    96), QNULL(   112), QNULL(   128), /* 24 */ \
    QNULL(   160), QNULL(   192), QNULL(   224), QNULL(   256), QNULL(   320), QNULL(   384), QNULL(   448), QNULL(   512), /* 32 */ \
    QNULL(   640), QNULL(   768), QNULL(   896), QNULL(  1024), QNULL(  1280), QNULL(  1536), QNULL(  1792), QNULL(  2048), /* 40 */ \
    QNULL(  2560), QNULL(  3072), QNULL(  3584), QNULL(  4096), QNULL(  5120), QNULL(  6144), QNULL(  7168), QNULL(  8192), /* 48 */ \
    QNULL( 10240), QNULL( 12288), QNULL( 14336), QNULL( 16384), QNULL( 20480), QNULL( 24576), QNULL( 28672), QNULL( 32768), /* 56 */ \
    QNULL( 40960), QNULL( 49152), QNULL( 57344), QNULL( 65536), QNULL( 81920), QNULL( 98304), QNULL(114688), QNULL(131072), /* 64 */ \
    QNULL(163840), QNULL(196608), QNULL(229376), QNULL(262144), QNULL(327680), QNULL(393216), QNULL(458752), QNULL(524288), /* 72 */ \
    QNULL(MI_LARGE_OBJ_WSIZE_MAX + 1  /* 655360, Huge queue */), \
    QNULL(MI_LARGE_OBJ_WSIZE_MAX + 2) /* Full queue */ }

#define MI_STAT_COUNT_NULL()  {0,0,0,0}

// Empty statistics
#if MI_STAT>1
#define MI_STAT_COUNT_END_NULL()  , { MI_STAT_COUNT_NULL(), MI_INIT32(MI_STAT_COUNT_NULL) }
#else
#define MI_STAT_COUNT_END_NULL()
#endif

#define MI_STATS_NULL  \
  MI_STAT_COUNT_NULL(), MI_STAT_COUNT_NULL(), \
  MI_STAT_COUNT_NULL(), MI_STAT_COUNT_NULL(), \
  MI_STAT_COUNT_NULL(), MI_STAT_COUNT_NULL(), \
  MI_STAT_COUNT_NULL(), MI_STAT_COUNT_NULL(), \
  MI_STAT_COUNT_NULL(), MI_STAT_COUNT_NULL(), \
  MI_STAT_COUNT_NULL(), MI_STAT_COUNT_NULL(), \
  MI_STAT_COUNT_NULL(), \
  { 0, 0 }, { 0, 0 }, { 0, 0 },  \
  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } \
  MI_STAT_COUNT_END_NULL()

// --------------------------------------------------------
// Statically allocate an empty heap as the initial
// thread local value for the default heap,
// and statically allocate the backing heap for the main
// thread so it can function without doing any allocation
// itself (as accessing a thread local for the first time
// may lead to allocation itself on some platforms)
// --------------------------------------------------------

const mi_heap_t _mi_heap_empty = {
  NULL,
  MI_SMALL_PAGES_EMPTY,
  MI_PAGE_QUEUES_EMPTY,
  ATOMIC_VAR_INIT(NULL),
  0,                // tid
  0,                // cookie
  { 0, 0 },         // keys
  { {0}, {0}, 0 },
  0,                // page count
  MI_BIN_FULL, 0,   // page retired min/max
  NULL,             // next
  false
};

// the thread-local default heap for allocation
mi_decl_thread mi_heap_t* _mi_heap_default = (mi_heap_t*)&_mi_heap_empty;

extern mi_heap_t _mi_heap_main;

static mi_tld_t tld_main = {
  0, false,
  &_mi_heap_main, &_mi_heap_main,
  { { NULL, NULL }, {NULL ,NULL}, {NULL ,NULL, 0},
    0, 0, 0, 0, 0, 0, NULL,
    &tld_main.stats, &tld_main.os
  }, // segments
  { 0, &tld_main.stats },  // os
  { MI_STATS_NULL }       // stats
};

mi_heap_t _mi_heap_main = {
  &tld_main,
  MI_SMALL_PAGES_EMPTY,
  MI_PAGE_QUEUES_EMPTY,
  ATOMIC_VAR_INIT(NULL),
  0,                // thread id
  0,                // initial cookie
  { 0, 0 },         // the key of the main heap can be fixed (unlike page keys that need to be secure!)
  { {0x846ca68b}, {0}, 0 },  // random
  0,                // page count
  MI_BIN_FULL, 0,   // page retired min/max
  NULL,             // next heap
  false             // can reclaim
};

bool _mi_process_is_initialized = false;  // set to `true` in `mi_process_init`.

mi_stats_t _mi_stats_main = { MI_STATS_NULL };


static void mi_heap_main_init(void) {
  if (_mi_heap_main.cookie == 0) {
    _mi_heap_main.thread_id = _mi_thread_id();
    _mi_heap_main.cookie = _os_random_weak((uintptr_t)&mi_heap_main_init);
    _mi_random_init(&_mi_heap_main.random);
    _mi_heap_main.keys[0] = _mi_heap_random_next(&_mi_heap_main);
    _mi_heap_main.keys[1] = _mi_heap_random_next(&_mi_heap_main);
  }
}

mi_heap_t* _mi_heap_main_get(void) {
  mi_heap_main_init();
  return &_mi_heap_main;
}


/* -----------------------------------------------------------
  Initialization and freeing of the thread local heaps
----------------------------------------------------------- */

// note: in x64 in release build `sizeof(mi_thread_data_t)` is under 4KiB (= OS page size).
typedef struct mi_thread_data_s {
  mi_heap_t  heap;  // must come first due to cast in `_mi_heap_done`
  mi_tld_t   tld;
} mi_thread_data_t;

// Initialize the thread local default heap, called from `mi_thread_init`
static bool _mi_heap_init(void) {
  if (mi_heap_is_initialized(mi_get_default_heap())) return true;
  if (_mi_is_main_thread()) {
    // mi_assert_internal(_mi_heap_main.thread_id != 0);  // can happen on freeBSD where alloc is called before any initialization
    // the main heap is statically allocated
    mi_heap_main_init();
    _mi_heap_set_default_direct(&_mi_heap_main);
    //mi_assert_internal(_mi_heap_default->tld->heap_backing == mi_get_default_heap());
  }
  else {
    // use `_mi_os_alloc` to allocate directly from the OS
    mi_thread_data_t* td = (mi_thread_data_t*)_mi_os_alloc(sizeof(mi_thread_data_t), &_mi_stats_main); // Todo: more efficient allocation?
    if (td == NULL) {
      // if this fails, try once more. (issue #257)
      td = (mi_thread_data_t*)_mi_os_alloc(sizeof(mi_thread_data_t), &_mi_stats_main);
      if (td == NULL) {
        // really out of memory
        _mi_error_message(ENOMEM, "unable to allocate thread local heap metadata (%zu bytes)\n", sizeof(mi_thread_data_t));
        return false;
      }
    }
    // OS allocated so already zero initialized
    mi_tld_t*  tld = &td->tld;
    mi_heap_t* heap = &td->heap;
    memcpy(heap, &_mi_heap_empty, sizeof(*heap));
    heap->thread_id = _mi_thread_id();
    _mi_random_init(&heap->random);
    heap->cookie  = _mi_heap_random_next(heap) | 1;
    heap->keys[0] = _mi_heap_random_next(heap);
    heap->keys[1] = _mi_heap_random_next(heap);
    heap->tld = tld;
    tld->heap_backing = heap;
    tld->heaps = heap;
    tld->segments.stats = &tld->stats;
    tld->segments.os = &tld->os;
    tld->os.stats = &tld->stats;
    _mi_heap_set_default_direct(heap);
  }
  return false;
}

// Free the thread local default heap (called from `mi_thread_done`)
static bool _mi_heap_done(mi_heap_t* heap) {
  if (!mi_heap_is_initialized(heap)) return true;

  // reset default heap
  _mi_heap_set_default_direct(_mi_is_main_thread() ? &_mi_heap_main : (mi_heap_t*)&_mi_heap_empty);

  // switch to backing heap
  heap = heap->tld->heap_backing;
  if (!mi_heap_is_initialized(heap)) return false;

  // delete all non-backing heaps in this thread
  mi_heap_t* curr = heap->tld->heaps;
  while (curr != NULL) {
    mi_heap_t* next = curr->next; // save `next` as `curr` will be freed
    if (curr != heap) {
      mi_assert_internal(!mi_heap_is_backing(curr));
      mi_heap_delete(curr);
    }
    curr = next;
  }
  mi_assert_internal(heap->tld->heaps == heap && heap->next == NULL);
  mi_assert_internal(mi_heap_is_backing(heap));

  // collect if not the main thread
  if (heap != &_mi_heap_main) {
    _mi_heap_collect_abandon(heap);
  }
  

  // merge stats
  _mi_stats_done(&heap->tld->stats);

  // free if not the main thread
  if (heap != &_mi_heap_main) {
    mi_assert_internal(heap->tld->segments.count == 0 || heap->thread_id != _mi_thread_id());
    _mi_os_free(heap, sizeof(mi_thread_data_t), &_mi_stats_main);
  }
#if 0  
  // never free the main thread even in debug mode; if a dll is linked statically with mimalloc,
  // there may still be delete/free calls after the mi_fls_done is called. Issue #207
  else {
    _mi_heap_destroy_pages(heap);
    mi_assert_internal(heap->tld->heap_backing == &_mi_heap_main);
  }
#endif
  return false;
}



// --------------------------------------------------------
// Try to run `mi_thread_done()` automatically so any memory
// owned by the thread but not yet released can be abandoned
// and re-owned by another thread.
//
// 1. windows dynamic library:
//     call from DllMain on DLL_THREAD_DETACH
// 2. windows static library:
//     use `FlsAlloc` to call a destructor when the thread is done
// 3. unix, pthreads:
//     use a pthread key to call a destructor when a pthread is done
//
// In the last two cases we also need to call `mi_process_init`
// to set up the thread local keys.
// --------------------------------------------------------

static void _mi_thread_done(mi_heap_t* default_heap);

#ifdef __wasi__
// no pthreads in the WebAssembly Standard Interface
#elif !defined(_WIN32)
#define MI_USE_PTHREADS
#endif

#if defined(_WIN32) && defined(MI_SHARED_LIB)
  // nothing to do as it is done in DllMain
#elif defined(_WIN32) && !defined(MI_SHARED_LIB)
  // use thread local storage keys to detect thread ending
  #include <windows.h>
  #include <fibersapi.h>
  #if (_WIN32_WINNT < 0x600)  // before Windows Vista 
  WINBASEAPI DWORD WINAPI FlsAlloc( _In_opt_ PFLS_CALLBACK_FUNCTION lpCallback );
  WINBASEAPI PVOID WINAPI FlsGetValue( _In_ DWORD dwFlsIndex );
  WINBASEAPI BOOL  WINAPI FlsSetValue( _In_ DWORD dwFlsIndex, _In_opt_ PVOID lpFlsData );
  WINBASEAPI BOOL  WINAPI FlsFree(_In_ DWORD dwFlsIndex);
  #endif
  static DWORD mi_fls_key = (DWORD)(-1);
  static void NTAPI mi_fls_done(PVOID value) {
    if (value!=NULL) _mi_thread_done((mi_heap_t*)value);
  }
#elif defined(MI_USE_PTHREADS)
  // use pthread local storage keys to detect thread ending
  // (and used with MI_TLS_PTHREADS for the default heap)
  #include <pthread.h>
  pthread_key_t _mi_heap_default_key = (pthread_key_t)(-1);
  static void mi_pthread_done(void* value) {
    if (value!=NULL) _mi_thread_done((mi_heap_t*)value);
  }
#elif defined(__wasi__)
// no pthreads in the WebAssembly Standard Interface
#else
  #pragma message("define a way to call mi_thread_done when a thread is done")
#endif

// Set up handlers so `mi_thread_done` is called automatically
static void mi_process_setup_auto_thread_done(void) {
  static bool tls_initialized = false; // fine if it races
  if (tls_initialized) return;
  tls_initialized = true;
  #if defined(_WIN32) && defined(MI_SHARED_LIB)
    // nothing to do as it is done in DllMain
  #elif defined(_WIN32) && !defined(MI_SHARED_LIB)
    mi_fls_key = FlsAlloc(&mi_fls_done);
  #elif defined(MI_USE_PTHREADS)
    mi_assert_internal(_mi_heap_default_key == (pthread_key_t)(-1));
    pthread_key_create(&_mi_heap_default_key, &mi_pthread_done);
  #endif
  _mi_heap_set_default_direct(&_mi_heap_main);
}


bool _mi_is_main_thread(void) {
  return (_mi_heap_main.thread_id==0 || _mi_heap_main.thread_id == _mi_thread_id());
}

// This is called from the `mi_malloc_generic`
void mi_thread_init(void) mi_attr_noexcept
{
  // ensure our process has started already
  mi_process_init();

  // initialize the thread local default heap
  // (this will call `_mi_heap_set_default_direct` and thus set the
  //  fiber/pthread key to a non-zero value, ensuring `_mi_thread_done` is called)
  if (_mi_heap_init()) return;  // returns true if already initialized

  // don't further initialize for the main thread
  if (_mi_is_main_thread()) return;

  mi_heap_t* const heap = mi_get_default_heap();
  if (mi_heap_is_initialized(heap)) { _mi_stat_increase(&heap->tld->stats.threads, 1); }

  //_mi_verbose_message("thread init: 0x%zx\n", _mi_thread_id());
}

void mi_thread_done(void) mi_attr_noexcept {
  _mi_thread_done(mi_get_default_heap());
}

static void _mi_thread_done(mi_heap_t* heap) {
  // check thread-id as on Windows shutdown with FLS the main (exit) thread may call this on thread-local heaps...
  if (heap->thread_id != _mi_thread_id()) return;

  // stats
  if (!_mi_is_main_thread() && mi_heap_is_initialized(heap))  {
    _mi_stat_decrease(&heap->tld->stats.threads, 1);
  }

  // abandon the thread local heap
  if (_mi_heap_done(heap)) return;  // returns true if already ran
}

void _mi_heap_set_default_direct(mi_heap_t* heap)  {
  mi_assert_internal(heap != NULL);
  #if defined(MI_TLS_SLOT)
  mi_tls_slot_set(MI_TLS_SLOT,heap);
  #elif defined(MI_TLS_PTHREAD_SLOT_OFS)
  *mi_tls_pthread_heap_slot() = heap;
  #elif defined(MI_TLS_PTHREAD)
  // we use _mi_heap_default_key
  #else
  _mi_heap_default = heap;
  #endif

  // ensure the default heap is passed to `_mi_thread_done`
  // setting to a non-NULL value also ensures `mi_thread_done` is called.
  #if defined(_WIN32) && defined(MI_SHARED_LIB)
    // nothing to do as it is done in DllMain
  #elif defined(_WIN32) && !defined(MI_SHARED_LIB)
    mi_assert_internal(mi_fls_key != 0);
    FlsSetValue(mi_fls_key, heap);
  #elif defined(MI_USE_PTHREADS)
  if (_mi_heap_default_key != (pthread_key_t)(-1)) {  // can happen during recursive invocation on freeBSD
    pthread_setspecific(_mi_heap_default_key, heap);
  }
  #endif
}


// --------------------------------------------------------
// Run functions on process init/done, and thread init/done
// --------------------------------------------------------
static void mi_process_done(void);

static bool os_preloading = true;    // true until this module is initialized
static bool mi_redirected = false;   // true if malloc redirects to mi_malloc

// Returns true if this module has not been initialized; Don't use C runtime routines until it returns false.
bool _mi_preloading() {
  return os_preloading;
}

bool mi_is_redirected() mi_attr_noexcept {
  return mi_redirected;
}

// Communicate with the redirection module on Windows
#if defined(_WIN32) && defined(MI_SHARED_LIB)
#ifdef __cplusplus
extern "C" {
#endif
mi_decl_export void _mi_redirect_entry(DWORD reason) {
  // called on redirection; careful as this may be called before DllMain
  if (reason == DLL_PROCESS_ATTACH) {
    mi_redirected = true;
  }
  else if (reason == DLL_PROCESS_DETACH) {
    mi_redirected = false;
  }
  else if (reason == DLL_THREAD_DETACH) {
    mi_thread_done();
  }
}
__declspec(dllimport) bool mi_allocator_init(const char** message);
__declspec(dllimport) void mi_allocator_done();
#ifdef __cplusplus
}
#endif
#else
static bool mi_allocator_init(const char** message) {
  if (message != NULL) *message = NULL;
  return true;
}
static void mi_allocator_done() {
  // nothing to do
}
#endif

// Called once by the process loader
static void mi_process_load(void) {
  mi_heap_main_init();
  #if defined(MI_TLS_RECURSE_GUARD)
  volatile mi_heap_t* dummy = _mi_heap_default; // access TLS to allocate it before setting tls_initialized to true;
  UNUSED(dummy);
  #endif
  os_preloading = false;
  atexit(&mi_process_done);
  _mi_options_init();
  mi_process_init();
  //mi_stats_reset();-
  if (mi_redirected) _mi_verbose_message("malloc is redirected.\n");

  // show message from the redirector (if present)
  const char* msg = NULL;
  mi_allocator_init(&msg);
  if (msg != NULL && (mi_option_is_enabled(mi_option_verbose) || mi_option_is_enabled(mi_option_show_errors))) {
    _mi_fputs(NULL,NULL,NULL,msg);
  }
}

// Initialize the process; called by thread_init or the process loader
void mi_process_init(void) mi_attr_noexcept {
  // ensure we are called once
  if (_mi_process_is_initialized) return;
  _mi_process_is_initialized = true;
  mi_process_setup_auto_thread_done();

  _mi_verbose_message("process init: 0x%zx\n", _mi_thread_id());
  _mi_os_init();
  mi_heap_main_init();
  #if (MI_DEBUG)
  _mi_verbose_message("debug level : %d\n", MI_DEBUG);
  #endif
  _mi_verbose_message("secure level: %d\n", MI_SECURE);
  mi_thread_init();
  mi_stats_reset();  // only call stat reset *after* thread init (or the heap tld == NULL)

  if (mi_option_is_enabled(mi_option_reserve_huge_os_pages)) {
    size_t pages = mi_option_get(mi_option_reserve_huge_os_pages);
    mi_reserve_huge_os_pages_interleave(pages, 0, pages*500);
  }
}

// Called when the process is done (through `at_exit`)
static void mi_process_done(void) {
  // only shutdown if we were initialized
  if (!_mi_process_is_initialized) return;
  // ensure we are called once
  static bool process_done = false;
  if (process_done) return;
  process_done = true;

  #if defined(_WIN32) && !defined(MI_SHARED_LIB)
  FlsSetValue(mi_fls_key, NULL);  // don't call main-thread callback
  FlsFree(mi_fls_key);            // call thread-done on all threads to prevent dangling callback pointer if statically linked with a DLL; Issue #208
  #endif
  
  #if (MI_DEBUG != 0) || !defined(MI_SHARED_LIB)  
  // free all memory if possible on process exit. This is not needed for a stand-alone process
  // but should be done if mimalloc is statically linked into another shared library which
  // is repeatedly loaded/unloaded, see issue #281.
  mi_collect(true /* force */ );
  #endif

  if (mi_option_is_enabled(mi_option_show_stats) || mi_option_is_enabled(mi_option_verbose)) {
    mi_stats_print(NULL);
  }
  mi_allocator_done();  
  _mi_verbose_message("process done: 0x%zx\n", _mi_heap_main.thread_id);
  os_preloading = true; // don't call the C runtime anymore
}



#if defined(_WIN32) && defined(MI_SHARED_LIB)
  // Windows DLL: easy to hook into process_init and thread_done
  __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved) {
    UNUSED(reserved);
    UNUSED(inst);
    if (reason==DLL_PROCESS_ATTACH) {
      mi_process_load();
    }
    else if (reason==DLL_THREAD_DETACH) {
      if (!mi_is_redirected()) mi_thread_done();
    }
    return TRUE;
  }

#elif defined(__cplusplus)
  // C++: use static initialization to detect process start
  static bool _mi_process_init(void) {
    mi_process_load();
    return (_mi_heap_main.thread_id != 0);
  }
  static bool mi_initialized = _mi_process_init();

#elif defined(__GNUC__) || defined(__clang__)
  // GCC,Clang: use the constructor attribute
  static void __attribute__((constructor)) _mi_process_init(void) {
    mi_process_load();
  }

#elif defined(_MSC_VER)
  // MSVC: use data section magic for static libraries
  // See <https://www.codeguru.com/cpp/misc/misc/applicationcontrol/article.php/c6945/Running-Code-Before-and-After-Main.htm>
  static int _mi_process_init(void) {
    mi_process_load();
    return 0;
  }
  typedef int(*_crt_cb)(void);
  #ifdef _M_X64
    __pragma(comment(linker, "/include:" "_mi_msvc_initu"))
    #pragma section(".CRT$XIU", long, read)
  #else
    __pragma(comment(linker, "/include:" "__mi_msvc_initu"))
  #endif
  #pragma data_seg(".CRT$XIU")
  _crt_cb _mi_msvc_initu[] = { &_mi_process_init };
  #pragma data_seg()

#else
#pragma message("define a way to call mi_process_load on your platform")
#endif
/**** ended inlining init.c ****/
/**** start inlining options.c ****/
/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
/**** skipping file: mimalloc.h ****/
/**** skipping file: mimalloc-internal.h ****/
/**** skipping file: mimalloc-atomic.h ****/

#include <stdio.h>
#include <stdlib.h> // strtol
#include <string.h> // strncpy, strncat, strlen, strstr
#include <ctype.h>  // toupper
#include <stdarg.h>

static uintptr_t mi_max_error_count = 16;  // stop outputting errors after this

static void mi_add_stderr_output();

int mi_version(void) mi_attr_noexcept {
  return MI_MALLOC_VERSION;
}

#ifdef _WIN32
#include <conio.h>
#endif

// --------------------------------------------------------
// Options
// These can be accessed by multiple threads and may be
// concurrently initialized, but an initializing data race
// is ok since they resolve to the same value.
// --------------------------------------------------------
typedef enum mi_init_e {
  UNINIT,       // not yet initialized
  DEFAULTED,    // not found in the environment, use default value
  INITIALIZED   // found in environment or set explicitly
} mi_init_t;

typedef struct mi_option_desc_s {
  long        value;  // the value
  mi_init_t   init;   // is it initialized yet? (from the environment)
  mi_option_t option; // for debugging: the option index should match the option
  const char* name;   // option name without `mimalloc_` prefix
} mi_option_desc_t;

#define MI_OPTION(opt)        mi_option_##opt, #opt
#define MI_OPTION_DESC(opt)   {0, UNINIT, MI_OPTION(opt) }

static mi_option_desc_t options[_mi_option_last] =
{
  // stable options
#if MI_DEBUG || defined(MI_SHOW_ERRORS)
  { 1, UNINIT, MI_OPTION(show_errors) },
#else
  { 0, UNINIT, MI_OPTION(show_errors) },
#endif
  { 0, UNINIT, MI_OPTION(show_stats) },
  { 0, UNINIT, MI_OPTION(verbose) },

  // the following options are experimental and not all combinations make sense.
  { 1, UNINIT, MI_OPTION(eager_commit) },        // commit on demand
  #if defined(_WIN32) || (MI_INTPTR_SIZE <= 4)   // and other OS's without overcommit?
  { 0, UNINIT, MI_OPTION(eager_region_commit) },
  { 1, UNINIT, MI_OPTION(reset_decommits) },     // reset decommits memory
  #else
  { 1, UNINIT, MI_OPTION(eager_region_commit) },
  { 0, UNINIT, MI_OPTION(reset_decommits) },     // reset uses MADV_FREE/MADV_DONTNEED
  #endif
  { 0, UNINIT, MI_OPTION(large_os_pages) },      // use large OS pages, use only with eager commit to prevent fragmentation of VMA's
  { 0, UNINIT, MI_OPTION(reserve_huge_os_pages) },
  { 0, UNINIT, MI_OPTION(segment_cache) },       // cache N segments per thread
  { 1, UNINIT, MI_OPTION(page_reset) },          // reset page memory on free
  { 0, UNINIT, MI_OPTION(abandoned_page_reset) },// reset free page memory when a thread terminates
  { 0, UNINIT, MI_OPTION(segment_reset) },       // reset segment memory on free (needs eager commit)
#if defined(__NetBSD__)
  { 0, UNINIT, MI_OPTION(eager_commit_delay) },  // the first N segments per thread are not eagerly committed
#else
  { 1, UNINIT, MI_OPTION(eager_commit_delay) },  // the first N segments per thread are not eagerly committed
#endif
  { 100, UNINIT, MI_OPTION(reset_delay) },       // reset delay in milli-seconds
  { 0,   UNINIT, MI_OPTION(use_numa_nodes) },    // 0 = use available numa nodes, otherwise use at most N nodes.
  { 100, UNINIT, MI_OPTION(os_tag) },            // only apple specific for now but might serve more or less related purpose
  { 16,  UNINIT, MI_OPTION(max_errors) }         // maximum errors that are output
};

static void mi_option_init(mi_option_desc_t* desc);

void _mi_options_init(void) {
  // called on process load; should not be called before the CRT is initialized!
  // (e.g. do not call this from process_init as that may run before CRT initialization)
  mi_add_stderr_output(); // now it safe to use stderr for output
  for(int i = 0; i < _mi_option_last; i++ ) {
    mi_option_t option = (mi_option_t)i;
    long l = mi_option_get(option); UNUSED(l); // initialize
    if (option != mi_option_verbose) {
      mi_option_desc_t* desc = &options[option];
      _mi_verbose_message("option '%s': %ld\n", desc->name, desc->value);
    }
  }
  mi_max_error_count = mi_option_get(mi_option_max_errors);
}

long mi_option_get(mi_option_t option) {
  mi_assert(option >= 0 && option < _mi_option_last);
  mi_option_desc_t* desc = &options[option];
  mi_assert(desc->option == option);  // index should match the option
  if (mi_unlikely(desc->init == UNINIT)) {
    mi_option_init(desc);
  }
  return desc->value;
}

void mi_option_set(mi_option_t option, long value) {
  mi_assert(option >= 0 && option < _mi_option_last);
  mi_option_desc_t* desc = &options[option];
  mi_assert(desc->option == option);  // index should match the option
  desc->value = value;
  desc->init = INITIALIZED;
}

void mi_option_set_default(mi_option_t option, long value) {
  mi_assert(option >= 0 && option < _mi_option_last);
  mi_option_desc_t* desc = &options[option];
  if (desc->init != INITIALIZED) {
    desc->value = value;
  }
}

bool mi_option_is_enabled(mi_option_t option) {
  return (mi_option_get(option) != 0);
}

void mi_option_set_enabled(mi_option_t option, bool enable) {
  mi_option_set(option, (enable ? 1 : 0));
}

void mi_option_set_enabled_default(mi_option_t option, bool enable) {
  mi_option_set_default(option, (enable ? 1 : 0));
}

void mi_option_enable(mi_option_t option) {
  mi_option_set_enabled(option,true);
}

void mi_option_disable(mi_option_t option) {
  mi_option_set_enabled(option,false);
}


static void mi_out_stderr(const char* msg, void* arg) {
  UNUSED(arg);
  #ifdef _WIN32
  // on windows with redirection, the C runtime cannot handle locale dependent output
  // after the main thread closes so we use direct console output.
  if (!_mi_preloading()) { _cputs(msg); }
  #else
  fputs(msg, stderr);
  #endif
}

// Since an output function can be registered earliest in the `main`
// function we also buffer output that happens earlier. When
// an output function is registered it is called immediately with
// the output up to that point.
#ifndef MI_MAX_DELAY_OUTPUT
#define MI_MAX_DELAY_OUTPUT (32*1024)
#endif
static char out_buf[MI_MAX_DELAY_OUTPUT+1];
static _Atomic(uintptr_t) out_len;

static void mi_out_buf(const char* msg, void* arg) {
  UNUSED(arg);
  if (msg==NULL) return;
  if (mi_atomic_read_relaxed(&out_len)>=MI_MAX_DELAY_OUTPUT) return;
  size_t n = strlen(msg);
  if (n==0) return;
  // claim space
  uintptr_t start = mi_atomic_add(&out_len, n);
  if (start >= MI_MAX_DELAY_OUTPUT) return;
  // check bound
  if (start+n >= MI_MAX_DELAY_OUTPUT) {
    n = MI_MAX_DELAY_OUTPUT-start-1;
  }
  memcpy(&out_buf[start], msg, n);
}

static void mi_out_buf_flush(mi_output_fun* out, bool no_more_buf, void* arg) {
  if (out==NULL) return;
  // claim (if `no_more_buf == true`, no more output will be added after this point)
  size_t count = mi_atomic_add(&out_len, (no_more_buf ? MI_MAX_DELAY_OUTPUT : 1));
  // and output the current contents
  if (count>MI_MAX_DELAY_OUTPUT) count = MI_MAX_DELAY_OUTPUT;
  out_buf[count] = 0;
  out(out_buf,arg);
  if (!no_more_buf) {
    out_buf[count] = '\n'; // if continue with the buffer, insert a newline
  }
}


// Once this module is loaded, switch to this routine
// which outputs to stderr and the delayed output buffer.
static void mi_out_buf_stderr(const char* msg, void* arg) {
  mi_out_stderr(msg,arg);
  mi_out_buf(msg,arg);
}



// --------------------------------------------------------
// Default output handler
// --------------------------------------------------------

// Should be atomic but gives errors on many platforms as generally we cannot cast a function pointer to a uintptr_t.
// For now, don't register output from multiple threads.
#pragma warning(suppress:4180)
static mi_output_fun* volatile mi_out_default; // = NULL
static volatile _Atomic(void*) mi_out_arg; // = NULL

static mi_output_fun* mi_out_get_default(void** parg) {
  if (parg != NULL) { *parg = mi_atomic_read_ptr(void,&mi_out_arg); }
  mi_output_fun* out = mi_out_default;
  return (out == NULL ? &mi_out_buf : out);
}

void mi_register_output(mi_output_fun* out, void* arg) mi_attr_noexcept {
  mi_out_default = (out == NULL ? &mi_out_stderr : out); // stop using the delayed output buffer
  mi_atomic_write_ptr(void,&mi_out_arg, arg);
  if (out!=NULL) mi_out_buf_flush(out,true,arg);         // output all the delayed output now
}

// add stderr to the delayed output after the module is loaded
static void mi_add_stderr_output() {
  mi_assert_internal(mi_out_default == NULL);
  mi_out_buf_flush(&mi_out_stderr, false, NULL); // flush current contents to stderr
  mi_out_default = &mi_out_buf_stderr;           // and add stderr to the delayed output
}

// --------------------------------------------------------
// Messages, all end up calling `_mi_fputs`.
// --------------------------------------------------------
static volatile _Atomic(uintptr_t) error_count; // = 0;  // when MAX_ERROR_COUNT stop emitting errors and warnings

// When overriding malloc, we may recurse into mi_vfprintf if an allocation
// inside the C runtime causes another message.
static mi_decl_thread bool recurse = false;

static bool mi_recurse_enter(void) {
  #ifdef MI_TLS_RECURSE_GUARD
  if (_mi_preloading()) return true;
  #endif
  if (recurse) return false;
  recurse = true;
  return true;
}

static void mi_recurse_exit(void) {
  #ifdef MI_TLS_RECURSE_GUARD
  if (_mi_preloading()) return;
  #endif
  recurse = false;
}

void _mi_fputs(mi_output_fun* out, void* arg, const char* prefix, const char* message) {
  if (out==NULL || (FILE*)out==stdout || (FILE*)out==stderr) { // TODO: use mi_out_stderr for stderr?
    if (!mi_recurse_enter()) return;
    out = mi_out_get_default(&arg);
    if (prefix != NULL) out(prefix, arg);
    out(message, arg);
    mi_recurse_exit();
  }
  else {
    if (prefix != NULL) out(prefix, arg);
    out(message, arg);
  }
}

// Define our own limited `fprintf` that avoids memory allocation.
// We do this using `snprintf` with a limited buffer.
static void mi_vfprintf( mi_output_fun* out, void* arg, const char* prefix, const char* fmt, va_list args ) {
  char buf[512];
  if (fmt==NULL) return;
  if (!mi_recurse_enter()) return;
  vsnprintf(buf,sizeof(buf)-1,fmt,args);
  mi_recurse_exit();
  _mi_fputs(out,arg,prefix,buf);
}

void _mi_fprintf( mi_output_fun* out, void* arg, const char* fmt, ... ) {
  va_list args;
  va_start(args,fmt);
  mi_vfprintf(out,arg,NULL,fmt,args);
  va_end(args);
}

void _mi_trace_message(const char* fmt, ...) {
  if (mi_option_get(mi_option_verbose) <= 1) return;  // only with verbose level 2 or higher
  va_list args;
  va_start(args, fmt);
  mi_vfprintf(NULL, NULL, "mimalloc: ", fmt, args);
  va_end(args);
}

void _mi_verbose_message(const char* fmt, ...) {
  if (!mi_option_is_enabled(mi_option_verbose)) return;
  va_list args;
  va_start(args,fmt);
  mi_vfprintf(NULL, NULL, "mimalloc: ", fmt, args);
  va_end(args);
}

static void mi_show_error_message(const char* fmt, va_list args) {
  if (!mi_option_is_enabled(mi_option_show_errors) && !mi_option_is_enabled(mi_option_verbose)) return;
  if (mi_atomic_increment(&error_count) > mi_max_error_count) return;
  mi_vfprintf(NULL, NULL, "mimalloc: error: ", fmt, args);
}

void _mi_warning_message(const char* fmt, ...) {
  if (!mi_option_is_enabled(mi_option_show_errors) && !mi_option_is_enabled(mi_option_verbose)) return;
  if (mi_atomic_increment(&error_count) > mi_max_error_count) return;
  va_list args;
  va_start(args,fmt);
  mi_vfprintf(NULL, NULL, "mimalloc: warning: ", fmt, args);
  va_end(args);
}


#if MI_DEBUG
void _mi_assert_fail(const char* assertion, const char* fname, unsigned line, const char* func ) {
  _mi_fprintf(NULL, NULL, "mimalloc: assertion failed: at \"%s\":%u, %s\n  assertion: \"%s\"\n", fname, line, (func==NULL?"":func), assertion);
  abort();
}
#endif

// --------------------------------------------------------
// Errors
// --------------------------------------------------------

static mi_error_fun* volatile  mi_error_handler; // = NULL
static volatile _Atomic(void*) mi_error_arg;     // = NULL

static void mi_error_default(int err) {
  UNUSED(err);
#if (MI_DEBUG>0) 
  if (err==EFAULT) {
    #ifdef _MSC_VER
    __debugbreak();
    #endif
    abort();
  }
#endif
#if (MI_SECURE>0)
  if (err==EFAULT) {  // abort on serious errors in secure mode (corrupted meta-data)
    abort();
  }
#endif
#if defined(MI_XMALLOC)
  if (err==ENOMEM || err==EOVERFLOW) { // abort on memory allocation fails in xmalloc mode
    abort();
  }
#endif
}

void mi_register_error(mi_error_fun* fun, void* arg) {
  mi_error_handler = fun;  // can be NULL
  mi_atomic_write_ptr(void,&mi_error_arg, arg);
}

void _mi_error_message(int err, const char* fmt, ...) {
  // show detailed error message
  va_list args;
  va_start(args, fmt);
  mi_show_error_message(fmt, args);
  va_end(args);
  // and call the error handler which may abort (or return normally)
  if (mi_error_handler != NULL) {
    mi_error_handler(err, mi_atomic_read_ptr(void,&mi_error_arg));
  }
  else {
    mi_error_default(err);
  }
}

// --------------------------------------------------------
// Initialize options by checking the environment
// --------------------------------------------------------

static void mi_strlcpy(char* dest, const char* src, size_t dest_size) {
  dest[0] = 0;
  #pragma warning(suppress:4996)
  strncpy(dest, src, dest_size - 1);
  dest[dest_size - 1] = 0;
}

static void mi_strlcat(char* dest, const char* src, size_t dest_size) {
  #pragma warning(suppress:4996)
  strncat(dest, src, dest_size - 1);
  dest[dest_size - 1] = 0;
}

static inline int mi_strnicmp(const char* s, const char* t, size_t n) {
  if (n==0) return 0;
  for (; *s != 0 && *t != 0 && n > 0; s++, t++, n--) {
    if (toupper(*s) != toupper(*t)) break;
  }
  return (n==0 ? 0 : *s - *t);
}

#if 1
static bool mi_getenv(const char* name, char* result, size_t result_size) {
    return false;
}
#elif defined _WIN32
// On Windows use GetEnvironmentVariable instead of getenv to work
// reliably even when this is invoked before the C runtime is initialized.
// i.e. when `_mi_preloading() == true`.
// Note: on windows, environment names are not case sensitive.
#include <windows.h>
static bool mi_getenv(const char* name, char* result, size_t result_size) {
  result[0] = 0;
  size_t len = GetEnvironmentVariableA(name, result, (DWORD)result_size);
  return (len > 0 && len < result_size);
}
#elif !defined(MI_USE_ENVIRON) || (MI_USE_ENVIRON!=0)
// On Posix systemsr use `environ` to acces environment variables 
// even before the C runtime is initialized.
#if defined(__APPLE__)
#include <crt_externs.h>
static char** mi_get_environ(void) {
  return (*_NSGetEnviron());
}
#else 
extern char** environ;
static char** mi_get_environ(void) {
  return environ;
}
#endif
static bool mi_getenv(const char* name, char* result, size_t result_size) {
  if (name==NULL) return false;  
  const size_t len = strlen(name);
  if (len == 0) return false;  
  char** env = mi_get_environ();
  if (env == NULL) return false;
  // compare up to 256 entries
  for (int i = 0; i < 256 && env[i] != NULL; i++) {
    const char* s = env[i];
    if (mi_strnicmp(name, s, len) == 0 && s[len] == '=') { // case insensitive
      // found it
      mi_strlcpy(result, s + len + 1, result_size);
      return true;
    }
  }
  return false;
}
#else  
// fallback: use standard C `getenv` but this cannot be used while initializing the C runtime
static bool mi_getenv(const char* name, char* result, size_t result_size) {
  // cannot call getenv() when still initializing the C runtime.
  if (_mi_preloading()) return false;
  const char* s = getenv(name);
  if (s == NULL) {
    // we check the upper case name too.
    char buf[64+1];
    size_t len = strlen(name);
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    for (size_t i = 0; i < len; i++) {
      buf[i] = toupper(name[i]);
    }
    buf[len] = 0;
    s = getenv(buf);
  }
  if (s != NULL && strlen(s) < result_size) {
    mi_strlcpy(result, s, result_size);
    return true;
  }
  else {
    return false;
  }
}
#endif

static void mi_option_init(mi_option_desc_t* desc) {  
  // Read option value from the environment
  char buf[64+1];
  mi_strlcpy(buf, "mimalloc_", sizeof(buf));
  mi_strlcat(buf, desc->name, sizeof(buf));
  char s[64+1];
  if (mi_getenv(buf, s, sizeof(s))) {
    size_t len = strlen(s);
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    for (size_t i = 0; i < len; i++) {
      buf[i] = (char)toupper(s[i]);
    }
    buf[len] = 0;
    if (buf[0]==0 || strstr("1;TRUE;YES;ON", buf) != NULL) {
      desc->value = 1;
      desc->init = INITIALIZED;
    }
    else if (strstr("0;FALSE;NO;OFF", buf) != NULL) {
      desc->value = 0;
      desc->init = INITIALIZED;
    }
    else {
      char* end = buf;
      long value = strtol(buf, &end, 10);
      if (*end == 0) {
        desc->value = value;
        desc->init = INITIALIZED;
      }
      else {
        _mi_warning_message("environment option mimalloc_%s has an invalid value: %s\n", desc->name, buf);
        desc->init = DEFAULTED;
      }
    }
    mi_assert_internal(desc->init != UNINIT);
  }
  else if (!_mi_preloading()) {
    desc->init = DEFAULTED;
  }
}
/**** ended inlining options.c ****/

#endif
