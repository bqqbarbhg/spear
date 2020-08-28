#include "String.h"

#include "Reflection.h"

#include <stdarg.h>
#include <stdio.h>

namespace sf {

struct StringBufType : Type
{
	StringBufType()
		: Type("sf::StringBuf", getTypeInfo<StringBuf>(), HasArray|HasArrayResize|HasString|HasSetString)
	{
		elementType = typeOfRecursive<char>();
	}

	virtual VoidSlice instGetArray(void *inst, sf::Array<char> *scratch) override
	{
		StringBuf *buf = (StringBuf*)inst;
		return { buf->data, buf->size };
	}

	virtual VoidSlice instArrayReserve(void *inst, size_t size, sf::Array<char> *scratch) override
	{
		StringBuf *buf = (StringBuf*)inst;
		buf->reserve(size);
		return { buf->data, buf->capacity };
	}

	virtual void instArrayResize(void *inst, size_t size, VoidSlice elements) override
	{
		StringBuf *buf = (StringBuf*)inst;
		buf->resize(size);
	}

	virtual void instSetString(void *inst, sf::String str) override
	{
		StringBuf *buf = (StringBuf*)inst;
		*buf = str;
	}

};

template<> void initType<StringBuf>(Type *t) { new (t) StringBufType(); }


bool String::operator<(const String &rhs) const
{
	size_t len = sf::min(size, rhs.size);
	int cmp = memcmp(data, rhs.data, len);
	if (cmp != 0) return cmp < 0;
	return size < rhs.size;
}

char StringBuf::zeroCharBuffer[1] = { };

StringBuf::StringBuf(String s)
{
	sf_assert(s.size < UINT32_MAX);
	if (s.size > 0) {
		data = (char*)memAlloc(s.size + 1);
		memcpy(data, s.data, s.size);
		data[s.size] = '\0';
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
	int appendSize = vsnprintf(data + size, capacity + 1 - size, fmt, args1);

	if (appendSize < 0) return;
	va_end(args1);

	// Second pass: Potentially re-allocate and re-format
	if (size + appendSize + 1 > capacity) {
		impGrowToGeometric(size + appendSize);
		vsnprintf(data + size, capacity + 1 - size, fmt, args2);
	}
	size += appendSize;

	va_end(args2);
}

void StringBuf::vformat(const char *fmt, va_list args1)
{
	va_list args2;

	va_copy(args2, args1);

	// First pass: Try to format to a the current buffer
	int appendSize = vsnprintf(data + size, capacity + 1 - size, fmt, args1);

	if (appendSize < 0) return;

	// Second pass: Potentially re-allocate and re-format
	if (size + appendSize + 1 > capacity) {
		impGrowToGeometric(size + appendSize);
		vsnprintf(data + size, capacity + 1 - size, fmt, args2);
	}
	size += appendSize;

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
		memcpy(newData, data, size + 1);
		data = newData;
	}
	capacity = (uint32_t)sz;
}

bool beginsWith(sf::String s, sf::String prefix)
{
	return s.size >= prefix.size && sf::String(s.data, prefix.size) == prefix;
}

bool endsWith(sf::String s, sf::String suffix)
{
	return s.size >= suffix.size && sf::String(s.data + s.size - suffix.size, suffix.size) == suffix;
}

size_t indexOf(sf::String s, sf::String substr)
{
	if (s.size < substr.size) return SIZE_MAX;
	if (substr.size == 0) return SIZE_MAX;

	// Hash s[0]*a^n + s[1]*a^2 + s[len]*a
	uint32_t a = 13;
	uint32_t an = 1;

	uint32_t substrHash = 0;
	for (size_t i = substr.size; i > 0; i--) {
		an *= a;
		substrHash += (uint32_t)substr.data[i - 1] * an;
	}


	uint32_t sHash = 0;
	for (size_t i = 0; i < substr.size - 1; i++) {
		sHash = (sHash + (uint32_t)s.data[i]) * a;
	}

	for (size_t i = substr.size - 1; i < s.size; i++) {
		sHash = (sHash + (uint32_t)s.data[i]) * a;

		size_t offset = i - (substr.size - 1);
		if (sHash == substrHash && !memcmp(s.data + offset, substr.data, substr.size)) {
			return offset;
		}

		sHash -= s.data[offset] * an;
	}

	return SIZE_MAX;
}

size_t indexOf(sf::String s, char ch)
{
	const char *ptr = (const char*)memchr(s.data, ch, s.size);
	if (!ch) return SIZE_MAX;
	return ptr - s.data;
}

bool contains(sf::String s, sf::String substr)
{
	return indexOf(s, substr) != SIZE_MAX;
}

bool contains(sf::String s, char ch)
{
	return memchr(s.data, ch, s.size);
}

}
