#ifndef RHMAP_H_INCLUDED
#define RHMAP_H_INCLUDED

/*
	------------------------------------------------------------------------------
	This software is available under 2 licenses -- choose whichever you prefer.
	------------------------------------------------------------------------------
	ALTERNATIVE A - MIT License
	Copyright (c) 2020 Samuli Raivio
	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to
	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
	of the Software, and to permit persons to whom the Software is furnished to do
	so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
	------------------------------------------------------------------------------
	ALTERNATIVE B - Public Domain (www.unlicense.org)
	This is free and unencumbered software released into the public domain.
	Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
	software, either in source code form or as a compiled binary, for any purpose,
	commercial or non-commercial, and by any means.
	In jurisdictions that recognize copyright laws, the author or authors of this
	software dedicate any and all copyright interest in the software to the public
	domain. We make this dedication for the benefit of the public at large and to
	the detriment of our heirs and successors. We intend this dedication to be an
	overt act of relinquishment in perpetuity of all present and future rights to
	this software under copyright law.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
	ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	----------------------------------------
*/

/*
	rhmap is a hash map associating `uint32_t` values to `uint32_t` hashes.
	A single hash value can have multiple value entries. The map can be directly
	used as an U32->U32 map or more generally via storing entry indices as values.

	Define `RHMAP_IMPLEMENTATION` before including this header in a single file
	to implement the functions.

		#define RHMAP_IMEPLENTATION
		#include "rhmap.h"

	Alternatively you can define `RHMAP_INLINE` as your preferred inline qualifiers
	to include `_inline()` variants of the functions. If you define both
	`RHMAP_IMPLEMENTATION` and `RHMAP_INLINE` you need to include "rhmap.h" twice.
	You can use `RHMAP_FORCEINLINE` for a default force-inline qualifier.

		#define RHMAP_INLINE static RHMAP_FORCEINLINE
		#include "rhmap.h"

	Initialize a map by simply zeroing it or call `rhmap_init()`.

		rhmap map = { 0 };
		rhmap map; rhmap_init(&map);

	The map doesn't manage it's own memory so you need to check for rehashing
	before inserting anything to the map. Use `rhmap_grow()` or `rhmap_shrink()`
	to calculate required internal data size and entry count.

		if (map.size == map.capacity) {
			size_t count, alloc_size;
			rhmap_grow(&map, &count, &alloc_size, 8, 0.0);
			void *new_data = malloc(alloc_size);
			void *old_data = rhmap_rehash(&map, count, alloc_size, new_data);
			free(old_data);
		}

	`alloc_size` is guaranteed to be aligned to 16 bytes so if you need additional
	entry payload it can be conveniently allocated with the internal data.

		// my_entry_t *my_entries;
		size_t count, alloc_size;
		rhmap_grow(&map, &count, &alloc_size, 8, 0.0);
		char *new_data = (char*)malloc(alloc_size + count * sizeof(my_entry_t));
		my_entry_t *new_entries = (my_entry_t*)(new_data + alloc_size);
		memcpy(new_entries, my_entries, map.size * sizeof(my_entry_t));
		my_entries = new_entries;
		void *old_data = rhmap_rehash(&map, count, alloc_size, new_data);
		free(old_data);

	To free the internal data you should call `rhmap_reset()` which returns the
	pointer previously passed to `rhmap_rehash()` and resets the map to the
	initial zero state.

		void *old_data = rhmap_reset(&map);
		free(old_data);

	Map entries are indexed using `hash + scan` values. `hash` should be a well
	distributed hash of a key. `scan` is an internal offset that should be
	initialized to zero. `rhmap_find()` iterates through all the entries with
	a matching hash. `rhmap_insert()` inserts a new value at the current position.

		// Try to find the entry from the map
		uint32_t hash = hash_key(&key), scan = 0, index;
		while (rhmap_find(&map, hash, &scan, &index)) {
			if (my_entries[index].key == key) {
				return &my_entries[index];
			}
		}

		// Insert to the end of `my_entries` array
		index = map.size;
		my_entries[index].key = key;
		rhmap_insert(&map, hash, scan, index);

	Removing values works similarly: use `rhmap_find()` or `rhmap_next()` to find
	and entry to remove. If you are using a compact auxilary entry array you can
	swap the last entry and the removed one and use `rhmap_update_value()` to
	"rename" the index of the swapped entry.

		// Find the entry from the map
		uint32_t hash = hash_key(&key), scan = 0, index;
		while (rhmap_find(&map, hash, &scan, &index)) {
			if (my_entries[index].key == key) break;
		}

		rhmap_remove(&map, hash, scan);
		if (index < map.size) {
			uint32_t swap_hash = hash_key(&my_entries[map.size].key);
			rhmap_update_value(&map, swap_hash, map.size, index);
			my_entries[index] = my_entries[map.size];
		}

	To iterate through all the entries in the map you can use `rhmap_next()`.
	Setting `hash` and `scan` to zero will start iteration from the first entry.

		uint32_t hash = 0, scan = 0, index;
		while (rhmap_next(&map, &hash, &scan, &index)) {
			// ...
		}

	You can also provide some defines to customize the behavior:

		RHMAP_MEMSET(data, value, size): always called with `value=0` and `size % 8 == 0`
		default: memset()

		RHMAP_ASSERT(cond), default: assert(cond) from <assert.h>

		RHMAP_DEFAULT_LOAD_FACTOR: Load factor used if the parameter is <= 0.0.
		default: 0.75

	rhmap depends on parts of the C standard library, these can be disabled via macros:

		RHMAP_NO_STDLIB: Use built-in memset() and no-op assert() (unless provided)

		RHMAP_NO_STDINT: Don't include <stdint.h> or <stddef.h>, you _must_ provide
		definitions to `uint32_t`, `uint64_t`, `size_t`

*/

#ifndef RHMAP_NO_STDINT
	#include <stddef.h>
	#include <stdint.h>
#endif

#ifndef RHMAP_FORCEINLINE
	#if defined(_MSC_VER)
		#define RHMAP_FORCEINLINE __forceinline
	#elif defined(__GNUC__)
		#define RHMAP_FORCEINLINE __attribute__((always_inline, unused))
	#else
		#define RHMAP_FORCEINLINE
	#endif
#endif

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct rhmap {

	// Internal state
	uint64_t *entries;
	uint32_t mask;

	// Maximum number of entries that fit in the map before needing to re-hash.
	uint32_t capacity;

	// Number of entries in the map
	uint32_t size;

} rhmap;

// Initialize the map to zero.
// Equivalent to `memset(map, 0, sizeof(rhmap))`.
void rhmap_init(rhmap *map);

// Free the map data and reset to zero.
// Returns pointer to free, eg. `free(rhmap_reset(map))`.
// If you don't free the pointer you can use this to move the map to another struct
void *rhmap_reset(rhmap *map);

// Remove all entries from the map without freeing the internal storage.
void rhmap_clear(rhmap *map);

// Retrieve the size of the current internal data pointer
size_t rhmap_alloc_size(const rhmap *map);

// Calculate required entry count and internal allocation size to fit at least one more entry to the map.
void rhmap_grow(const rhmap *map, size_t *p_count, size_t *p_alloc_size, size_t min_size, double load_factor);

// Calculate required entry count and internal allocation size to fit the current entries.
int rhmap_shrink(const rhmap *map, size_t *p_count, size_t *p_alloc_size, size_t min_size, double load_factor);

// Rehash the map contents, use `rhmap_grow()` or `rhmap_shrink()` to calculate `count` and `alloc_size`.
// Pass in a new internal data pointer of `alloc_size` bytes, returns the old data pointer.
// eg. `free(rhmap_rehash(map, count, alloc_size, malloc(alloc_size))`.
// `data_ptr` must be aligned to the alignof(uint64_t), 8 bytes is safe
void *rhmap_rehash(rhmap *map, size_t count, size_t alloc_size, void *data_ptr);

// Iterate through all the entries that match `hash`. Returns 1 while there are matching entries, otherwise 0.
// eg. `while (rhmap_find(map, hash, &scan, &value)) { ... }`
int rhmap_find(const rhmap *map, uint32_t hash, uint32_t *p_scan, uint32_t *p_value);

// Insert a new entry at the iterator `hash + scan`. Use for example `rhmap_find()` to find the place to
// insert to. If you want to unconditionally insert an entry to the map you can call `rhmap_insert()` directly
// with a hash and `scan = 0`.
// NOTE: Requires `map->size < map->capacity`!
void rhmap_insert(rhmap *map, uint32_t hash, uint32_t scan, uint32_t value);

// Iterate through all the entries in the map. Set `p_hash` and `p_scan` to zero to start from the beginning.
int rhmap_next(const rhmap *map, uint32_t *p_hash, uint32_t *p_scan, uint32_t *p_value);

// Update the value of a found entry. `scan` must have been returned from `rhmap_find()` or `rhmap_next()`.
void rhmap_set(rhmap *map, uint32_t hash, uint32_t scan, uint32_t value);

// Remove a found entry from the map. `p_scan` must have been returned from `rhmap_find()` or `rhmap_next()`.
void rhmap_remove(rhmap *map, uint32_t hash, uint32_t scan);

// Optimized utility function to "rename" an existing value.
// NOTE: An entry with `hash` and `old_value` _must_ exist in the map!
// Semantically equivalent to:
// void rhmap_update_value(rhmap *map, uint32_t hash, uint32_t old_value, uint32_t new_value) {
//     uint32_t scan = 0, value;
//     while (rhmap_find(map, hash, &scan, &value)) {
//         if (value == old_value) {
//             rhmap_set(map, hash, scan, new_value);
//             return;
//         }
//     }
//     RHMAP_ASSERT(0);
// }
void rhmap_update_value(rhmap *map, uint32_t hash, uint32_t old_value, uint32_t new_value);

// Optimized utility function to find an entry with a specific value.
// NOTE: An entry with `hash` and `value` _must_ exist in the map!
// Semantically equivalent to:
// void rhmap_find_value(const rhmap *map, uint32_t hash, uint32_t *p_scan, uint32_t value) {
//     uint32_t ref_value;
//     while (rhmap_find(map, hash, &p_scan, &ref_value)) {
//         if (ref_value == value) return;
//     }
//     RHMAP_ASSERT(0);
// }
void rhmap_find_value(const rhmap *map, uint32_t hash, uint32_t *p_scan, uint32_t value);

#ifdef __cplusplus
	}
#endif

#endif

#if (defined(RHMAP_IMPLEMENTATION) && !defined(RHMAP_H_IMPLEMENTED)) || (defined(RHMAP_INLINE) && !defined(RHMAP_H_INLINED))

#if defined(RHMAP_INLINE) && !defined(RHMAP_H_INLINED)
	#define RHMAP_H_INLINED
	#define RHMAP_DO_INLINE RHMAP_INLINE
#else
	#define RHMAP_H_IMPLEMENTED
#endif

#ifndef RHMAP_DEFAULT_LOAD_FACTOR
	#define RHMAP_DEFAULT_LOAD_FACTOR 0.75
#endif

#ifdef RHMAP_NO_STDLIB

	#ifndef RHMAP_MEMSET
		static void rhmap_imp_memset(void *data, int value, size_t num)
		{
			uint64_t *ptr = (uint64_t*)data, *end = ptr + num / 8;
			while (ptr != end) *ptr++ = 0;
		}
		#define RHMAP_MEMSET rhmap_imp_memset
	#endif

	#ifndef RHMAP_ASSERT
		#define RHMAP_ASSERT(cond) (void)0
	#endif

#else // RHMAP_NO_STDLIB

	#ifndef RHMAP_MEMSET
		#include <string.h>
		#define RHMAP_MEMSET memset
	#endif

	#ifndef RHMAP_ASSERT
		#include <assert.h>
		#define RHMAP_ASSERT(cond) assert(cond)
	#endif

#endif // RHMAP_NO_STDLIB

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE void rhmap_init_inline(rhmap *map)
#else
void rhmap_init(rhmap *map)
#endif
{
	map->entries = 0;
	map->mask = map->capacity = map->size = 0;
}

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE void *rhmap_reset_inline(rhmap *map)
#else
void *rhmap_reset(rhmap *map)
#endif
{
	void *data = map->entries;
	map->entries = 0;
	map->mask = map->capacity = map->size = 0;
	return data;
}

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE void rhmap_clear_inline(rhmap *map)
#else
void rhmap_clear(rhmap *map)
#endif
{
	if (map->size > 0) {
		map->size = 0;
		RHMAP_MEMSET(map->entries, 0, sizeof(uint64_t) * (map->mask + 1));
	}
}

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE size_t rhmap_alloc_size_inline(const rhmap *map)
#else
size_t rhmap_alloc_size(const rhmap *map)
#endif
{
	return map->mask ? (map->mask + 1) * sizeof(uint64_t) : 0;
}

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE void rhmap_grow_inline(const rhmap *map, size_t *p_count, size_t *p_alloc_size, size_t min_size, double load_factor)
#else
void rhmap_grow(const rhmap *map, size_t *p_count, size_t *p_alloc_size, size_t min_size, double load_factor)
#endif
{
	size_t num_entries, size;
	RHMAP_ASSERT(load_factor < 1.0); /* Load factor must be either default (<= 0) or less than one */
	if (load_factor <= 0.0) load_factor = RHMAP_DEFAULT_LOAD_FACTOR;
	num_entries = map->mask + 1;
	size = (size_t)((double)num_entries * load_factor);
	if (min_size < map->capacity + 1) min_size = map->capacity + 1;
	while (size < min_size) {
		num_entries *= 2;
		size = (size_t)((double)num_entries * load_factor);
	}
	*p_count = size;
	*p_alloc_size = num_entries * sizeof(uint64_t);
}

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE int rhmap_shrink_inline(const rhmap *map, size_t *p_count, size_t *p_alloc_size, size_t min_size, double load_factor)
#else
int rhmap_shrink(const rhmap *map, size_t *p_count, size_t *p_alloc_size, size_t min_size, double load_factor)
#endif
{
	size_t num_entries, size;
	RHMAP_ASSERT(load_factor < 1.0); /* Load factor must be either default (<= 0) or less than one */
	if (load_factor <= 0.0) load_factor = RHMAP_DEFAULT_LOAD_FACTOR;
	num_entries = 2;
	size = (size_t)((double)num_entries * load_factor);
	if (min_size < map->size) min_size = map->size;
	while (size < min_size) {
		num_entries *= 2;
		size = (size_t)((double)num_entries * load_factor);
	}
	*p_count = size;
	*p_alloc_size = num_entries * sizeof(uint64_t);
	return num_entries != map->mask + 1;
}

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE void *rhmap_rehash_inline(rhmap *map, size_t count, size_t alloc_size, void *data_ptr)
#else
void *rhmap_rehash(rhmap *map, size_t count, size_t alloc_size, void *data_ptr)
#endif
{
	size_t num_entries = alloc_size / sizeof(uint64_t);
	uint64_t *old_entries = map->entries;
	uint64_t *entries = (uint64_t*)data_ptr;
	uint32_t old_mask = map->mask;
	uint32_t mask = (uint32_t)(num_entries) - 1;
	map->entries = entries;
	map->mask = mask;
	map->capacity = (uint32_t)count;
	RHMAP_ASSERT(data_ptr); /* You must pass a non-NULL pointer to internal storage */
	RHMAP_MEMSET(entries, 0, sizeof(uint64_t) * num_entries);
	if (old_mask) {
		uint32_t i;
		for (i = 0; i <= old_mask; i++) {
			uint64_t entry, new_entry = old_entries[i];
			if (new_entry) {
				uint32_t old_scan = (uint32_t)(new_entry & old_mask) - 1;
				uint32_t hash = ((uint32_t)new_entry & ~old_mask) | ((i - old_scan) & old_mask);
				uint32_t slot = hash & mask;
				new_entry &= ~(uint64_t)mask;
				uint32_t scan = 1;
				while ((entry = entries[slot]) != 0) {
					uint32_t entry_scan = (entry & mask);
					if (entry_scan < scan) {
						entries[slot] = new_entry + scan;
						new_entry = (entry & ~(uint64_t)mask);
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

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE int rhmap_find_inline(const rhmap *map, uint32_t hash, uint32_t *p_scan, uint32_t *p_value)
#else
int rhmap_find(const rhmap *map, uint32_t hash, uint32_t *p_scan, uint32_t *p_value)
#endif
{
	uint32_t mask = map->mask, scan = *p_scan;
	const uint64_t *entries = map->entries;
	uint32_t ref = hash & ~mask;
	if (!mask) return 0;
	for (;;) {
		uint64_t entry = entries[(hash + scan) & mask];
		scan += 1;
		if ((uint32_t)entry == ref + scan) {
			*p_scan = scan;
			*p_value = (uint32_t)(entry >> 32u);
			return 1;
		} else if ((entry & mask) < scan) {
			*p_scan = scan - 1;
			return 0;
		}
	}
}

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE void rhmap_insert_inline(rhmap *map, uint32_t hash, uint32_t scan, uint32_t value)
#else
void rhmap_insert(rhmap *map, uint32_t hash, uint32_t scan, uint32_t value)
#endif
{
	uint64_t *entries = map->entries;
	uint32_t mask = map->mask;
	uint32_t slot = (hash + scan) & mask;
	uint64_t entry, new_entry = (uint64_t)value << 32u | (hash & ~mask);
	RHMAP_ASSERT(map->capacity > map->size); /* You must ensure space before calling `rhmap_insert()` */
	scan += 1;
	while ((entry = entries[slot]) != 0) {
		uint32_t entry_scan = (entry & mask);
		if (entry_scan < scan) {
			entries[slot] = new_entry + scan;
			new_entry = (entry & ~(uint64_t)mask);
			scan = entry_scan;
		}
		scan += 1;
		slot = (slot + 1) & mask;
	}
	entries[slot] = new_entry + scan;
	map->size++;
}

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE int rhmap_next_inline(const rhmap *map, uint32_t *p_hash, uint32_t *p_scan, uint32_t *p_value)
#else
int rhmap_next(const rhmap *map, uint32_t *p_hash, uint32_t *p_scan, uint32_t *p_value)
#endif
{
	uint32_t mask = map->mask;
	const uint64_t *entries = map->entries;
	uint32_t scan = (*p_hash & mask) + *p_scan;
	if (!mask) return 0;
	while (scan != mask + 1) {
		uint64_t entry = entries[scan & mask];
		scan += 1;
		if (entry) {
			uint32_t ref_scan = (uint32_t)(entry & mask) - 1;
			uint32_t ref_hash = ((uint32_t)entry & ~mask) | ((scan - ref_scan - 1) & mask);
			*p_hash = ref_hash;
			*p_scan = ref_scan + 1;
			*p_value = (uint64_t)entry >> 32u;
			return 1;
		}
	}
	return 0;
}

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE void rhmap_set_inline(rhmap *map, uint32_t hash, uint32_t scan, uint32_t value)
#else
void rhmap_set(rhmap *map, uint32_t hash, uint32_t scan, uint32_t value)
#endif
{
	uint32_t mask = map->mask;
	uint32_t slot = (hash + scan - 1) & mask;
	uint64_t *entries = map->entries;
	RHMAP_ASSERT(scan > 0); /* Must be called with a found entry */
	entries[slot] = (entries[slot] & 0xffffffffu) | (uint64_t)value << 32u;
}

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE void rhmap_remove_inline(rhmap *map, uint32_t hash, uint32_t scan)
#else
void rhmap_remove(rhmap *map, uint32_t hash, uint32_t scan)
#endif
{
	uint64_t *entries = map->entries;
	uint32_t mask = map->mask;
	uint32_t slot = (hash + scan - 1) & mask;
	RHMAP_ASSERT(scan > 0); /* Must be called with a found entry */
	for (;;) {
		uint32_t next_slot = (slot + 1) & mask;
		uint64_t next_entry = entries[next_slot];
		uint32_t next_scan = (next_entry & mask);
		if (next_scan <= 1) break;
		entries[slot] = next_entry - 1;
		slot = next_slot;
	}
	entries[slot] = 0;
	map->size--;
}

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE void rhmap_update_value_inline(rhmap *map, uint32_t hash, uint32_t old_value, uint32_t new_value)
#else
void rhmap_update_value(rhmap *map, uint32_t hash, uint32_t old_value, uint32_t new_value)
#endif
{
	uint64_t *entries = map->entries;
	uint32_t mask = map->mask, scan = 0;
	uint64_t old_entry = (uint64_t)old_value << 32u | (hash & ~mask);
	uint64_t new_entry = (uint64_t)new_value << 32u | (hash & ~mask);
	for (;;) {
		uint32_t slot = (hash + scan) & mask;
		scan += 1;
		if (entries[slot] == old_entry + scan) {
			entries[slot] = new_entry + scan;
			return;
		}
		RHMAP_ASSERT((entries[slot] & mask) >= scan);
	}
}

#ifdef RHMAP_DO_INLINE
RHMAP_DO_INLINE void rhmap_find_value_inline(const rhmap *map, uint32_t hash, uint32_t *p_scan, uint32_t value)
#else
void rhmap_find_value(const rhmap *map, uint32_t hash, uint32_t *p_scan, uint32_t value)
#endif
{
	uint64_t *entries = map->entries;
	uint32_t mask = map->mask, scan = *p_scan;
	uint64_t entry = (uint64_t)value << 32u | (hash & ~mask);
	for (;;) {
		uint32_t slot = (hash + scan) & mask;
		scan += 1;
		if (entries[slot] == entry + scan) {
			*p_scan = scan;
			return;
		}
		RHMAP_ASSERT((entries[slot] & mask) >= scan);
	}
}

#ifdef RHMAP_DO_INLINE
	#undef RHMAP_DO_INLINE
#endif

#ifdef __cplusplus
	}
#endif

#endif // (defined(RHMAP_IMPLEMENTATION) && !defined(RHMAP_H_IMPLEMENTED)) || (defined(RHMAP_INLINE) && !defined(RHMAP_H_INLINED))