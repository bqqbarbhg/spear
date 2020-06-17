#include "mx_platform.h"

#if defined(MX_PLATFORM_EMSCRIPTEN) || defined(MX_PLATFORM_POSIX)
uint32_t mx_imp_next_thread_id;
__thread uint32_t mx_imp_thread_id;
#endif
