#include "sf/Platform.h"

#define jsi_default_malloc(size) sf_malloc(size)
#define jsi_default_realloc(ptr, size) sf_realloc(ptr, size)
#define jsi_default_free(ptr) sf_free(ptr)

#define JSI_IMPL

#include "json_input.cpp.h"
