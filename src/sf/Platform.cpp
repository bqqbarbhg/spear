#include "Platform.h"
#include "Base.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void sf_debug_log(const char *str)
{
	sf::debugPrintLine("%s", str);
}

#if SF_OS_WASM
void sf_wasm_debugbreak(const char *file, int line)
{
	printf("Debug break: %s %d\n", file, line);
	abort();
}
#endif

#ifdef __cplusplus
}
#endif
