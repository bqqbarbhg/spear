#include "Internal.h"

#if SF_OS_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

namespace sf {

#if SF_OS_WINDOWS

bool win32Utf8To16(sf::Array<wchar_t> &arr, sf::String str)
{
	arr.clear();
	if (str.size == 0) {
		arr.push((wchar_t)0);
		return true;
	}

	int num = MultiByteToWideChar(CP_UTF8, 0, str.data, (int)str.size, arr.data, arr.capacity);
	if (num < 0) {
		arr.clear();
		return false;
	}

	if (num >= (int)arr.capacity) {
		if (arr.capacity > 0) {
			num = MultiByteToWideChar(CP_UTF8, 0, str.data, (int)str.size, nullptr, 0);
			if (num < 0) {
				arr.clear();
				return false;
			}
		}
		arr.reserve(num + 1);
		num = MultiByteToWideChar(CP_UTF8, 0, str.data, (int)str.size, arr.data, arr.capacity);
		if (num < 0) {
			arr.clear();
			return false;
		}
	}
	sf_assert(num < (int)arr.capacity);
	arr.size = num + 1;
	arr.data[num] = (wchar_t)0;
	return true;
}

bool win32Utf16To8(sf::StringBuf &str, const wchar_t *strW, int lengthW)
{
	if (lengthW == 0) {
		str.clear();
		return false;
	}

	int num = WideCharToMultiByte(CP_UTF8, 0, strW, lengthW, str.data, (int)(str.capacity ? str.capacity + 1 : 0), NULL, NULL);
	if (num < 0) {
		str.clear();
		return false;
	}
	if (lengthW < 0) num--;

	if (num >= (int)str.capacity) {
		if (str.capacity > 0) {
			num = WideCharToMultiByte(CP_UTF8, 0, strW, lengthW, NULL, 0, NULL, NULL);
			if (num < 0) {
				str.clear();
				return false;
			}
			if (lengthW < 0) num--;
		}
		if (num <= 0) {
			str.clear();
			return true;
		}

		str.reserveGeometric(num);
		num = WideCharToMultiByte(CP_UTF8, 0, strW, lengthW, str.data, (int)str.capacity + 1, NULL, NULL);
		if (num < 0) {
			str.clear();
			return false;
		}
		if (lengthW < 0) num--;
	}
	sf_assert((uint32_t)num <= str.capacity);
	str.size = num;
	str.data[num] = '\0';
	return true;
}

#endif

}
