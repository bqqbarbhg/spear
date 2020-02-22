#pragma once

#include <stdio.h>
#include "sf/String.h"
#include "sf/Array.h"

namespace sf {

FILE *stdioFileOpen(sf::String name, const char *mode);

bool deleteFile(sf::String name);
bool replaceFile(sf::String dst, sf::String src);

bool readFile(sf::Array<char> &data, sf::String name);
bool writeFile(sf::String name, const void *data, size_t size);
sf_inline bool writeFile(sf::String name, sf::Slice<const char> slice) {
	return writeFile(name, slice.data, slice.size);
}

}
