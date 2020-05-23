#include "json_input.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <limits.h>

#if defined(_MSC_VER)
	#pragma warning(push)
	#pragma warning(disable: 4200)
#endif

#ifdef _MSC_VER
	#define jsi_forceinline __forceinline
	#define jsi_noinline __declspec(noinline)
	#define jsi_likely(x) (x)
#else
	#define jsi_forceinline __attribute__((always_inline)) inline
	#define jsi_noinline __attribute__((noinline))
	#define jsi_likely(x) __builtin_expect((x), 1)
#endif

#if !defined(JSI_USE_SSE) && !defined(JSI_NO_SIMD)
	#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
		#include <xmmintrin.h>
		#include <emmintrin.h>
		#include <intrin.h>
		#define JSI_SSE_CTZ(dst, mask) _BitScanForward((unsigned long*)&(dst), (mask))
		#define JSI_USE_SSE 1
	#endif
#endif

#ifndef JSI_USE_SSE
#define JSI_USE_SSE 0
#endif

#define JSI_DEPTH_IMPLICIT_ROOT (-1)

typedef struct {

	const char *data, *ptr, *end;
	size_t data_offset;

	jsi_refill_fn refill_fn;
	void *refill_user;

	jsi_args *args;
	jsi_dialect dialect;
	int depth_left;
	int max_depth;

	char *temp_stack;
	size_t temp_top, temp_size;

	char *result_page;
	size_t result_pos, result_size;
	size_t result_min_alloc_size;

	size_t newline_offset;
	size_t line_index;

	unsigned temp_allocated : 1;
	unsigned result_allocated : 1;

} jsi_parser;

typedef struct {
	jsi_value value;
	void *memory;

	jsi_free_fn free_fn;
	void *free_user;
} jsi_result_value;

struct jsi_obj_map {
	uint32_t mask;
	uint32_t entries[];
};

typedef struct {
	char *begin, *ptr, *end;
} jsi_copy_range;

typedef int (*jsi_parse_fn)(jsi_parser *p, const char *ptr, const char *end, jsi_value *value);

typedef enum {
	jsi_char_whitespace,
	jsi_char_error,
	jsi_char_object,
	jsi_char_array,
	jsi_char_string,
	jsi_char_number,
	jsi_char_true,
	jsi_char_false,
	jsi_char_null,
} jsi_char_type;

static const char jsi_char_tab[256] = {
	1,1,1,1,1,1,1,1,1,0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,4,1,1,1,1,1,1,1,1,5,1,5,5,0,5,5,5,5,5,5,5,5,5,5,1,1,1,1,1,1,
	1,1,1,1,1,5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,1,1,1,
	1,1,1,1,1,5,7,1,1,1,1,1,1,1,8,1,1,1,1,1,6,1,1,1,1,1,1,2,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

static const char jsi_null_buffer[4] = { 0 };

static const jsi_obj null_obj = { 0 };
static const jsi_arr null_arr = { 0 };

#define jsi_advance(p_p, p_ptr, p_end) \
	if (jsi_likely(++p_ptr != p_end)) { } else { \
		jsi_refill(p_p); p_ptr = p_p->ptr; p_end = p_p->end; }

jsi_forceinline static void *
jsi_mem_alloc(jsi_allocator *a, size_t size)
{
	if (a->memory_limit && a->memory_used + size > a->memory_limit) {
		return NULL;
	}
	a->memory_used += size;

	if (a->alloc_fn) {
		return a->alloc_fn(a->user, size);
	} else {
		return malloc(size);
	}
}

jsi_forceinline static void *
jsi_mem_realloc(jsi_allocator *a, void *ptr, size_t new_size, size_t old_size)
{
	if (a->memory_limit && a->memory_used - old_size + new_size > a->memory_limit) {
		return NULL;
	}
	a->memory_used = a->memory_used - old_size + new_size;

	if (a->realloc_fn) {
		return a->realloc_fn(a->user, ptr, new_size, old_size);
	} else if (a->alloc_fn) {
		void *new_ptr = a->alloc_fn(a->user, new_size);
		if (new_ptr) {
			memcpy(new_ptr, ptr, old_size);
			if (a->free_fn) {
				a->free_fn(a->user, ptr, old_size);
			}
		}
		return new_ptr;
	} else {
		return realloc(ptr, new_size);
	}
}

jsi_forceinline static void
jsi_mem_free(jsi_allocator *a, void *ptr, size_t size)
{
	a->memory_used -= size;

	if (ptr) {
		if (a->alloc_fn) {
			if (a->free_fn) {
				a->free_fn(a->user, ptr, size);
			}
		} else {
			free(ptr);
		}
	}
}

jsi_forceinline static uint32_t
jsi_key_hash(const char *key, size_t len)
{
	const char *end = key + len;
	uint32_t hash = 2166136261u;
	while (key != end) {
		hash = (hash ^ *key++) * 16777619u;
	}
	return hash;
}

jsi_noinline static void
jsi_refill(jsi_parser *p)
{
	p->data_offset += p->end - p->data;

	size_t size = 0;
	if (p->refill_fn) {
		p->data = (const char*)p->refill_fn(p->refill_user, &size);
	}
	if (p->data && size > 0) {
		p->ptr = p->data;
		p->end = p->data + size;
	} else {
		p->data = p->ptr = jsi_null_buffer;
		p->end = p->ptr + sizeof(jsi_null_buffer);
	}
}

jsi_noinline static int
jsi_err_at(jsi_parser *p, size_t offset, const char *description)
{
	p->args->error.byte_offset = offset;
	p->args->error.description = description;
	if (offset >= p->newline_offset) {
		p->args->error.line = p->line_index;
		p->args->error.column = offset - p->newline_offset;
	} else {
		p->args->error.line = p->line_index - 1;
		p->args->error.column = 0;
	}
	return 0;
}

jsi_noinline static int
jsi_err_offset(jsi_parser *p, const char *ptr, int off, const char *description)
{
	size_t offset = p->data_offset + (ptr - p->data) + off;
	return jsi_err_at(p, offset, description);
}

jsi_noinline static int
jsi_err(jsi_parser *p, const char *ptr, const char *description)
{
	size_t offset = p->data_offset + (ptr - p->data);
	return jsi_err_at(p, offset, description);
}

jsi_noinline int
jsi_skip_comment(jsi_parser *p, const char *ptr, const char *end)
{
	size_t begin_offset = p->data_offset + ptr - p->data;
	if (p->dialect.allow_comments) {
		jsi_advance(p, ptr, end);
		char c = *ptr;
		jsi_advance(p, ptr, end);
		if (c == '/') {
			while (*ptr != '\n' && *ptr != '\0') {
				jsi_advance(p, ptr, end);
			}
		} else if (c == '*') {
			for (;;) {
				char c = *ptr;
				if (c == '\0') return jsi_err_at(p, begin_offset, "Unclosed block comment");
				jsi_advance(p, ptr, end);
				if (c == '*' && *ptr == '/') break;
			}
			jsi_advance(p, ptr, end);
		} else {
			return jsi_err(p, ptr, "Invalid character");
		}
	} else {
		return jsi_err(p, ptr, "Comments are not allowed");
	}

	p->ptr = ptr;
	return 1;
}

jsi_noinline int
jsi_skip_whitespace(jsi_parser *p, const char *ptr, const char *end)
{
	// Optimistic parsing for the most common whitespace pattern:
	// Possible newline followed by 0-16 spaces or tabs for indentation
	#if JSI_USE_SSE
	__m128i sse_space = _mm_set1_epi8(' ');
	__m128i sse_tab = _mm_set1_epi8('\t');
	#endif
	if (*ptr == '\r') {
		jsi_advance(p, ptr, end);
	}
	if (*ptr == '\n') {
		p->newline_offset = p->data_offset + (ptr - p->data);
		p->line_index++;
		jsi_advance(p, ptr, end);
	}
	#if JSI_USE_SSE
	if (end - ptr > 16)
	{
		__m128i chars = _mm_loadu_si128((const __m128i*)ptr);
		__m128i is_space = _mm_cmpeq_epi8(chars, sse_space);
		__m128i is_tab = _mm_cmpeq_epi8(chars, sse_tab);
		__m128i is_whitespace = _mm_or_si128(is_space, is_tab);
		int mask = _mm_movemask_epi8(is_whitespace);
		uint32_t skip;
		JSI_SSE_CTZ(skip, ~mask);
		skip = mask ? skip : 0;
		ptr += skip;
	}
	#endif

	while (jsi_char_tab[*ptr] == jsi_char_whitespace) {
		if (*ptr == '\n') {
			p->newline_offset = p->data_offset + (ptr - p->data);
			p->line_index++;
			jsi_advance(p, ptr, end);
		} else if (*ptr == '/') {
			if (!jsi_skip_comment(p, ptr, end)) return 0;
			ptr = p->ptr; end = p->end;
		} else {
			jsi_advance(p, ptr, end);
		}
	}
	p->ptr = ptr;
	return 1;
}

jsi_noinline int
jsi_match(jsi_parser *p, const char *ptr, const char *end, const char *str)
{
	while (*str) {
		if (*ptr != *str++) return 0;
		jsi_advance(p, ptr, end);
	}
	p->ptr = ptr;
	return 1;
}

jsi_noinline static void*
jsi_result_grow(jsi_parser *p, size_t size)
{
	jsi_args *args = p->args;
	if (args->no_allocation) {
		jsi_err(p, p->ptr, "Out of pre-allocated memory");
		return NULL;
	}

	p->result_min_alloc_size *= 2;
	if (size + 16 > p->result_min_alloc_size) {
		size_t new_size = size + 16;
		char *new_page = (char*)jsi_mem_alloc(&p->args->result_allocator, new_size);
		if (!new_page) {
			jsi_err(p, p->ptr, "Failed to allocate memory");
			return NULL;
		}
		*(size_t*)(new_page + 8) = new_size;
		if (p->result_allocated) {
			*(void**)new_page = *(void**)p->result_page;
			*(void**)p->result_page = new_page;
		} else {
			p->result_page = new_page;
			p->result_size = new_size;
			p->result_pos = new_size;
			p->result_allocated = 1;
			*(void**)new_page = NULL;
		}
		return new_page + 16;
	} else {
		size_t new_size = p->result_min_alloc_size;
		char *new_page = (char*)jsi_mem_alloc(&p->args->result_allocator, new_size);
		if (!new_page) {
			jsi_err(p, p->ptr, "Failed to allocate memory");
			return NULL;
		}
		*(void**)new_page = p->result_allocated ? p->result_page : NULL;
		*(size_t*)(new_page + 8) = p->result_min_alloc_size;
		p->result_page = new_page;
		p->result_size = new_size;
		p->result_pos = 16 + size;
		p->result_allocated = 1;
		return new_page + 16;
	}
}

jsi_noinline static int
jsi_result_grow_copy(jsi_parser *p, jsi_copy_range *range)
{
	jsi_args *args = p->args;
	if (args->no_allocation) {
		return jsi_err(p, p->ptr, "Out of pre-allocated memory");
	}
	size_t total_size = range->end - range->begin;
	size_t data_size = range->ptr - range->begin;
	size_t new_size = total_size * 2 + 8 + 16;
	if (range->begin == p->result_page + 16 && p->result_allocated) {
		// String to copy takes the whole page, safe to reallocate
		char *new_page = (char*)jsi_mem_realloc(&p->args->result_allocator,
			p->result_page, new_size, p->result_size);
		if (!new_page) {
			return jsi_err(p, p->ptr, "Failed to allocate memory");
		}
		*(size_t*)(new_page + 8) = new_size;
		p->result_page = new_page;
		p->result_size = new_size;
		p->result_pos = 16;
		range->begin = new_page + 16;
		range->ptr = new_page + 16 + data_size;
		range->end = new_page + new_size;
		return 1;
	} else {
		p->result_min_alloc_size *= 2;
		if (new_size < p->result_min_alloc_size) new_size = p->result_min_alloc_size;
		char *new_page = (char*)jsi_mem_alloc(&p->args->result_allocator, new_size);
		if (!new_page) {
			return jsi_err(p, p->ptr, "Failed to allocate memory");
		}
		memcpy(new_page + 16, range->begin, data_size);
		*(void**)new_page = p->result_allocated ? p->result_page : NULL;
		*(size_t*)(new_page + 8) = new_size;
		p->result_page = new_page;
		p->result_size = new_size;
		p->result_pos = 16;
		p->result_allocated = 1;
		range->begin = new_page + 16;
		range->ptr = new_page + 16 + data_size;
		range->end = new_page + new_size;
		return 1;
	}
	return 0;
}

jsi_forceinline static void*
jsi_push_result(jsi_parser *p, size_t size, size_t align)
{
	size_t pos = (p->result_pos + (align - 1)) & ~(align - 1);
	if (pos + size <= p->result_size) {
		p->result_pos = pos + size;
		return p->result_page + pos;
	} else {
		return jsi_result_grow(p, size);
	}
}

jsi_noinline static int
jsi_temp_grow(jsi_parser *p)
{
	size_t new_size = p->temp_size * 2;
	char *new_temp;
	jsi_args *args = p->args;
	if (args->no_allocation) {
		// No allocation allowed, try to fit temp buffer into result
		new_temp = (char*)jsi_push_result(p, new_size, 8);
		if (!new_temp) return 0;
		memcpy(new_temp, p->temp_stack, p->temp_size);
	} else if (p->temp_allocated) {
		// Heap -> Heap
		new_temp = (char*)jsi_mem_realloc(&args->temp_allocator, p->temp_stack, new_size, p->temp_size);
		if (!new_temp) {
			return jsi_err(p, p->ptr, "Failed to allocate memory");
		}
	} else {
		// Stack -> Heap
		new_temp = (char*)jsi_mem_alloc(&args->temp_allocator, new_size);
		if (!new_temp) {
			return jsi_err(p, p->ptr, "Failed to allocate memory");
		}
		memcpy(new_temp, p->temp_stack, p->temp_size);
		p->temp_allocated = 1;
	}
	p->temp_stack = new_temp;
	p->temp_size = new_size;
	return 1;
}

static const char jsi_stop_char_tab[256] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static const char jsi_escape_tab[256] =
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\\\0\0\0"
	"\0\0\b\0\0\0\f\0\0\0\0\0\0\0\n\0\0\0\r\0\t\0\0\0\0\0\0\0\0\0\0\0";

jsi_noinline static unsigned
jsi_read_utf16(jsi_parser *p, const char *ptr, const char *end)
{
	unsigned value = 0;
	for (int i = 0; i < 4; i++) {
		char c = *ptr;
		value <<= 4;
		if (c >= '0' && c <= '9') value |= c - '0';
		else if (c >= 'A' && c <= 'F') value |= c - 'A' + 10;
		else if (c >= 'a' && c <= 'f') value |= c - 'a' + 10;
		else {
			jsi_err(p, ptr, "Invalid Unicode hex digit");
			return ~0u;
		}
		jsi_advance(p, ptr, end);
	}
	p->ptr = ptr;
	return value;
}

jsi_noinline
char *jsi_write_utf8(char *buf, unsigned code)
{
	if (code <= 0x7f) {
		buf[0] = (unsigned char)code;
		return buf + 1;
	} else if (code <= 0x7ff) {
		buf[0] = (unsigned char)((code >> 6) | 0xc0);
		buf[1] = (unsigned char)((code & 0x3f) | 0x80);
		return buf + 2;
	} else if (code <= 0xffff) {
		buf[0] = (unsigned char)((code >> 12) | 0xe0);
		buf[1] = (unsigned char)(((code >> 6) & 0x3f) | 0x80);
		buf[2] = (unsigned char)((code & 0x3f) | 0x80);
		return buf + 3;
	} else if (code <= 0x10ffff) {
		buf[0] = (unsigned char)((code >> 18) | 0xf0);
		buf[1] = (unsigned char)(((code >> 12) & 0x3f) | 0x80);
		buf[2] = (unsigned char)(((code >> 6) & 0x3f) | 0x80);
		buf[3] = (unsigned char)((code & 0x3f) | 0x80);
		return buf + 4;
	} else {
		// This should never happen
		return buf;
	}
}

jsi_noinline char *
jsi_copy_utf16(jsi_parser *p, const char *ptr, const char *end, char *buf)
{
	unsigned hi = jsi_read_utf16(p, ptr, end);
	ptr = p->ptr; end = p->end;
	if (hi == ~0u) return NULL;
	if (hi >= 0xd800 && hi <= 0xdbff) {
		// High surrogate, try to combine with next escape
		if (*ptr == '\\') {
			jsi_advance(p, ptr, end);
			if (*ptr == 'u') {
				jsi_advance(p, ptr, end);
				unsigned lo = jsi_read_utf16(p, ptr, end);
				ptr = p->ptr; end = p->end;
				if (lo == ~0u) return NULL;
				if (lo >= 0xdc00 && lo <= 0xdfff) {
					// Valid surrogate pair
					unsigned code = 0x10000 + ((hi - 0xd800) << 10 | (lo - 0xdc00));
					buf = jsi_write_utf8(buf, code);
				} else {
					// Bad surrogate pair
					buf = jsi_write_utf8(buf, hi);
					buf = jsi_write_utf8(buf, lo);
				}
			} else {
				// Surrogate with unrelated escape
				buf = jsi_write_utf8(buf, hi);
				char ch = jsi_escape_tab[*ptr];
				if (ch != 0) {
					*buf++ = ch;
				} else if (!p->dialect.allow_unknown_escape) {
					jsi_err(p, ptr, "Bad escape character");
					return NULL;
				} else {
					*buf++ = *ptr;
				}
				jsi_advance(p, ptr, end);
			}
		} else {
			// Unpaired surrogate
			buf = jsi_write_utf8(buf, hi);
		}
	} else {
		// Valid non-surrogate
		buf = jsi_write_utf8(buf, hi);
	}
	p->ptr = ptr;
	return buf;
}

jsi_noinline static const char *
jsi_copy_string(jsi_parser *p, const char *ptr, const char *end, jsi_value *value)
{
	size_t begin_offset = p->data_offset + (ptr - p->data) - 1;
	char *dst_begin = p->result_page + p->result_pos;
	char *dst_end = p->result_page + p->result_size;
	if (dst_end - dst_begin < 8) {
		jsi_copy_range range = { dst_begin, dst_begin, dst_end };
		if (!jsi_result_grow_copy(p, &range)) return 0;
		dst_begin = range.begin; dst_end = range.end;
	}
	size_t misalign = (size_t)(p->data_offset & 3u);
	size_t fixup = (4 - misalign) & 3u;
	dst_begin += fixup + 4;
	char *dst_ptr = dst_begin;

	#if JSI_USE_SSE
	__m128i sse_low = _mm_set1_epi8(31);
	__m128i sse_quot = _mm_set1_epi8('"');
	__m128i sse_slash = _mm_set1_epi8('\\');
	#endif

	for (;;) {
		#if JSI_USE_SSE
		while (end - ptr > 16 && dst_end - dst_ptr > 16) {
			__m128i chars = _mm_loadu_si128((const __m128i*)ptr);
			_mm_storeu_si128((__m128i*)dst_ptr, chars);
			__m128i isquot = _mm_cmpeq_epi8(chars, sse_quot);
			__m128i islow = _mm_cmpeq_epi8(_mm_max_epu8(chars, sse_low), sse_low);
			__m128i isslash = _mm_cmpeq_epi8(chars, sse_slash);
			int specmask = _mm_movemask_epi8(_mm_or_si128(islow, isslash));
			int quotmask = _mm_movemask_epi8(isquot);

			// Ignore special characters outside of the string
			specmask &= quotmask - 1;

			uint32_t index;
			if (specmask != 0) {
				JSI_SSE_CTZ(index, specmask);
				ptr += index;
				dst_ptr += index;
				break;
			} else if (quotmask != 0) {
				JSI_SSE_CTZ(index, quotmask);
				ptr += index;
				dst_ptr += index;
				p->ptr = ptr + 1;
				((uint32_t*)dst_begin)[-1] = (uint32_t)(dst_ptr - dst_begin);
				*dst_ptr++ = '\0';
				p->result_pos = dst_ptr - p->result_page;
				return dst_begin;
			} else {
				ptr += 16;
				dst_ptr += 16;
			}
		}
		#else
		while (end - ptr > 4 && dst_end - dst_ptr > 4) {
			char a = ptr[0], b = ptr[1], c = ptr[2], d = ptr[3];
			dst_ptr[0] = a;
			dst_ptr[1] = b;
			dst_ptr[2] = c;
			dst_ptr[3] = d;
			if (jsi_stop_char_tab[a]) { ptr += 0; dst_ptr += 0; break; }
			if (jsi_stop_char_tab[b]) { ptr += 1; dst_ptr += 1; break; }
			if (jsi_stop_char_tab[c]) { ptr += 2; dst_ptr += 2; break; }
			if (jsi_stop_char_tab[d]) { ptr += 3; dst_ptr += 3; break; }
			ptr += 4;
			dst_ptr += 4;
		}
		#endif

		if (dst_end - dst_ptr < 8) {
			jsi_copy_range range = { dst_begin, dst_ptr, dst_end };
			if (!jsi_result_grow_copy(p, &range)) return 0;
			dst_begin = range.begin; dst_ptr = range.ptr; dst_end = range.end;
		}

		char c = *ptr;
		jsi_advance(p, ptr, end);
		if (c == '\\') {
			char esc = *ptr;
			jsi_advance(p, ptr, end);
			char ch = jsi_escape_tab[esc];
			if (ch != 0) {
				*dst_ptr++ = ch;
			} else if (esc == 'u') {
				dst_ptr = jsi_copy_utf16(p, ptr, end, dst_ptr);
				ptr = p->ptr; end = p->end;
				if (!dst_ptr) return NULL;
			} else if (!p->dialect.allow_unknown_escape) {
				jsi_err_offset(p, ptr, -1, "Bad escape character");
				return NULL;
			} else {
				*dst_ptr++ = esc;
			}
		} else if (c == '"') {
			((uint32_t*)dst_begin)[-1] = (uint32_t)(dst_ptr - dst_begin);
			*dst_ptr++ = '\0';
			p->ptr = ptr;
			p->result_pos = dst_ptr - p->result_page;
			return dst_begin;
		} else if ((unsigned)c < 32) {
			if (c == '\0') {
				jsi_err_at(p, begin_offset, "Unclosed string");
				return NULL;
			} else if (c == '\n') {
				if (p->dialect.allow_control_in_string) {
					if (dst_ptr == dst_begin || (dst_ptr == dst_begin + 1 && *dst_begin == '\r')) {
						value->flags |= jsi_flag_multiline;
					}
					p->newline_offset = p->data_offset + (ptr - p->data) - 1;
					p->line_index++;
				} else {
					jsi_err_at(p, begin_offset, "Unclosed string");
					return NULL;
				}
			} else if (!p->dialect.allow_control_in_string) {
				jsi_err_offset(p, ptr, -1, "Control character in string");
				return NULL;
			}
			*dst_ptr++ = c;
		} else {
			*dst_ptr++ = c;
		}
	}
}

jsi_noinline static const char *
jsi_copy_bare_key(jsi_parser *p, const char *ptr, const char *end)
{
	char *dst_begin = p->result_page + p->result_pos;
	char *dst_end = p->result_page + p->result_size;
	if (dst_end - dst_begin < 8) {
		jsi_copy_range range = { dst_begin, dst_begin, dst_end };
		if (!jsi_result_grow_copy(p, &range)) return 0;
		dst_begin = range.begin; dst_end = range.end;
	}
	size_t misalign = (size_t)(p->data_offset & 3u);
	size_t fixup = (4 - misalign) & 3u;
	dst_begin += fixup + 4;
	char *dst_ptr = dst_begin;

	for (;;) {
		if (dst_end - dst_ptr < 2) {
			jsi_copy_range range = { dst_begin, dst_ptr, dst_end };
			if (!jsi_result_grow_copy(p, &range)) return 0;
			dst_begin = range.begin; dst_ptr = range.ptr; dst_end = range.end;
		}

		char c = *ptr;
		if ((unsigned)(c - 'A') <= 'Z' - 'A' || (unsigned)(c - 'a') <= 'z' - 'a'
			|| (unsigned)(c - '0') <= '9' - '0' || c == '_' || c == '$') {
			*dst_ptr++ = c;
			jsi_advance(p, ptr, end);
		} else {
			if (dst_ptr == dst_begin) {
				jsi_err(p, ptr, "Expected a key or '}'");
				return NULL;
			}
			((uint32_t*)dst_begin)[-1] = (uint32_t)(dst_ptr - dst_begin);
			*dst_ptr++ = '\0';
			p->ptr = ptr;
			p->result_pos = dst_ptr - p->result_page;
			return dst_begin;
		}
	}
}

jsi_forceinline static int
jsi_compare_internal(const char *jsi_a, const char *jsi_b)
{
	size_t len_a = jsi_length(jsi_a), len_b = jsi_length(jsi_b);
	if (len_a != len_b) return len_a < len_b ? -1 : 1;
	return memcmp(jsi_a, jsi_b, len_a);
}

jsi_forceinline static int
jsi_compare_external(const char *jsi_a, const char *str, size_t len)
{
	size_t len_a = jsi_length(jsi_a);
	if (len_a != len) return len_a < len ? -1 : 1;
	return memcmp(jsi_a, str, len);
}

static void
jsi_sort_obj_map(jsi_obj *obj, jsi_obj_map *map)
{
	size_t num = obj->num_props;
	jsi_prop *props = obj->props;
	uint32_t *src = map->entries, *dst = src + num;
	const size_t bottom_size = 16;

	// Swap `src` and `dst` so that `dst` ends up as the final array
	for (size_t width = bottom_size; width < num; width *= 2) {
		uint32_t *temp = src; src = dst; dst = temp;
	}

	// Write initial values
	for (size_t i = 0; i < num; i++) {
		src[i] = (uint32_t)i;
	}

	// Insertion sort bottom level blocks
	for (size_t begin = 0; begin < num; begin += bottom_size) {
		size_t end = begin + bottom_size < num ? begin + bottom_size : num;
		for (size_t i = begin + 1; i < end; i++) {
			for (size_t j = i; j > begin
				&& jsi_compare_internal(props[src[j - 1]].key, props[src[j]].key) > 0; j--) {
				uint32_t temp = src[j - 1]; src[j - 1] = src[j]; src[j] = temp;
			}
		}
	}

	// Merge sort bottom up
	for (size_t width = bottom_size; width < num; width *= 2) {
		for (size_t begin = 0; begin < num; begin += width * 2) {
			size_t i = begin;
			size_t iend = begin + width < num ? begin + width : num;
			size_t j = iend, jend = begin + width * 2 < num ? begin + width * 2 : num;
			for (size_t out = i; out < jend; out++) {
				if (i < iend && (j >= jend ||
					jsi_compare_internal(props[src[i]].key, props[src[j]].key) <= 0)) {
					dst[out] = src[i++];
				} else {
					dst[out] = src[j++];
				}
			}
		}
		
		uint32_t *temp = src; src = dst; dst = temp;
	}
}

static int
jsi_create_map(jsi_parser *p, jsi_obj *obj)
{
	size_t num_props = obj->num_props;

	// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
	size_t v = num_props * 2 - 1;
	v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16;
	size_t hash_size = v + 1;

	if (hash_size >= UINT32_MAX) {
		return jsi_err(p, p->ptr, "Object has too many properties");
	}
	jsi_obj_map *map = (jsi_obj_map*)jsi_push_result(p, sizeof(jsi_obj_map) + sizeof(uint32_t) * hash_size, 8);
	if (!map) return 0;
	obj->map = map;

	uint32_t mask = (uint32_t)hash_size - 1;
	map->mask = mask;
	memset(map->entries, 0, sizeof(uint32_t) * hash_size);
	size_t i;
	for (i = 0; i < num_props; i++) {
		uint32_t hash = jsi_key_hash(obj->props[i].key, jsi_length(obj->props[i].key));
		size_t ix = hash & mask;
		size_t scan = 0;
		uint32_t entry = (uint32_t)((hash & ~mask) | (i + 1));
		uint32_t ref;
		while ((ref = map->entries[ix]) != 0) {
			map->entries[ix] = entry;
			entry = ref;
			ix = (ix + 1) & mask;
			scan++;
		}
		map->entries[ix] = entry;

		if (scan > 32) break;
	}

	// If initializing as a hashmap fails use a sorted list
	if (i < num_props) {
		map->mask = 0;
		jsi_sort_obj_map(obj, map);
	}
	return 1;
}

jsi_noinline static int jsi_parse_value(jsi_parser *p, const char *ptr, const char *end, jsi_value *value);

static int
jsi_parse_error(jsi_parser *p, const char *ptr, const char *end, jsi_value *value)
{
	return jsi_err(p, ptr, "Expected a value");
}

static int
jsi_parse_object(jsi_parser *p, const char *ptr, const char *end, jsi_value *value)
{
	int depth = p->depth_left--;
	if (depth <= 0) {
		if (depth == JSI_DEPTH_IMPLICIT_ROOT) {
			// Skip `jsi_advance()` when parsing an implicit root object
			p->depth_left = p->max_depth - 1;
		} else {
			return jsi_err(p, ptr, "Too many nested values");
		}
	} else {
		jsi_advance(p, ptr, end);
	}

	value->type = jsi_type_object;
	if (*ptr != '"') {
		if (*ptr != '}') {
			if (!jsi_skip_whitespace(p, ptr, end)) return 0;
			ptr = p->ptr; end = p->end;
		}
		if (*ptr == '}') {
			jsi_advance(p, ptr, end);
			value->object = (jsi_obj*)&null_obj;
			p->ptr = ptr;
			p->depth_left++;
			return 1;
		}
	}

	size_t temp_base = p->temp_top;

	for (;;) {
		const char *key = NULL;
		if (*ptr != '"') {
			if (!jsi_skip_whitespace(p, ptr, end)) return 0;
			ptr = p->ptr; end = p->end;
		}
		if (*ptr == '"') {
			jsi_advance(p, ptr, end);
			key = jsi_copy_string(p, ptr, end, NULL);
		} else if (*ptr == '}') {
			if (p->dialect.allow_trailing_comma) {
				break;
			} else {
				while (ptr != p->data && *ptr != ',') --ptr;
				return jsi_err(p, ptr, "Trailing comma");
			}
		} else if (p->dialect.allow_bare_keys && *ptr) {
			key = jsi_copy_bare_key(p, ptr, end);
		} else {
			if (p->args->implicit_root_object && p->depth_left == p->max_depth - 1) {
				break;
			}
			return jsi_err(p, ptr, "Expected a key or '}'");
		}

		// Load pointers after string/bare key copy
		if (!key) return 0;
		ptr = p->ptr; end = p->end;

		if (jsi_likely(end - ptr > 2 && *ptr == ':')) {
			ptr += ptr[1] == ' ' ? 2 : 1;
		} else {
			if (*ptr != ':') {
				if (!jsi_skip_whitespace(p, ptr, end)) return 0;
				ptr = p->ptr; end = p->end;
				if (*ptr != ':') {
					if (p->dialect.allow_equals_as_colon && *ptr == '=') {
						// Parse as ':'
					} else {
						return jsi_err(p, ptr, "Expected ':' after key");
					}
				}
			}
			jsi_advance(p, ptr, end);
		}

		jsi_value item;
		if (!jsi_parse_value(p, ptr, end, &item)) return 0;
		item.key_hash = (uint16_t)((unsigned)key[0] + (jsi_length(key) << 7));

		ptr = p->ptr; end = p->end;

		if (p->temp_top + sizeof(jsi_prop) > p->temp_size) {
			if (!jsi_temp_grow(p)) return 0;
		}
		jsi_prop *prop = (jsi_prop*)(p->temp_stack + p->temp_top);
		p->temp_top += sizeof(jsi_prop);
		prop->key = key;
		prop->value = item;

		if (jsi_likely(end - ptr > 2 && *ptr == ',')) {
			ptr += ptr[1] == ' ' ? 2 : 1;
		} else {
			if (*ptr != '}') {
				jsi_skip_whitespace(p, ptr, end);
				ptr = p->ptr; end = p->end;
			}
			if (*ptr == '}') {
				break;
			} else if (*ptr == ',') {
				jsi_advance(p, ptr, end);
			} else if (p->dialect.allow_missing_comma) {
				continue;
			} else {
				return jsi_err(p, ptr, "Expected ',' or '}'");
			}
		}
	}

	jsi_advance(p, ptr, end);
	size_t props_size = p->temp_top - temp_base;
	jsi_obj *obj = (jsi_obj*)jsi_push_result(p, sizeof(jsi_obj) + props_size, 8);
	value->object = obj;
	if (!obj) return 0;
	size_t num_props = props_size / sizeof(jsi_prop);
	obj->num_props = num_props;
	memcpy(obj->props, p->temp_stack + temp_base, props_size);

	if (num_props >= 8) {
		if (!jsi_create_map(p, obj)) return 0;
	} else {
		obj->map = NULL;
	}

	p->depth_left++;
	p->temp_top = temp_base;
	p->ptr = ptr;
	return 1;
}

static int
jsi_parse_array(jsi_parser *p, const char *ptr, const char *end, jsi_value *value)
{
	int depth = p->depth_left--;
	if (depth <= 0) {
		if (depth == JSI_DEPTH_IMPLICIT_ROOT) {
			// Skip `jsi_advance()` when parsing an implicit root array
			p->depth_left = p->max_depth - 1;
		} else {
			return jsi_err(p, ptr, "Too many nested values");
		}
	} else {
		jsi_advance(p, ptr, end);
	}

	value->type = jsi_type_array;

	if (jsi_char_tab[*ptr] == jsi_char_whitespace) {
		jsi_skip_whitespace(p, ptr, end);
		ptr = p->ptr; end = p->end;
	}
	if (*ptr == ']') {
		jsi_advance(p, ptr, end);
		value->array = (jsi_arr*)&null_arr;
		p->ptr = ptr;
		p->depth_left++;
		return 1;
	}

	size_t temp_base = p->temp_top;
	for (;;) {
		jsi_value item;
		if (!jsi_parse_value(p, ptr, end, &item)) return 0;
		ptr = p->ptr; end = p->end;

		if (p->temp_top + sizeof(jsi_value) > p->temp_size) {
			if (!jsi_temp_grow(p)) return 0;
		}
		*(jsi_value*)(p->temp_stack + p->temp_top) = item;
		p->temp_top += sizeof(jsi_value);

		if (jsi_likely(end - ptr > 2 && *ptr == ',')) {
			ptr += ptr[1] == ' ' ? 2 : 1;
		} else {
			if (*ptr != ']') {
				jsi_skip_whitespace(p, ptr, end);
				ptr = p->ptr; end = p->end;
			}
			if (*ptr == ']') {
				break;
			} else if (*ptr == ',') {
				jsi_advance(p, ptr, end);
			} else if (*ptr == '\0' && p->args->implicit_root_array && p->depth_left == p->max_depth - 1) {
				break;
			} else if (p->dialect.allow_missing_comma) {
				continue;
			} else {
				return jsi_err(p, ptr, "Expected ',' or ']'");
			}
		}

		if (jsi_char_tab[*ptr] == jsi_char_whitespace) {
			jsi_skip_whitespace(p, ptr, end);
			ptr = p->ptr; end = p->end;
		}
		if (*ptr == ']') {
			if (p->dialect.allow_trailing_comma) {
				break;
			} else {
				while (ptr != p->data && *ptr != ',') --ptr;
				return jsi_err(p, ptr, "Trailing comma");
			}
		}
	}

	jsi_advance(p, ptr, end);
	size_t values_size = p->temp_top - temp_base;
	jsi_arr *arr = (jsi_arr*)jsi_push_result(p, sizeof(jsi_arr) + values_size, 8);
	value->array = arr;
	if (!arr) return 0;
	arr->num_values = values_size / sizeof(jsi_value);
	memcpy(arr->values, p->temp_stack + temp_base, values_size);

	p->depth_left++;
	p->temp_top = temp_base;
	p->ptr = ptr;
	return 1;
}

static int
jsi_parse_string_value(jsi_parser *p, const char *ptr, const char *end, jsi_value *value)
{
	value->type = jsi_type_string;
	jsi_advance(p, ptr, end);
	value->string = jsi_copy_string(p, ptr, end, value);
	return value->string != 0;
}

static int
jsi_parse_number(jsi_parser *p, const char *ptr, const char *end, jsi_value *value)
{
	value->type = jsi_type_number;
	int sign = 1;
	if (*ptr == '-') {
		jsi_advance(p, ptr, end);
		sign = -1;
		if (jsi_char_tab[*ptr] != jsi_char_number) {
			return jsi_err(p, ptr, "Invalid number");
		}
	}

	char buf[64], *buf_ptr = buf, *buf_end = buf + sizeof(buf) - 1;
	int int_val = 0;
	if (end - ptr > 10) {
		const char *int_end = ptr + 9;
		while (ptr != int_end) {
			char c = *ptr;
			unsigned digit = (unsigned)c - '0';
			if (digit > 10) break;
			*buf_ptr++ = c;
			ptr++;
			int_val = int_val * 10 + (int)digit;
		}
		if (jsi_char_tab[*ptr] != jsi_char_number) {
			value->number = (double)(int_val * sign);
			value->flags |= jsi_flag_integer;
			p->ptr = ptr;
			return 1;
		}
	}

	do {
		*buf_ptr++ = *ptr;
		jsi_advance(p, ptr, end);
		if (buf_ptr == buf_end) {
			return jsi_err(p, ptr, "Number is too long");
		}
	} while (jsi_char_tab[*ptr] == jsi_char_number);

	*buf_ptr = '\0';
	char *conv_end;
	value->number = strtod(buf, &conv_end) * (double)sign;
	if (conv_end != buf_ptr) {
		return jsi_err(p, ptr, "Failed to parse number");
	}

	p->ptr = ptr;
	return 1;
}

static int
jsi_parse_true(jsi_parser *p, const char *ptr, const char *end, jsi_value *value)
{
	value->type = jsi_type_boolean;
	value->boolean = 1;
	if (end - ptr >= 5) {
		if ((ptr[1]^'r')|(ptr[2]^'u')|(ptr[3]^'e')) {
			return jsi_err(p, ptr, "Bad value, 'true'?");
		}
		p->ptr = ptr + 4;
	} else {
		if (!jsi_match(p, ptr, end, "true")) {
			return jsi_err(p, ptr, "Bad value, 'true'?");
		}
	}
	return 1;
}

static int
jsi_parse_false(jsi_parser *p, const char *ptr, const char *end, jsi_value *value)
{
	value->type = jsi_type_boolean;
	value->boolean = 0;
	if (end - ptr >= 6) {
		if ((ptr[1]^'a')|(ptr[2]^'l')|(ptr[3]^'s')|(ptr[4]^'e')) {
			return jsi_err(p, ptr, "Bad value, 'false'?");
		}
		p->ptr = ptr + 5;
	} else {
		if (!jsi_match(p, ptr, end, "false")) {
			return jsi_err(p, ptr, "Bad value, 'false'?");
		}
	}
	return 1;
}

static int
jsi_parse_null(jsi_parser *p, const char *ptr, const char *end, jsi_value *value)
{
	value->type = jsi_type_null;
	if (end - ptr >= 5) {
		if ((ptr[1]^'u')|(ptr[2]^'l')|(ptr[3]^'l')) {
			return jsi_err(p, ptr, "Bad value, 'null'?");
		}
		p->ptr = ptr + 4;
	} else {
		if (!jsi_match(p, ptr, end, "null")) {
			return jsi_err(p, ptr, "Bad value, 'null'?");
		}
	}
	return 1;
}

// Must match `jsi_char_type`
jsi_parse_fn jsi_parse_funcs[] = {
	NULL,
	jsi_parse_error,
	jsi_parse_object,
	jsi_parse_array,
	jsi_parse_string_value,
	jsi_parse_number,
	jsi_parse_true,
	jsi_parse_false,
	jsi_parse_null,
};

jsi_noinline static int
jsi_parse_value(jsi_parser *p, const char *ptr, const char *end, jsi_value *value)
{
	for (;;) {
		char char_type = jsi_char_tab[*ptr];
		if (char_type != 0) {
			value->key_hash = 0;
			value->flags = 0;
			jsi_parse_fn func = jsi_parse_funcs[char_type];
			return func(p, ptr, end, value);
		} else {
			if (!jsi_skip_whitespace(p, ptr, end)) return 0;
			ptr = p->ptr; end = p->end;
		}
	}
}

static void
jsi_align_pointer(char **dst_ptr, size_t *dst_size, void *src_ptr, size_t src_size)
{
	size_t misalign = (size_t)((uintptr_t)src_ptr & 7u);
	size_t fixup = (8 - misalign) & 7u;
	if (src_size > fixup) {
		*dst_ptr = (char*)src_ptr + fixup;
		*dst_size = src_size - fixup;
	} else {
		*dst_ptr = NULL;
		*dst_size = 0;
	}
}

jsi_noinline static jsi_value *
jsi_parse(jsi_parser *p, jsi_args *args)
{
	jsi_args null_args;
	jsi_prop null_temp_buffer[128];
	if (!args) {
		memset(&null_args, 0, sizeof(null_args));
		p->args = &null_args;
	} else {
		p->args = args;
	}
	p->dialect = p->args->dialect;
	p->ptr = p->data;
	p->data_offset = 0;
	p->temp_top = 0;
	p->temp_stack = (char*)null_temp_buffer;
	p->temp_size = sizeof(null_temp_buffer);
	p->result_page = NULL;
	p->result_size = 0;
	p->result_pos = 0;
	p->line_index = 1;
	p->newline_offset = 0;
	p->result_min_alloc_size = 128;
	p->temp_allocated = 0;
	p->result_allocated = 0;
	p->max_depth = p->args->nesting_limit ? p->args->nesting_limit : INT_MAX;
	if (p->max_depth < 1) p->max_depth = 1;
	p->depth_left = p->max_depth;

	if (p->ptr == p->end) {
		jsi_refill(p);
	}

	if (args) {
		if (args->temp_size > sizeof(null_temp_buffer)) {
			jsi_align_pointer(&p->temp_stack, &p->temp_size,
				args->temp_buffer, args->temp_size);
		}
		if (args->result_size > 0) {
			jsi_align_pointer(&p->result_page, &p->result_size,
				args->result_buffer, args->result_size);
		}
	}

	jsi_result_value *result = (jsi_result_value*)jsi_push_result(p, sizeof(jsi_result_value), 8);
	if (!result) return NULL;

	int success;
	if (p->args->implicit_root_object) {
		p->depth_left = JSI_DEPTH_IMPLICIT_ROOT;
		result->value.key_hash = 0;
		result->value.flags = 0;
		success = jsi_parse_object(p, p->ptr, p->end, &result->value);
	} else if (p->args->implicit_root_array) {
		p->depth_left = JSI_DEPTH_IMPLICIT_ROOT;
		result->value.key_hash = 0;
		result->value.flags = 0;
		success = jsi_parse_array(p, p->ptr, p->end, &result->value);
	} else {
		success = jsi_parse_value(p, p->ptr, p->end, &result->value);
	}

	result->memory = p->result_allocated ? p->result_page : NULL;
	if (args && args->result_allocator.alloc_fn) {
		result->free_fn = args->result_allocator.free_fn;
		result->free_user = args->result_allocator.user;
	} else {
		result->free_fn = NULL;
		result->free_user = NULL;
	}

	// Check for trailing data
	if (success) {
		const char *ptr = p->ptr, *end = p->end;
		if (!p->args->allow_trailing_data && *ptr != '\0') {
			if (jsi_skip_whitespace(p, ptr, end)) {
				ptr = p->ptr;
				if (*ptr != '\0') {
					jsi_err(p, ptr, "Trailing data");
					success = 0;
				}
			} else {
				success = 0;
			}
		}
		p->args->end_offset = p->data_offset + (p->ptr - p->data);
	}

	if (p->temp_allocated) {
		jsi_mem_free(&p->args->temp_allocator, p->temp_stack, p->temp_size);
	}
	if (success) {
		if (args) {
			if (p->result_allocated) {
				args->result_used = args->result_size;
			} else {
				size_t misalign = p->result_page - (char*)args->result_buffer;
				args->result_used = p->result_pos + misalign;
			}
		}
	} else {
		jsi_free(&result->value);
		return NULL;
	}
	return &result->value;
}

typedef struct {
	FILE *file;
	char buffer[8192];
} jsi_file_stream;

static const void *
jsi_file_stream_refill(void *user, size_t *size)
{
	jsi_file_stream *stream = (jsi_file_stream*)user;
	if (feof(stream->file)) return NULL;
	size_t read = fread(stream->buffer, 1, sizeof(stream->buffer), stream->file);
	if (read < sizeof(stream->buffer)) {
		stream->buffer[read++] = '\0';
	}
	if (ferror(stream->file)) return NULL;
	*size = read;
	return stream->buffer;
}

// API

jsi_value *jsi_parse_memory(const void *data, size_t size, jsi_args *args)
{
	jsi_parser p;
	p.data = (const char*)data;
	p.end = (const char*)data + size;
	p.refill_fn = NULL;
	return jsi_parse(&p, args);
}

jsi_value *jsi_parse_string(const char *str, jsi_args *args)
{
	jsi_parser p;
	p.data = str;
	p.end = str + strlen(str) + 1; // Include \0 to skip refill()
	p.refill_fn = NULL;
	return jsi_parse(&p, args);
}

jsi_value *jsi_parse_file(const char *filename, jsi_args *args)
{
	jsi_parser p;
	jsi_file_stream stream;

#if defined(_MSC_VER) && _MSC_VER >= 1400
	if (fopen_s(&stream.file, filename, "rb")) {
		stream.file = NULL;
	}
#else
	stream.file = fopen(filename, "rb");
#endif

	if (!stream.file) {
		if (args) {
			p.line_index = 0;
			p.newline_offset = 0;
			jsi_err_at(&p, 0, "Could not open file");
		}
		return NULL;
	}
	setvbuf(stream.file, NULL, _IONBF, 0);

	size_t read = fread(stream.buffer, 1, sizeof(stream.buffer), stream.file);
	p.refill_fn = &jsi_file_stream_refill;
	p.refill_user = &stream;
	p.data = stream.buffer;
	p.end = stream.buffer + read;
	if (read < sizeof(stream.buffer)) {
		stream.buffer[read] = '\0';
		p.end++;
	}
	jsi_value *value = jsi_parse(&p, args);
	fclose(stream.file);
	return value;
}

jsi_value *jsi_parse_stream(jsi_refill_fn refill, void *user, jsi_args *args)
{
	jsi_parser p;
	p.refill_fn = refill;
	p.refill_user = user;
	size_t size;
	p.data = (const char*)refill(user, &size);
	p.end = p.data + size;
	return jsi_parse(&p, args);
}

jsi_value *jsi_parse_stream_initial(jsi_refill_fn refill, void *user, const void *data, size_t size, jsi_args *args)
{
	jsi_parser p;
	p.refill_fn = refill;
	p.refill_user = user;
	p.data = (const char*)data;
	p.end = (const char*)data + size;
	return jsi_parse(&p, args);
}

void jsi_free(jsi_value *value)
{
	if (!value) return;
	jsi_result_value *result = (jsi_result_value*)value;
	void *page = result->memory;
	if (result->free_fn) {
		jsi_free_fn free_fn = result->free_fn;
		void *free_user = result->free_user;
		while (page) {
			void *to_free = page;
			size_t size = *(size_t*)((char*)page + 8);
			page = *(void**)page;
			free_fn(free_user, to_free, size);
		}
	} else {
		while (page) {
			void *to_free = page;
			page = *(void**)page;
			free(to_free);
		}
	}
}

jsi_value *jsi_get_len(jsi_obj *obj, const char *key, size_t length)
{
	jsi_obj_map *map = obj->map;
	jsi_prop *props = obj->props;
	if (map) {
		uint32_t *entries = map->entries;
		if (map->mask) {
			// Hash map
			uint32_t hash = jsi_key_hash(key, length);
			uint32_t mask = map->mask, ix = hash & mask;
			for (;;) {
				uint32_t entry = entries[ix];
				if (!entry) return NULL;
				if (((entry ^ hash) & ~mask) == 0) {
					size_t prop_ix = (entry & mask) - 1;
					if (jsi_equal_len(props[prop_ix].key, key, length)) {
						return &props[prop_ix].value;
					}
				}
				ix = (ix + 1) & mask;
			}
		} else {
			// Sorted list
			size_t lo = 0, hi = obj->num_props;
			while (lo != hi) {
				size_t mid = (lo + hi) >> 1;
				if (jsi_compare_external(props[entries[mid]].key, key, length) > 0) {
					hi = mid;
				} else {
					lo = mid + 1;
				}
			}
			if (lo > 0 && jsi_equal_len(props[entries[lo - 1]].key, key, length)) {
				return &props[entries[lo - 1]].value;
			} else {
				return NULL;
			}
		}
	} else {
		// Unsorted properties
		char prefix = key[0];
		uint16_t key_hash = (uint16_t)((unsigned)key[0] + ((unsigned)length << 7));
		size_t num_props = obj->num_props;
		for (size_t i = 0; i < num_props; i++) {
			if (props[i].value.key_hash == key_hash && jsi_equal_len(props[i].key, key, length)) {
				return &props[i].value;
			}
		}
		return NULL;
	}
}

#if defined(_MSC_VER)
	#pragma warning(pop)
#endif
