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

namespace sf {

#if SF_DEBUG

void *memAlloc(size_t size)
{
	return malloc(size);
}

void *memRealloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

void memFree(void *ptr)
{
	free(ptr);
}


#endif // SF_DEBUG (for debug allocation)

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
	}
#else
	{
		vprintf(fmt, args);
		putchar('\n');
	}
#endif

	va_end(args);
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
	while (size >= 4) {
		hash = ((hash << 5u | hash >> 27u) ^ *word++) * seed;
		size -= 4;
	}

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

struct PointerType : Type
{
	PointerType(Type *type)
		: Type("pointer", sizeof(void*), HasArray)
	{
		elementType = type;
	}

	virtual void getName(sf::StringBuf &buf)
	{
		elementType->getName(buf);
		buf.append("*");
	}

	virtual VoidSlice instGetArray(void *inst)
	{
		void **ptr = (void**)inst;
		return { *ptr, (size_t)(*ptr != 0 ? 1 : 0) };
	}
};

void initPointerType(Type *t, Type *type)
{
	new (t) PointerType(type);
}

static uint32_t g_numTypeInits;

bool beginTypeInit(uint32_t *flag)
{
	if (mxa_load32_acq(flag) != 0) return false;
	if (mxa_cas32(flag, 0, 1)) {
		mxa_inc32_nf(&g_numTypeInits);
		return true;
	} else {
		return false;
	}
}

void endTypeInit()
{
	mxa_dec32_rel(&g_numTypeInits);
}

void waitForTypeInit()
{
	// TODO: Something better
	while (mxa_load32_acq(&g_numTypeInits) > 0) {
		Sleep(1);
	}
}

}
