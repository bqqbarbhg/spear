#include "mx_platform.h"

#ifdef MX_PLATFORFM_EMSCRIPTEN
uint32_t mx_imp_next_thread_id;
__thread uint32_t mx_imp_thread_id;
#endif
