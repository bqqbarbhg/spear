#include "Base.h"

#include "Array.h"

#include <stdarg.h>
#include <stdio.h>

#if SF_OS_WINDOWS
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

// TODO: Custom lightweight mutex
#include <mutex>
#include <condition_variable>
#include <atomic>

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

void *alignedAlloc(size_t size, size_t align)
{
	return _aligned_malloc(size, align);
}

void *alignedRealloc(void *ptr, size_t size, size_t align)
{
	return _aligned_realloc(ptr, size, align);
}

void alignedFree(void *ptr)
{
	return _aligned_free(ptr);
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
		char line[512];
		vsnprintf(line, sizeof(line), fmt, args);
		OutputDebugStringA(line);
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
		char line[512];
		vsnprintf(line, sizeof(line), fmt, args);
		OutputDebugStringA(line);
		OutputDebugStringA("\n");
	}
#else
	{
		vprintf(fmt, args);
		putchar('\n');
	}
#endif

	va_end(args);
}

}
