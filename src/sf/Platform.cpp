#include "Platform.h"
#include "Base.h"

#ifdef __cplusplus
extern "C" {
#endif

void sf_debug_log(const char *str)
{
	sf::debugPrint("%s", str);
}

#ifdef __cplusplus
}
#endif
