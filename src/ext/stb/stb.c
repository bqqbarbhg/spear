#include "sf/Platform.h"

#define STBI_ASSERT(cond) sf_assert(cond)
#define STBIR_ASSERT(cond) sf_assert(cond)
#define STBI_NO_STDIO

#define STBTT_malloc(x,u) ((void)(u),sf_malloc(x))
#define STBTT_free(x,u) ((void)(u),sf_free(x))

#define STBI_MALLOC(sz) sf_malloc(sz)
#define STBI_REALLOC(p,newsz) sf_realloc(p,newsz)
#define STBI_FREE(p) sf_free(p)

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_resize.h"
#include "stb_truetype.h"
