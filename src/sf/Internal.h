#pragma once

#include "Base.h"
#include "Array.h"
#include "String.h"

namespace sf {

#if SF_OS_WINDOWS

bool win32Utf8To16(sf::Array<wchar_t> &arr, sf::String str);
bool win32Utf16To8(sf::StringBuf &str, const wchar_t *strW, int length=-1);

#endif

}
