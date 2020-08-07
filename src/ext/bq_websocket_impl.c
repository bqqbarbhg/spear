
#include "sf/Platform.h"

#define bqws_malloc(size) sf_malloc(size)
#define bqws_realloc(ptr, size) sf_realloc(ptr, size)
#define bqws_free(ptr) sf_free(ptr)

#define BQWS_IMPL

#include "bq_websocket.c"
#include "bq_websocket_platform.c"
