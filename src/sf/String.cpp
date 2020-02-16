#include "String.h"

#include <stdarg.h>
#include <stdio.h>

namespace sf {

StringBuf::StringBuf(String s)
{
	sf_assert(s.size < UINT32_MAX);
	if (s.size > 0) {
		data = (char*)memAlloc(s.size + 1);
		memcpy(data, s.data, s.size);
		capacity = size = (uint32_t)s.size;
	} else {
		data = (char*)"";
		size = 0;
		capacity = 0;
	}
}

StringBuf& StringBuf::operator=(String s)
{
	if (s.size == 0) return *this;
	if (s.size > capacity) impGrowToGeometric(s.size);
	memcpy(data, s.data, s.size);
	data[s.size] = '\0';
	size = (uint32_t)s.size;
	return *this;
}

void StringBuf::append(String a)
{
	size_t sz = size + a.size;
	if (sz == 0) return;
	if (sz > capacity) impGrowToGeometric(sz);
	memcpy(data + size, a.data, a.size);
	size += (uint32_t)a.size;
	data[size] = '\0';
}

void StringBuf::append(String a, String b)
{
	size_t sz = size + a.size + b.size;
	if (sz == 0) return;
	if (sz > capacity) impGrowToGeometric(sz);
	memcpy(data + size, a.data, a.size);
	size += (uint32_t)a.size;
	memcpy(data + size, b.data, b.size);
	size += (uint32_t)b.size;
	data[size] = '\0';
}

void StringBuf::append(String a, String b, String c)
{
	size_t sz = size + a.size + b.size + c.size;
	if (sz == 0) return;
	if (sz > capacity) impGrowToGeometric(sz);
	memcpy(data + size, a.data, a.size);
	size += (uint32_t)a.size;
	memcpy(data + size, b.data, b.size);
	size += (uint32_t)b.size;
	memcpy(data + size, c.data, c.size);
	size += (uint32_t)c.size;
	data[size] = '\0';
}

void StringBuf::append(String a, String b, String c, String d)
{
	size_t sz = size + a.size + b.size + c.size + d.size;
	if (sz == 0) return;
	if (sz > capacity) impGrowToGeometric(sz);
	memcpy(data + size, a.data, a.size);
	size += (uint32_t)a.size;
	memcpy(data + size, b.data, b.size);
	size += (uint32_t)b.size;
	memcpy(data + size, c.data, c.size);
	size += (uint32_t)c.size;
	memcpy(data + size, d.data, d.size);
	size += (uint32_t)d.size;
	data[size] = '\0';
}

void StringBuf::format(const char *fmt, ...)
{
	va_list args1, args2;

	va_start(args1, fmt);
	va_copy(args2, args1);

	// First pass: Try to format to a the current buffer
	int appendSize;
	if (capacity == 0) {
		appendSize = vsnprintf(nullptr, 0, fmt, args1);
	} else {
		appendSize = vsnprintf(data + size, capacity - size, fmt, args1);
	}

	if (appendSize < 0) return;
	va_end(args1);

	// Second pass: Potentially re-allocate and re-format
	if (size + appendSize > capacity) {
		impGrowToGeometric(size + appendSize);
		vsnprintf(data + size, capacity - size, fmt, args2);
	}

	va_end(args2);
}

sf_noinline void StringBuf::impGrowTo(size_t sz) {
	sf_assert(sz <= UINT32_MAX);
	if (capacity > 0 && data != (char*)(this + 1)) {
		data = (char*)memRealloc(data, sz + 1);
	} else {
		char *newData = (char*)memAlloc(sz + 1);
		if (sz > 0) memcpy(newData, data, sz + 1);
		data = newData;
	}
	capacity = (uint32_t)sz;
}

sf_noinline void StringBuf::impGrowToGeometric(size_t sz) {
	sf_assert(sz <= UINT32_MAX);
	if (sz < capacity * 2) sz = capacity * 2;
	if (capacity > 0 && data != (char*)(this + 1)) {
		data = (char*)memRealloc(data, sz + 1);
	} else {
		char *newData = (char*)memAlloc(sz + 1);
		if (sz > 0) memcpy(newData, data, sz + 1);
		data = newData;
	}
	capacity = (uint32_t)sz;
}

uint32_t hash(const String &str)
{
	uint32_t h = 0x811c9dc5;
	for (char c : str) {
		 h = (h ^ (uint8_t)c) * 0x01000193;
	}
	return h;
}

}
