#define STB_IMAGE_IMPLEMENTATION

#include "sf/Platform.h"

#define STBI_ASSERT(cond) sf_assert(cond)
#define STBI_NO_STDIO

#include "stb_image.h"
