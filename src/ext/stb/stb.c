#include "sf/Platform.h"

#define STBI_ASSERT(cond) sf_assert(cond)
#define STBIR_ASSERT(cond) sf_assert(cond)
#define STBI_NO_STDIO

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_resize.h"
#include "stb_truetype.h"
