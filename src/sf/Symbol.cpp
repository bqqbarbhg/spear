#include "Symbol.h"

#include "sf/Reflection.h"

#include "ext/mx/mx_platform.h"
#include "ext/mx/mx_sync.h"
#include "ext/rhmap.h"

namespace sf {

struct SymbolType : Type
{
	SymbolType()
		: Type("Symbol", sizeof(StringBuf), HasArray|HasString|HasSetString)
	{
		elementType = typeOfRecursive<char>();
	}

	virtual VoidSlice instGetArray(void *inst)
	{
		Symbol *sym = (Symbol*)inst;
		return { (void*)sym->data, sym->size() };
	}

	virtual void instSetString(void *inst, sf::String str)
	{
		Symbol *sym = (Symbol*)inst;
		*sym = Symbol(str);
	}

};

template<> void initType<Symbol>(Type *t) { new (t) SymbolType(); }

static const constexpr uint32_t DataHeaderSize = sizeof(uint32_t) * 2;

sf_forceinline uint32_t &dataSize(const char *data) { return ((uint32_t*)data)[-1]; }
sf_forceinline uint32_t &dataRefs(const char *data) { return ((uint32_t*)data)[-2]; }

struct SymbolPool
{
	mx_mutex mutex;
	rhmap map;
	char **data;
};

SymbolPool g_symbolPool;

static const char *findSymbolData(const char *data, size_t length)
{
	uint32_t hash = sf::hashBuffer(data, length);
	SymbolPool &pool = g_symbolPool;

	mx_mutex_lock(&pool.mutex);

	if (pool.map.size == pool.map.capacity) {
		size_t count, allocSize;
		rhmap_grow(&pool.map, &count, &allocSize, 16, 0.5);
		char *alloc = (char*)memAlloc(allocSize + sizeof(char*) * count);
		char **newData = (char**)(alloc + allocSize);
		memcpy(newData, pool.data, sizeof(char*) * pool.map.size);
		void *oldAlloc = rhmap_rehash(&pool.map, count, allocSize, alloc);
		memFree(oldAlloc);
		pool.data = newData;
	}

	rhmap_iter iter = { &pool.map, hash };
	uint32_t index;
	while (rhmap_find(&iter, &index)) {
		if (pool.data[index] == data) {
			uint32_t refs = mxa_inc32_nf(&dataRefs(pool.data[index]));
			if (refs > 0) return pool.data[index];
			// About to be deleted, decrement refs back to zero
			// and continue search
			mxa_dec32_nf(&dataRefs(pool.data[index]));
		}
	}

	char *newData = (char*)memAlloc(DataHeaderSize + length + 1) + DataHeaderSize;
	dataSize(newData) = (uint32_t)length;
	dataRefs(newData) = 1;
	memcpy(newData, data, length);
	newData[length] = '\0';

	pool.data[pool.map.size] = newData;
	rhmap_insert(&iter, pool.map.size);

	mx_mutex_unlock(&pool.mutex);

	return newData;
}

static void removeSymbolData(const char *data)
{
	uint32_t hash = sf::hashBuffer(data, dataSize(data));
	SymbolPool &pool = g_symbolPool;

	mx_mutex_lock(&pool.mutex);

	rhmap_iter iter = { &pool.map, hash };
	uint32_t index;
	while (rhmap_find(&iter, &index)) {
		if (pool.data[index] == data) {
			break;
		}
	}

	rhmap_remove(&iter);
	if (index < pool.map.size) {
		char *swap = pool.data[pool.map.size];
		uint32_t hash = sf::hashBuffer(swap, dataSize(swap));
		rhmap_update(&pool.map, hash, pool.map.size, index);
		pool.data[index] = swap;
	}

	memFree((char*)data - DataHeaderSize);

	mx_mutex_unlock(&pool.mutex);
}

const char Symbol::emptyDataBuf[9] = "\x00\x00\x00\x00" "\x00\x00\x00\x00" "";

Symbol::Symbol(const char *data)
	: data(findSymbolData(data, strlen(data)))
{
}

Symbol::Symbol(const char *data, size_t length)
	: data(findSymbolData(data, length))
{
}

Symbol::Symbol(const sf::String &str)
	: data(findSymbolData(str.data, str.size))
{
}

Symbol::Symbol(const Symbol &rhs)
	: data(rhs.data)
{
	if (rhs.data != emptyData) {
		mxa_inc32_nf(&dataRefs(rhs.data));
	}
}

Symbol::~Symbol()
{
	if (data == emptyData) return;
	uint32_t refs = mxa_dec32_nf(&dataRefs(data)) - 1;
	if (refs == 0) removeSymbolData(data);
}

Symbol &Symbol::operator=(const Symbol &rhs)
{
	if (data == rhs.data) return *this;
	if (data != emptyData) {
		uint32_t refs = mxa_dec32_nf(&dataRefs(data)) - 1;
		if (refs == 0) removeSymbolData(data);
	}
	if (rhs.data != emptyData) {
		uint32_t refs = mxa_inc32_nf(rhs.data);
		// We can't have revived a string since `rhs` may not
		// be freed concurrently!
		sf_assert(refs > 0);
	}
	data = rhs.data;
	return *this;
}

Symbol &Symbol::operator=(Symbol &&rhs) noexcept
{
	if (data == rhs.data) return *this;
	if (data != emptyData) {
		uint32_t refs = mxa_dec32_nf(&dataRefs(data)) - 1;
		if (refs == 0) removeSymbolData(data);
	}
	data = rhs.data;
	rhs.data = emptyData;
	return *this;
}

}
