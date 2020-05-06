#pragma once

#include "sf/Base.h"
#include "sf/String.h"

namespace sf {

struct Symbol
{
	static const char emptyDataBuf[9];
	static constexpr const char *emptyData = emptyDataBuf + 8;

	const char *data;

	sf_forceinline uint32_t size() const { return ((const uint32_t*)data)[-1]; }

	Symbol() : data(emptyData) { }
	explicit Symbol(const char *data);
	Symbol(const char *data, size_t length);
	explicit Symbol(const sf::String &str);

	Symbol(const Symbol &rhs);
	Symbol(Symbol &&rhs) noexcept
		: data(rhs.data) {
		rhs.data = emptyData;
	}
	~Symbol();

	operator String() const { return String(data, size()); }

	Symbol &operator=(const Symbol &rhs);
	Symbol &operator=(Symbol &&rhs) noexcept;

	explicit operator bool() const { return data != emptyData; }
	bool operator!() const { return data == emptyData; }

	sf_forceinline bool operator==(const Symbol &rhs) const { return data == rhs.data; }
	sf_forceinline bool operator!=(const Symbol &rhs) const { return data != rhs.data; }
	sf_forceinline bool operator<(const Symbol &rhs) const { return data < rhs.data; }
	sf_forceinline bool operator>(const Symbol &rhs) const { return data > rhs.data; }
	sf_forceinline bool operator<=(const Symbol &rhs) const { return data <= rhs.data; }
	sf_forceinline bool operator>=(const Symbol &rhs) const { return data >= rhs.data; }
};

sf_inline uint32_t hash(const Symbol &s) { return hashPointer(s.data); }

}
