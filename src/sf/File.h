#pragma once

#include <stdio.h>
#include "String.h"
#include "Array.h"

namespace sf {

struct FileInfo
{
	sf::StringBuf name;
	bool isDirectory;
};

FILE *stdioFileOpen(sf::String name, const char *mode);

bool fileExists(sf::String name);

bool deleteFile(sf::String name);
bool replaceFile(sf::String dst, sf::String src);

bool readFile(sf::Array<char> &data, sf::String name);
bool writeFile(sf::String name, const void *data, size_t size);
sf_inline bool writeFile(sf::String name, sf::Slice<const char> slice) {
	return writeFile(name, slice.data, slice.size);
}

bool isDirectory(sf::String name);
bool createDirectory(sf::String name);
bool createDirectories(sf::String name);

bool listFiles(sf::String path, sf::Array<FileInfo> &files);

uint64_t getFileTimestamp(sf::String path);

void appendPath(sf::StringBuf &path, sf::String a);
void appendPath(sf::StringBuf &path, sf::String a, sf::String b);
void appendPath(sf::StringBuf &path, sf::String a, sf::String b, sf::String c);
void appendPath(sf::StringBuf &path, sf::String a, sf::String b, sf::String c, sf::String d);

bool containsDirectory(sf::String path, sf::String dir);

struct DirectoryMonitor
{
	DirectoryMonitor();
	~DirectoryMonitor();

	DirectoryMonitor(DirectoryMonitor &&rhs);
	DirectoryMonitor &operator=(DirectoryMonitor&& rhs);

	DirectoryMonitor(const DirectoryMonitor&) = delete;
	DirectoryMonitor &operator=(const DirectoryMonitor&) = delete;

	void begin(sf::String root);
	void end();

	void getUpdates(sf::Array<sf::StringBuf> &paths);

	struct Data;
	Data *data;
};

}
