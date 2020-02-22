#include "File.h"

#include "Array.h"
#include "Internal.h"

#if SF_OS_WINDOWS
	#define _WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#else
	#include <sys/stat.h>
#endif

namespace sf {

FILE *stdioFileOpen(sf::String name, const char *mode)
{
	sf_assert(strlen(mode) < 8);
#if SF_OS_WINDOWS
	wchar_t wMode[8];
	sf::SmallArray<wchar_t, 256> nameBuf;

	for (size_t i = 0; i < sf_arraysize(wMode); i++) {
		wMode[i] = (wchar_t)mode[i];
		if (mode[i] == '\0') break;
	}
	if (!win32Utf8To16(nameBuf, name)) return nullptr;

	FILE *file;
    errno_t err = _wfopen_s(&file, nameBuf.data, wMode);
	return err == 0 ? file : nullptr;
#else
	sf::SmallStringBuf<512> nameBuf = name;
	return fopen(nameBuf.data, mode);
#endif
}

bool deleteFile(sf::String name)
{
#if SF_OS_WINDOWS
	sf::SmallArray<wchar_t, 256> nameBuf;
	if (!win32Utf8To16(nameBuf, name)) return false;
	return DeleteFileW(nameBuf.data) != 0;
#else
	sf::SmallStringBuf<512> nameBuf = name;
	return remove(nameBuf.data) == 0;
#endif
}

bool replaceFile(sf::String dst, sf::String src)
{
	if (dst == src) return true;

#if SF_OS_WINDOWS
	sf::SmallArray<wchar_t, 256> dstBuf, srcBuf;
	if (!win32Utf8To16(dstBuf, dst)) return false;
	if (!win32Utf8To16(srcBuf, src)) return false;
	return MoveFileExW(srcBuf.data, dstBuf.data, MOVEFILE_REPLACE_EXISTING) != 0;
#else
	sf::SmallStringBuf<512> srcBuf = src;
	sf::SmallStringBuf<512> dstBuf = dst;
	return rename(srcBuf.data, dstBuf.data) != 0;
#endif
}

bool readFile(sf::Array<char> &data, sf::String name)
{
	FILE *f = stdioFileOpen(name, "rb");
	if (!f) return false;

	data.clear();

	size_t begin = ftell(f);
	fseek(f, 0, SEEK_END);
	size_t end = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (ferror(f) == 0) {
		data.reserve(end - begin);
	} else {
		// Failed to seek, re-open without reserve
		fclose(f);
		f = stdioFileOpen(name, "rb");
	}


	while (!ferror(f) && !feof(f) && data.size < data.capacity) {
		size_t numRead = fread(data.data, 1, data.capacity - data.size, f);
		data.size += (uint32_t)numRead;
	}

	sf::SmallArray<char, 64> extra;
	while (!ferror(f) && !feof(f)) {
		size_t numRead = fread(extra.data, 1, extra.capacity - extra.size, f);
		extra.size += (uint32_t)numRead;
		if (extra.size == extra.capacity) {
			extra.reserve(extra.size * 2);
		}
	}
	if (extra.size > 0) {
		data.push(extra);
	}

	int err = ferror(f);
	int ret = fclose(f);

	if (err != 0 || ret != 0) {
		return false;
	}

	return true;
}

bool writeFile(sf::String name, const void *data, size_t size)
{
	FILE *f = stdioFileOpen(name, "wb");
	if (!f) return false;

	size_t num = fwrite(data, 1, size, f);
	int err = ferror(f);
	int ret = fclose(f);

	if (num < size || ret != 0 || err != 0) {
		deleteFile(name);
		return false;
	}

	return true;
}

bool createDirectory(sf::String name)
{
#if SF_OS_WINDOWS
	sf::SmallArray<wchar_t, 256> nameBuf;
	if (!win32Utf8To16(nameBuf, name)) return false;
	return CreateDirectoryW(nameBuf.data, NULL) != 0;
#else
	sf::SmallStringBuf<512> nameBuf = name;
	mkdir(nameBuf.data, 0777);
#endif
}

}
