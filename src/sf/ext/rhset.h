#ifndef RHSET_H_INCLUDED
#define RHSET_H_INCLUDED

#ifndef RHSET_NO_STDINT
	#include <stddef.h>
	#include <stdint.h>
#endif

#ifndef RHSET_FORCEINLINE
	#if defined(_MSC_VER)
		#define RHSET_FORCEINLINE __forceinline
	#elif defined(__GNUC__)
		#define RHSET_FORCEINLINE __attribute__((always_inline, unused))
	#else
		#define RHSET_FORCEINLINE
	#endif
#endif

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct rhset {

	// Internal state
	uint32_t *entries;
	uint32_t mask;

	// Maximum number of entries that fit in the set before needing to re-hash.
	uint32_t capacity;

	// Number of entries in the set
	uint32_t size;

} rhset;

// Initialize the set to zero.
// Equivalent to `memset(set, 0, sizeof(rhset))`.
void rhset_init(rhset *set);

// Free the set data and reset to zero.
// Returns pointer to free, eg. `free(rhset_reset(set))`.
// If you don't free the pointer you can use this to move the set to another struct
void *rhset_reset(rhset *set);

// Remove all entries from the set without freeing the internal storage.
void rhset_clear(rhset *set);

// Retrieve the size of the current internal data pointer
size_t rhset_alloc_size(const rhset *set);

// Calculate required entry count and internal allocation size to fit at least one more entry to the set.
void rhset_grow(const rhset *set, size_t *p_count, size_t *p_alloc_size, size_t min_size, double load_factor);

// Calculate required entry count and internal allocation size to fit the current entries.
int rhset_shrink(const rhset *set, size_t *p_count, size_t *p_alloc_size, size_t min_size, double load_factor);

// Rehash the set contents, use `rhset_grow()` or `rhset_shrink()` to calculate `count` and `alloc_size`.
// Pass in a new internal data pointer of `alloc_size` bytes, returns the old data pointer.
// eg. `free(rhset_rehash(set, count, alloc_size, malloc(alloc_size))`.
// `data_ptr` must be aligned to the alignof(uint32_t), 8 bytes is safe
void *rhset_rehash(rhset *set, size_t count, size_t alloc_size, void *data_ptr);

// Iterate through all the entries that match `hash`. Returns 1 while there are matching entries, otherwise 0.
// eg. `while (rhset_find(set, hash, &scan)) { ... }`
int rhset_find(const rhset *set, uint32_t hash, uint32_t *p_scan);

// Insert a new entry at the iterator `hash + scan`. Use for example `rhset_find()` to find the place to
// insert to. If you want to unconditionally insert an entry to the set you can call `rhset_insert()` directly
// with a hash and `scan = 0`.
// NOTE: Requires `set->size < set->capacity`!
void rhset_insert(rhset *set, uint32_t hash, uint32_t scan);

// Iterate through all the entries in the set. Set `p_hash` and `p_scan` to zero to start from the beginning.
int rhset_next(const rhset *set, uint32_t *p_hash, uint32_t *p_scan);

// Remove a found entry from the set. `p_scan` must have been returned from `rhset_find()` or `rhset_next()`.
void rhset_remove(rhset *set, uint32_t hash, uint32_t scan);

#ifdef __cplusplus
	}
#endif

#endif

#if (defined(RHSET_IMPLEMENTATION) && !defined(RHSET_H_IMPLEMENTED)) || (defined(RHSET_INLINE) && !defined(RHSET_H_INLINED))

#if defined(RHSET_INLINE) && !defined(RHSET_H_INLINED)
	#define RHSET_H_INLINED
	#define RHSET_DO_INLINE RHSET_INLINE
#else
	#define RHSET_H_IMPLEMENTED
#endif

#ifndef RHSET_DEFAULT_LOAD_FACTOR
	#define RHSET_DEFAULT_LOAD_FACTOR 0.75
#endif

#ifdef RHSET_NO_STDLIB

	#ifndef RHSET_MEMSET
		static void rhset_imp_memset(void *data, int value, size_t num)
		{
			uint32_t *ptr = (uint32_t*)data, *end = ptr + num / 4;
			while (ptr != end) *ptr++ = 0;
		}
		#define RHSET_MEMSET rhset_imp_memset
	#endif

	#ifndef RHSET_ASSERT
		#define RHSET_ASSERT(cond) (void)0
	#endif

#else // RHSET_NO_STDLIB

	#ifndef RHSET_MEMSET
		#include <string.h>
		#define RHSET_MEMSET memset
	#endif

	#ifndef RHSET_ASSERT
		#include <assert.h>
		#define RHSET_ASSERT(cond) assert(cond)
	#endif

#endif // RHSET_NO_STDLIB

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef RHSET_DO_INLINE
RHSET_DO_INLINE void rhset_init_inline(rhset *set)
#else
void rhset_init(rhset *set)
#endif
{
	set->entries = 0;
	set->mask = set->capacity = set->size = 0;
}

#ifdef RHSET_DO_INLINE
RHSET_DO_INLINE void *rhset_reset_inline(rhset *set)
#else
void *rhset_reset(rhset *set)
#endif
{
	void *data = set->entries;
	set->entries = 0;
	set->mask = set->capacity = set->size = 0;
	return data;
}

#ifdef RHSET_DO_INLINE
RHSET_DO_INLINE void rhset_clear_inline(rhset *set)
#else
void rhset_clear(rhset *set)
#endif
{
	if (set->size > 0) {
		set->size = 0;
		RHSET_MEMSET(set->entries, 0, sizeof(uint32_t) * (set->mask + 1));
	}
}

#ifdef RHSET_DO_INLINE
RHSET_DO_INLINE size_t rhset_alloc_size_inline(const rhset *set)
#else
size_t rhset_alloc_size(const rhset *set)
#endif
{
	return set->mask ? (set->mask + 1) * sizeof(uint32_t) : 0;
}

#ifdef RHSET_DO_INLINE
RHSET_DO_INLINE void rhset_grow_inline(const rhset *set, size_t *p_count, size_t *p_alloc_size, size_t min_size, double load_factor)
#else
void rhset_grow(const rhset *set, size_t *p_count, size_t *p_alloc_size, size_t min_size, double load_factor)
#endif
{
	size_t num_entries, size;
	RHSET_ASSERT(load_factor < 1.0); /* Load factor must be either default (<= 0) or less than one */
	if (load_factor <= 0.0) load_factor = RHSET_DEFAULT_LOAD_FACTOR;
	num_entries = set->mask + 1;
	size = (size_t)((double)num_entries * load_factor);
	if (min_size < set->capacity + 1) min_size = set->capacity + 1;
	while (size < min_size) {
		num_entries *= 2;
		size = (size_t)((double)num_entries * load_factor);
	}
	*p_count = size;
	*p_alloc_size = num_entries * sizeof(uint32_t);
}

#ifdef RHSET_DO_INLINE
RHSET_DO_INLINE int rhset_shrink_inline(const rhset *set, size_t *p_count, size_t *p_alloc_size, size_t min_size, double load_factor)
#else
int rhset_shrink(const rhset *set, size_t *p_count, size_t *p_alloc_size, size_t min_size, double load_factor)
#endif
{
	size_t num_entries, size;
	RHSET_ASSERT(load_factor < 1.0); /* Load factor must be either default (<= 0) or less than one */
	if (load_factor <= 0.0) load_factor = RHSET_DEFAULT_LOAD_FACTOR;
	num_entries = 2;
	size = (size_t)((double)num_entries * load_factor);
	if (min_size < set->size) min_size = set->size;
	while (size < min_size) {
		num_entries *= 2;
		size = (size_t)((double)num_entries * load_factor);
	}
	*p_count = size;
	*p_alloc_size = num_entries * sizeof(uint32_t);
	return num_entries != set->mask + 1;
}

#ifdef RHSET_DO_INLINE
RHSET_DO_INLINE void *rhset_rehash_inline(rhset *set, size_t count, size_t alloc_size, void *data_ptr)
#else
void *rhset_rehash(rhset *set, size_t count, size_t alloc_size, void *data_ptr)
#endif
{
	size_t num_entries = alloc_size / sizeof(uint32_t);
	uint32_t *old_entries = set->entries;
	uint32_t *entries = (uint32_t*)data_ptr;
	uint32_t old_mask = set->mask;
	uint32_t mask = (uint32_t)(num_entries) - 1;
	set->entries = entries;
	set->mask = mask;
	set->capacity = (uint32_t)count;
	RHSET_ASSERT(data_ptr); /* You must pass a non-NULL pointer to internal storage */
	RHSET_MEMSET(entries, 0, sizeof(uint32_t) * num_entries);
	if (old_mask) {
		uint32_t i;
		for (i = 0; i <= old_mask; i++) {
			uint32_t entry, new_entry = old_entries[i];
			if (new_entry) {
				uint32_t old_scan = (uint32_t)(new_entry & old_mask) - 1;
				uint32_t hash = ((uint32_t)new_entry & ~old_mask) | ((i - old_scan) & old_mask);
				uint32_t slot = hash & mask;
				new_entry &= ~(uint32_t)mask;
				uint32_t scan = 1;
				while ((entry = entries[slot]) != 0) {
					uint32_t entry_scan = (entry & mask);
					if (entry_scan < scan) {
						entries[slot] = new_entry + scan;
						new_entry = (entry & ~(uint32_t)mask);
						scan = entry_scan;
					}
					scan += 1;
					slot = (slot + 1) & mask;
				}
				entries[slot] = new_entry + scan;
			}
		}
	}
	return old_entries;
}

#ifdef RHSET_DO_INLINE
RHSET_DO_INLINE int rhset_find_inline(const rhset *set, uint32_t hash, uint32_t *p_scan)
#else
int rhset_find(const rhset *set, uint32_t hash, uint32_t *p_scan)
#endif
{
	uint32_t mask = set->mask, scan = *p_scan;
	const uint32_t *entries = set->entries;
	uint32_t ref = hash & ~mask;
	if (!mask) return 0;
	for (;;) {
		uint32_t entry = entries[(hash + scan) & mask];
		scan += 1;
		if ((uint32_t)entry == ref + scan) {
			*p_scan = scan;
			return 1;
		} else if ((entry & mask) < scan) {
			*p_scan = scan - 1;
			return 0;
		}
	}
}

#ifdef RHSET_DO_INLINE
RHSET_DO_INLINE void rhset_insert_inline(rhset *set, uint32_t hash, uint32_t scan)
#else
void rhset_insert(rhset *set, uint32_t hash, uint32_t scan)
#endif
{
	uint32_t *entries = set->entries;
	uint32_t mask = set->mask;
	uint32_t slot = (hash + scan) & mask;
	uint32_t entry, new_entry = hash & ~mask;
	RHSET_ASSERT(set->capacity > set->size); /* You must ensure space before calling `rhset_insert()` */
	scan += 1;
	while ((entry = entries[slot]) != 0) {
		uint32_t entry_scan = (entry & mask);
		if (entry_scan < scan) {
			entries[slot] = new_entry + scan;
			new_entry = (entry & ~(uint32_t)mask);
			scan = entry_scan;
		}
		scan += 1;
		slot = (slot + 1) & mask;
	}
	entries[slot] = new_entry + scan;
	set->size++;
}

#ifdef RHSET_DO_INLINE
RHSET_DO_INLINE int rhset_next_inline(const rhset *set, uint32_t *p_hash, uint32_t *p_scan)
#else
int rhset_next(const rhset *set, uint32_t *p_hash, uint32_t *p_scan)
#endif
{
	uint32_t mask = set->mask;
	const uint32_t *entries = set->entries;
	uint32_t scan = (*p_hash & mask) + *p_scan;
	if (!mask) return 0;
	while (scan != mask + 1) {
		uint32_t entry = entries[scan & mask];
		scan += 1;
		if (entry) {
			uint32_t ref_scan = (uint32_t)(entry & mask) - 1;
			uint32_t ref_hash = ((uint32_t)entry & ~mask) | ((scan - ref_scan - 1) & mask);
			*p_hash = ref_hash;
			*p_scan = ref_scan + 1;
			return 1;
		}
	}
	return 0;
}

#ifdef RHSET_DO_INLINE
RHSET_DO_INLINE void rhset_remove_inline(rhset *set, uint32_t hash, uint32_t scan)
#else
void rhset_remove(rhset *set, uint32_t hash, uint32_t scan)
#endif
{
	uint32_t *entries = set->entries;
	uint32_t mask = set->mask;
	uint32_t slot = (hash + scan - 1) & mask;
	RHSET_ASSERT(scan > 0); /* Must be called with a found entry */
	for (;;) {
		uint32_t next_slot = (slot + 1) & mask;
		uint32_t next_entry = entries[next_slot];
		uint32_t next_scan = (next_entry & mask);
		if (next_scan <= 1) break;
		entries[slot] = next_entry - 1;
		slot = next_slot;
	}
	entries[slot] = 0;
	set->size--;
}

#ifdef RHSET_DO_INLINE
	#undef RHSET_DO_INLINE
#endif

#ifdef __cplusplus
	}
#endif

#endif // (defined(RHSET_IMPLEMENTATION) && !defined(RHSET_H_IMPLEMENTED)) || (defined(RHSET_INLINE) && !defined(RHSET_H_INLINED))
