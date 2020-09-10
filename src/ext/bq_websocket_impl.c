
#include "sf/Platform.h"

#define bqws_malloc(size) sf_malloc(size)
#define bqws_realloc(ptr, old_size, new_size) sf_realloc(ptr, new_size)
#define bqws_free(ptr, size) sf_free(ptr)

#include "bq_websocket.c.h"
#include "bq_websocket_platform.c.h"
