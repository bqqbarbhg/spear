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
	sf::SmallStringBuf<512> nameBuf(name);
	return fopen(nameBuf.data, mode);
#endif
}

bool fileExists(sf::String name)
{
	FILE* f = stdioFileOpen(name, "rb");
	if (f) {
		fclose(f);
		return true;
	} else {
		return false;
	}
}

bool deleteFile(sf::String name)
{
#if SF_OS_WINDOWS
	sf::SmallArray<wchar_t, 256> nameBuf;
	if (!win32Utf8To16(nameBuf, name)) return false;
	return DeleteFileW(nameBuf.data) != 0;
#else
	sf::SmallStringBuf<512> nameBuf(name);
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
	sf::SmallStringBuf<512> srcBuf(src);
	sf::SmallStringBuf<512> dstBuf(dst);
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
	sf::SmallStringBuf<512> nameBuf(name);
	return mkdir(nameBuf.data, 0777) == 0;
#endif
}

void createDirectories(sf::String name)
{
	for (size_t i = 0; i < name.size; i++) {
		if (name.data[i] == '/' || name.data[i] == '\\') {
			createDirectory(sf::String(name.data, i));
		}
	}
}

bool listFiles(sf::String path, sf::Array<FileInfo> &files)
{
#if SF_OS_WINDOWS
	sf::SmallArray<wchar_t, 256> nameBuf;
	if (!win32Utf8To16(nameBuf, path)) return false;
	if (nameBuf.size <= 1) return false;
	nameBuf.pop();
	if (nameBuf.back() != '/' && nameBuf.back() != '\\') nameBuf.push('\\');
	nameBuf.push('*');
	nameBuf.push('\0');

	WIN32_FIND_DATAW findData;
	HANDLE findHandle = FindFirstFileW(nameBuf.data, &findData);
	if (findHandle == INVALID_HANDLE_VALUE) {
		return GetLastError() == ERROR_FILE_NOT_FOUND;
	}

	do {
		if (!wcscmp(findData.cFileName, L".") || !wcscmp(findData.cFileName, L"..")) continue;

		FileInfo &info = files.push();

		sf::win32Utf16To8(info.name, findData.cFileName);
		info.isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

	} while (FindNextFileW(findHandle, &findData));

	FindClose(findHandle);

	return true;
#else
	return false;
#endif
}

void appendPath(sf::StringBuf &path, sf::String a)
{
#if SF_OS_WINDOWS
	char sep = '\\';
#else
	char sep = '/';
#endif

	path.reserveGeometric(path.size + 1 + a.size);
	if (path.size > 0) {
		char last = path.data[path.size - 1];
		if (last != '\\' && last != '/') {
			path.append(sep);
		}
	}
	if (path.size > 0 && a.size > 0 && (a.data[0] == '\\' || a.data[0] == '/')) {
		path.append(a.slice().drop(1));
	} else {
		path.append(a);
	}
}

uint64_t getFileTimestamp(sf::String path)
{
#if SF_OS_WINDOWS
	sf::SmallArray<wchar_t, 256> nameBuf;
	if (!win32Utf8To16(nameBuf, path)) return false;
	HANDLE h = CreateFileW(nameBuf.data, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) return 0;

	FILETIME lastWrite;
	GetFileTime(h, NULL, NULL, &lastWrite);
	return (uint64_t)lastWrite.dwHighDateTime << 32u | (uint64_t)lastWrite.dwLowDateTime;
#else
	return 0;
#endif
}

void appendPath(sf::StringBuf &path, sf::String a, sf::String b)
{
	path.reserveGeometric(path.size + 2 + a.size + b.size);
	appendPath(path, a);
	appendPath(path, b);
}

void appendPath(sf::StringBuf &path, sf::String a, sf::String b, sf::String c)
{
	path.reserveGeometric(path.size + 3 + a.size + b.size);
	appendPath(path, a);
	appendPath(path, b);
	appendPath(path, c);
}

void appendPath(sf::StringBuf &path, sf::String a, sf::String b, sf::String c, sf::String d)
{
	path.reserveGeometric(path.size + 4 + a.size + b.size);
	appendPath(path, a);
	appendPath(path, b);
	appendPath(path, c);
	appendPath(path, d);
}

}
