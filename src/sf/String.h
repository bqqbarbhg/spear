#pragma once

#include "Base.h"
#include <string.h>

namespace sf {

struct String
{
	const char *data;
	size_t size;

	String() : data(nullptr), size(0) { }
	String(const char *data, size_t size) : data(data), size(size) { }
	sf_forceinline explicit String(const char *data) : data(data), size(strlen(data)) { }

	bool operator==(const String &rhs) const { return size == rhs.size && memcmp(data, rhs.data, size) == 0; }
	bool operator!=(const String &rhs) const { return size != rhs.size || memcmp(data, rhs.data, size) != 0; }

	const char *begin() const { return data; }
	const char *end() const { return data + size; }
};

struct StringBuf
{
	char *data;
	uint32_t size;
	uint32_t capacity;

	StringBuf() : data((char*)""), size(0), capacity(0) { }
	StringBuf(const StringBuf &rhs) {
		uint32_t sz = rhs.size;
		size = sz;
		capacity = sz;
		if (sz > 0) {
			data = (char*)memAlloc(sz + 1);
			memcpy(data, rhs.data, sz + 1);
		} else {
			data = (char*)"";
		}
	}

	StringBuf(StringBuf &&rhs) : data(rhs.data), size(rhs.size), capacity(rhs.capacity) {
		rhs.data = (char*)"";
		rhs.capacity = rhs.size = 0;
	}

	StringBuf &operator=(const StringBuf &rhs) {
		sf_assert(&rhs != this);
		uint32_t sz = rhs.size;
		if (sz > 0) {
			if (sz > size) impGrowTo(sz + 1);
			memcpy(data, rhs.data, sz + 1);
		}
		size = sz;
		return *this;
	}

	StringBuf &operator=(StringBuf &&rhs) {
		sf_assert(&rhs != this);
		if (capacity != 0 && data != (char*)(&rhs + 1)) {
			memFree(data);
		}
		data = rhs.data;
		size = rhs.size;
		capacity = rhs.capacity;
		rhs.data = (char*)"";
		rhs.capacity = rhs.size = 0;
		return *this;
	}

	~StringBuf() {
		if (capacity != 0 && data != (char*)(this + 1)) {
			memFree(data);
		}
	}

	const char *begin() const { return data; }
	const char *end() const { return data + size; }
	char *begin() { return data; }
	char *end() { return data + size; }

	explicit StringBuf(String s);
	StringBuf& operator=(String s);

	operator String() const { return String(data, size); }

	void clear() {
		size = 0;
		if (capacity != 0) {
			data[0] = '\0';
		}
	}

	void append(char ch) {
		if (size == capacity) impGrowToGeometric(size + 1);
		data[size++] = ch;
		data[size] = '\0';
	}

	void append(const char *str) { append(String(str)); }

	void append(String a);
	void append(String a, String b);
	void append(String a, String b, String c);
	void append(String a, String b, String c, String d);

	void format(const char *fmt, ...);

protected:
	StringBuf(char *data, uint32_t capacity) : data(data), size(0), capacity(capacity) { }

	sf_noinline void impGrowTo(size_t sz);
	sf_noinline void impGrowToGeometric(size_t sz);

};

template <uint32_t N>
struct SmallStringBuf : StringBuf
{
	char localData[N + 1];

	SmallStringBuf() : StringBuf(localData, N) { localData[0] = '\0'; }

	SmallStringBuf(SmallStringBuf &&rhs) : StringBuf(localData, N) {
		uint32_t sz = rhs.size;
		this->size = sz;
		if (sz > 0) {
			if (rhs.data == rhs.localData) {
				memcpy(this->data, rhs.data, sz + 1);
			} else {
				if (sz > N) {
					this->data = rhs.data;
					this->capacity = rhs.capacity;
				} else {
					memcpy(this->data, rhs.data, sz + 1);
				}
			}
		}
		rhs.data = rhs.localData;
		rhs.size = 0;
		rhs.capacity = N;
	}

	SmallStringBuf &operator=(const SmallStringBuf &rhs) {
		sf_assert(&rhs != this);
		uint32_t sz = rhs.size;
		if (sz > 0) {
			if (sz > this->size) this->impGrowTo(sz);
			memcpy(this->data, rhs.data, sz + 1);
		}
		this->size = sz;
		return *this;
	}
	SmallStringBuf &operator=(SmallStringBuf &&rhs) {
		sf_assert((void*)&rhs != (void*)this);
		if (this->capacity != 0 && this->data != this->localData) {
			memFree(this->data);
		}
		uint32_t sz = rhs.size;
		this->size = sz;
		if (sz > 0) {
			if (rhs.data == rhs.localData) {
				this->data = localData;
				this->capacity = N;
				memcpy(this->data, rhs.data, sz + 1);
			} else {
				if (sz > N) {
					// Heap -> Heap
					this->data = rhs.data;
					this->capacity = rhs.capacity;
				} else {
					// Heap -> Stack
					this->data = localData;
					this->capacity = N;
					memcpy(this->data, rhs.data, sz + 1);
					memFree(rhs.data);
				}
			}
		}
		rhs.data = rhs.localData;
		rhs.size = 0;
		rhs.capacity = N;
		return *this;
	}
};

sf_inline bool operator==(const String &lhs, const StringBuf &rhs) {
	return lhs == (String)rhs;
}

uint32_t hash(const String &str);

template <> struct IsZeroInitializable<String> { enum { value = 1 }; };

}
