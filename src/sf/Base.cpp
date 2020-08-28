#include "Base.h"

#include "Array.h"
#include "Reflection.h"

#include <stdarg.h>
#include <stdio.h>

#include "ext/mx/mx_platform.h"
#include "ext/mx/mx_sync.h"

#if SF_OS_WINDOWS
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

#if SF_OS_EMSCRIPTEN
	#include <emscripten.h>
#endif

namespace sf {

char *memPrintf(const char *fmt, ...)
{
	va_list args1, args2;

	va_start(args1, fmt);
	va_copy(args2, args1);

	// First pass: Try to format to a local buffer
	char local[256];
	int size = vsnprintf(local, sizeof(local), fmt, args1);
	if (size < 0) {
		char *fail = (char*)memAlloc(1);
		fail[0] = '\0';
		return fail;
	}
	va_end(args1);

	// Second pass: Allocate the required size and copy or re-format
	size_t bufSize = (size_t)size + 1;
	char *result = (char*)memAlloc(bufSize);
	if (size < sizeof(local)) {
		memcpy(result, local, bufSize);
	} else {
		vsnprintf(result, bufSize, fmt, args2);
	}

	va_end(args2);
	return result;
}

void debugPrint(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

#if SF_OS_WINDOWS
	{
		SmallStringBuf<1024> line;
		line.vformat(fmt, args);
		OutputDebugStringA(line.data);
		printf("%s", line.data);
	}
#else
	{
		vprintf(fmt, args);
	}
#endif

	va_end(args);
}

void debugPrintLine(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

#if SF_OS_WINDOWS
	{
		SmallStringBuf<1024> line;
		line.vformat(fmt, args);
		line.append('\n');
		OutputDebugStringA(line.data);
		printf("%s", line.data);
	}
#else
	{
		vprintf(fmt, args);
		putchar('\n');
	}
#endif

	va_end(args);
}

#if SF_OS_EMSCRIPTEN
EM_JS(void, sf_em_print_json, (const char *label, int labelLen, const char *json, int jsonLen), {
	var label = UTF8ToString(label, labelLen);
	var json = UTF8ToString(json, jsonLen);
	console.log(label, JSON.parse(json));
});
#endif

void debugPrintJson(const sf::String &label, const sf::String &json)
{
#if SF_OS_EMSCRIPTEN
	sf_em_print_json(label.data, (int)label.size, json.data, (int)json.size);
#else
	sf::debugPrint("%.*s: %.*s\n", (int)label.size, label.data, (int)json.size, json.data);
#endif
}

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
uint32_t roundToPow2(uint32_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

uint32_t hashBuffer(const void *data, size_t size)
{
	uint32_t hash = 0;

	const uint32_t seed = UINT32_C(0x9e3779b9);
	const uint32_t *word = (const uint32_t*)data;

	#if SF_ARCH_WASM || (SF_ARCH_ARM && !SF_ARCH_ARM64)
	if ((uintptr_t)data % 4 == 0) {
		while (size >= 4) {
			hash = ((hash << 5u | hash >> 27u) ^ *word++) * seed;
			size -= 4;
		}
	} else {
		while (size >= 4) {
			const uint8_t *b = (const uint8_t*)word++;
			uint32_t w = (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
			hash = ((hash << 5u | hash >> 27u) ^ w) * seed;
			size -= 4;
		}
	}
	#else
	while (size >= 4) {
		hash = ((hash << 5u | hash >> 27u) ^ *word++) * seed;
		size -= 4;
	}
	#endif

	const uint8_t *byte = (const uint8_t*)word;
	if (size > 0) {
		uint32_t w = 0;
		while (size > 0) {
			w = w << 8 | *byte++;
			size--;
		}
		hash = ((hash << 5u | hash >> 27u) ^ w) * seed;
	}

	return (uint32_t)hash;
}

// https://github.com/skeeto/hash-prospector
uint32_t hash(uint32_t val)
{
	uint32_t x = val;
	x ^= x >> 16;
    x *= UINT32_C(0x7feb352d);
    x ^= x >> 15;
    x *= UINT32_C(0x846ca68b);
    x ^= x >> 16;
    return x;
}

uint32_t hash(uint64_t val)
{
	return hashCombine((uint32_t)val, (uint32_t)(val >> 32u));
}

uint32_t hashReverse32(uint32_t hash)
{
	uint32_t x = hash;
    x ^= x >> 16;
    x *= UINT32_C(0x43021123);
    x ^= x >> 15 ^ x >> 30;
    x *= UINT32_C(0x1d69e2a5);
    x ^= x >> 16;
    return x;
}

struct CPointerType final : Type
{
	CPointerType(Type *type)
		: Type("pointer", getTypeInfo<char*>(), HasPointer)
	{
		elementType = type;
	}

	virtual void init() override
	{
		if (elementType->flags & PolymorphBase) {
			flags |= Polymorph;
		}
	}

	virtual void getName(sf::StringBuf &buf) override
	{
		elementType->getName(buf);
		buf.append("*");
	}

	virtual PolymorphInstance instGetPolymorph(void *inst) override
	{
		void *ptr = *(void**)inst;
		if (ptr) {
			return elementType->instGetPolymorph(ptr);
		} else {
			return { };
		}
	}
};

void initCPointerType(Type *t, Type *type)
{
	new (t) CPointerType(type);
}

struct CArrayType final : Type
{
	size_t arraySize;

	CArrayType(const TypeInfo &info, Type *type,  size_t size)
		: Type("array", info, HasArray | HasArrayResize)
		, arraySize(size)
	{
		elementType = type;
		if (type->flags & IsPod) flags |= IsPod;
	}

	virtual void getName(sf::StringBuf &buf) override
	{
		elementType->getName(buf);
		buf.format("[%zu]", arraySize);
	}

	virtual VoidSlice instArrayReserve(void *inst, size_t size, sf::Array<char> *scratch) override
	{
		sf_assert(size == arraySize);
		return { inst, arraySize };
	}

	virtual void instArrayResize(void *inst, size_t size, VoidSlice elements) override
	{
		sf_assert(size == arraySize);
	}

	virtual VoidSlice instGetArray(void *inst, sf::Array<char> *scratch) override
	{
		return { inst, arraySize };
	}

};

void initCArrayType(Type *t, const TypeInfo &info, Type *type, size_t size)
{
	new (t) CArrayType(info, type, size);
}

static uint32_t g_typeNumInits;
static mx_mutex g_typeInitMutex;
static uint32_t g_typeNumWaiters;
static mx_semaphore g_typeWaiterSema;
static Type *g_nextType = nullptr;

bool beginTypeInit(uint32_t *flag)
{
	if (mxa_load32_acq(flag) != 0) return false;
	if (mxa_cas32(flag, 0, 1)) {
		mxa_inc32_nf(&g_typeNumInits);
		return true;
	} else {
		return false;
	}
}

void endTypeInit(Type *type)
{
	Type *next;
	do {
		next = (Type*)mxa_load_ptr(&g_nextType);
		type->next = next;
	} while (!mxa_cas_ptr(&g_nextType, next, type));

	uint32_t numLeft = mxa_dec32(&g_typeNumInits) - 1;
	if (numLeft == 0) {
		mx_mutex_lock(&g_typeInitMutex);

		Type *t = (Type*)mxa_load_ptr(&g_nextType);
		while (t && (t->flags & Type::Initialized) == 0) {
			t->init();
			t->flags |= Type::Initialized;
			t = t->next;
		}

		if (g_typeNumWaiters > 0) {
			mx_semaphore_signal_n(&g_typeWaiterSema, g_typeNumWaiters);
		}
		mx_mutex_unlock(&g_typeInitMutex);
	}
}

void waitForTypeInit()
{
	if (mxa_load32_acq(&g_typeNumInits) > 0) {
		mx_mutex_lock(&g_typeInitMutex);
		if (mxa_load32_acq(&g_typeNumInits) > 0) {
			mx_mutex_unlock(&g_typeInitMutex);
			return;
		}
		g_typeNumWaiters++;
		mx_mutex_unlock(&g_typeInitMutex);

		mx_semaphore_wait(&g_typeWaiterSema);
	}
}

}
