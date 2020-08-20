#include "Platform.h"
#include "Base.h"
#include "Thread.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void sf_debug_log(const char *str)
{
	sf::debugPrintLine("%s", str);
}

void sf_debug_logf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	sf::SmallStringBuf<1024> line;
	line.vformat(fmt, args);

	va_end(args);

	sf::debugPrint("%s\n", line.data);


}

#if SF_OS_WASM
void sf_wasm_debugbreak(const char *file, int line)
{
	printf("Debug break: %s %d\n", file, line);
	abort();
}
#endif

void sf_set_debug_thread_name(const char *name)
{
	sf::setDebugThreadName(sf::String(name));
}

#ifdef __cplusplus
}
#endif

#if SF_USE_MIMALLOC && !defined(MI_MALLOC_OVERRIDE)
	#include "ext/mimalloc/mimalloc-new-delete.h"
#endif
