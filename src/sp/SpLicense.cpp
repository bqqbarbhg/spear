#include "SpLicense.h"

#include "ext/sp_tools_common.h"
#include "sp/Json.h"

namespace sp {

// Generated by misc/gen-licenses.py sp-licenses.txt
static const uint32_t uncompressedSize = 7501;
static const char compressedLicenses[] =
        "\x28\xb5\x2f\xfd\x60\x4d\x1c\x1d\x43\x00\xfa\x4e\x54\x0e\x2b\xd0\x6e\xa8\x0a\xe8\xa0\x15\xa3\xc9\xea\x3b\xd2\x6e\xe4\x35\x7f\x64"
        "\xc1\x72\x82\x21\xd1\x29\x96\x55\x52\xe8\xeb\x81\x6e\xe5\xa4\xa5\x10\xa8\x1d\x93\x07\xf8\xe2\x01\xbe\xf7\xd1\x00\xe6\x00\xda\x00"
        "\x67\xac\x99\xda\xed\x6c\xfd\x62\x2b\xb5\x57\x4a\x86\xec\x8c\xb2\xf2\x8d\xd9\x2b\x7d\xa5\x03\x6e\x73\xb6\x5d\x14\x3b\x5f\x1b\xd6"
        "\x92\x51\x39\x93\x71\x5e\x58\xcf\xdf\x4a\xc7\x97\xbc\xb4\x2b\xa8\x15\x5b\xc6\x7b\xed\x86\xd1\x6b\x42\x6f\x2f\xe0\x0b\x2e\x67\xb7"
        "\x12\x6b\x7a\x9d\xd2\xd2\x42\x27\xe7\x8b\xc9\x0a\x5a\xb5\x63\x7c\xaf\xcc\x2a\x6b\xe7\xca\xc5\x4b\x4e\x24\x46\xb3\xdf\x8d\x56\xfa"
        "\xca\x95\x96\x3b\x9b\x55\x3a\xf9\xa5\xc6\x96\x16\x5f\x75\xa3\xf6\x2d\xff\x37\x95\x77\xb3\x64\xbd\x56\x62\x2c\x75\xd5\x15\x03\xe3"
        "\x8a\x7a\xfb\x39\x99\x33\xae\x86\x77\x14\x94\x5a\x7c\x45\xc7\xdd\x6d\xb6\x5c\xb5\x07\x04\x05\x08\x40\xdc\xe1\x6a\xf8\xad\x50\x64"
        "\xbd\x68\xb4\xb9\x16\x78\xc2\xe2\xf2\x9c\xb4\x79\x77\x9c\x58\x48\xb0\x14\x1d\x2b\x6c\xb7\x38\x9b\x07\x03\x77\xa6\x86\x76\xb9\x8b"
        "\xbb\x05\x56\x23\xd6\xf1\xda\x08\x8b\x2f\x58\x70\x16\x7b\x81\x41\x42\x52\x04\x45\x0e\x23\xe1\x00\x45\x90\x8a\x48\x24\xe9\x51\x70"
        "\x62\x30\x31\x94\x7b\x93\x64\x8a\x9e\xae\x18\x4c\xaf\x81\x0f\xa4\x37\x39\x51\x14\xf1\x47\x49\x28\x48\x4d\x0d\xc9\x04\x79\x92\xa4"
        "\x69\xe8\x6a\x7a\x11\x0c\x84\x71\x2e\x71\xf0\x26\x37\x21\x31\x0f\x36\x2f\x32\xdd\x3d\x0d\xd5\x3c\x09\x3a\xef\x69\x30\xaf\xf1\x3c"
        "\xcc\x63\x3a\x05\xbd\x07\xef\x0c\xe7\x41\x20\xf3\x34\x13\x74\xba\xfa\x9b\xa2\x88\x24\x89\x5c\x62\x1e\x49\x03\x29\x89\x89\x9c\x71"
        "\x40\x14\x74\x4c\x50\xf0\x81\x0d\xe6\xd1\x48\x0f\x72\x47\xf4\xae\x5c\x44\x51\x10\x02\x9f\x53\x11\xf4\xbb\x08\x08\x3a\x92\xa7\x81"
        "\xe0\xdd\xf4\x20\x05\x31\x12\x7c\x22\x10\x78\x29\xfa\x34\x3d\x29\x02\xe1\x7d\x02\x62\xba\x5a\x54\x13\xb3\xa8\x22\x4a\xd2\x80\x12"
        "\x05\x39\x10\xf3\x49\x7a\xce\x13\x3d\x4e\x3d\x22\xa0\x34\x11\x89\x9a\xbf\x07\xf5\x89\xc6\x9d\xe7\xc0\x07\x91\x14\xf9\x93\x18\x09"
        "\xca\x11\x4d\x9f\xe0\x83\xb2\xb6\xab\xbc\x58\xb3\x96\x3b\xa1\x76\x52\x81\x9d\x2f\xcd\x2a\x0d\x0f\x36\x06\x90\xf3\x14\x3a\x27\xd6"
        "\x4a\x96\x38\x57\xac\x19\x4a\x6b\x03\x2c\x39\x2f\x4c\x4a\x41\x27\x05\x10\x4a\x6a\x78\xe5\x46\xc8\x39\x93\x77\x46\x96\x1e\xe6\x39"
        "\xff\x83\x22\x48\x24\x39\x5c\xc3\x38\x49\x22\x39\x20\x08\x39\x50\x57\xae\x2b\xd7\xe0\x41\xc1\x57\xa9\xf6\x5b\x23\x5b\x9f\x33\x56"
        "\x2c\x75\x2f\xa6\x40\xed\x29\xc6\x5d\xb7\x49\x46\x2b\x16\x2b\x27\x5b\x67\x5d\xc5\xda\x34\x76\x8e\x5e\x99\xc9\x89\x6d\xf3\x6a\xf9"
        "\x8b\xeb\xd4\xd5\x8c\x8c\x39\x2c\xaa\x69\x51\x35\xdc\xf2\x94\x93\x47\x79\x51\x6b\xe3\x84\x76\x15\x2d\x6a\x2f\x94\x52\x63\x85\xb9"
        "\x4e\xee\x30\xbe\x1d\x2b\xe5\x17\x8b\x6f\xe5\x23\x07\xf0\x27\x60\xdb\x7c\xf1\xbd\x5c\xb9\xb8\x3a\x6e\xe3\xa4\x95\x4b\x42\x0e\x4f"
        "\xc3\x34\x54\x4b\x3e\x36\xcb\x64\x33\x77\x19\x16\x8c\x47\x03\x23\xf0\x81\xc3\xdc\x01\xef\xa0\x84\x91\x98\xa0\xc3\x9b\xa0\x44\xf4"
        "\x1a\xe8\x45\xc0\x27\xe2\x41\xa7\xa8\x66\x62\xa4\xf7\x90\xb8\x9b\x08\x13\xde\xd5\xcf\xa8\x8c\xf5\x56\xb7\xbb\x4d\x2e\x57\x6e\x97"
        "\xb5\x8a\x6d\x79\xc5\x5f\xcb\x5a\xbe\x6c\x95\x8c\x75\x15\xaf\x9c\x70\xb3\x45\x21\xf1\xae\x6c\xcb\x09\x75\xd6\x12\xe7\x05\xd9\x14"
        "\x8a\x59\x59\x37\x0a\x36\x7b\x68\xda\x2e\xde\xb6\xbc\x73\x4e\xcb\x18\x98\xc6\x36\xf2\xe4\x6a\x35\xb7\x95\x97\x6b\x8d\x8d\x77\xe4"
        "\x29\xa3\xce\x23\xf3\x0f\x3c\x6f\x5d\x7b\x02\x10\x04\x20\x1e\x0e\x38\x34\xfc\x6e\x5d\x69\xf3\x48\x6e\x94\x0c\xbe\x46\x76\x2a\x68"
        "\xc6\xf5\x62\xa0\x72\x31\x45\x8d\xd9\x2f\xb3\x35\x5b\xb9\x02\x63\xe6\x0c\x08\x86\xdc\x2a\x70\xe5\x8a\xcd\x8d\x5a\x28\x18\x4f\x0b"
        "\x6d\x8a\x2b\x7e\x02\x3f\xe3\xa5\xd5\xb2\xd1\xcb\x19\x30\xd7\x96\xf4\x8c\x02\xff\x7f\x99\x5d\x71\xbd\x6c\xa7\xa6\xc5\x2b\xa3\x17"
        "\x5b\xaa\xd5\x4d\xe3\xa6\x8c\x5b\x59\xe3\x2d\x53\x62\x16\xb1\x3a\xad\x88\x4d\xae\xd8\xba\xcd\x59\xc1\x87\x17\x39\xb5\x4c\x5a\x60"
        "\x17\xc0\xca\x82\x2c\xa8\x82\xb3\x9a\x93\x0c\xd0\x00\x0c\x00\x00\x40\x00\x22\x18\x00\x04\x43\x43\xa2\x26\x55\xdf\x01\x72\x01\x45"
        "\xc5\x91\x86\x19\x66\x00\xc1\x00\x04\x00\x00\x04\x00\x88\x88\x08\x50\x04\x05\xcc\x51\xc5\x7c\xdf\xcc\x03\xbe\x67\x91\xf4\x80\x6f"
        "\x3d\xc7\x5d\x2b\xa4\xf6\x18\xd4\xaf\x21\x12\x43\x21\x88\xd9\x54\xe0\x9a\x14\x48\x08\x62\x6b\xb8\x8a\x76\x73\x8a\x6d\x67\x01\x1b"
        "\x44\x90\x13\x7e\x02\xe9\x50\x35\x2b\x98\xc4\xb8\xe1\xbc\x44\x85\xeb\x23\xd5\x30\xbd\x81\xe6\x6c\x14\xb9\x91\x43\xdd\xe1\x7d\xc3"
        "\xa1\x2f\xad\x51\xb2\x28\x89\x39\x97\xdd\xaa\x63\x90\xcc\x59\x50\xca\x88\x3f\xfd\xe4\x0c\x21\xb6\x76\x9f\xb6\x7c\x6c\x87\x26\x9d"
        "\x4f\x38\x16\x0a\x84\xdf\xaf\xee\x29\x6c\x32\x76\xaf\xe0\x55\x78\x38\xfd\xa3\x95\x28\x99\x29\x83\x8a\x1b\xa8\x24\x8e\xcd\xb0\xc3"
        "\xaf\x50\x7b\x60\xd1\x63\x82\xf9\x82\xb5\xe6\x5d\xab\x35\xec\x5f\xed\x73\x38\x90\x20\x54\x48\xc1\xb1\x6c\xbc\xe6\x84\xbe\x2f\x40"
        "\xec\x9d\x8c\x19\x3c\xb5\xdb\x76\xf9\xf1\x2d\x3f\xa5\xe6\x8f\x89\xc7\x78\x79\x41\xa6\xda\xff\x34\xb9\x2b\xa5\x22\xd8\x44\xa6\x13"
        "\xfe\xde\x7a\xf4\x08\x50\x8e\xeb\x51\x14\x5d\x55\x32\x7f\x42\x4b\x5d\x13\x61\x1b\x0b\xa2\x67\xf0\xfb\x1b\xb5\xf5\xa2\x8f\x97\x8e"
        "\x09\xe8\x4e\xf9\x22\x4b\x76\x05\xe7\xf6\x45\x13\xb2\x0d\x03\x67\xdd\x10\x1a\x21\xb6\x60\x14\x2a\xfa\x5e\x80\x4c\x25\xa2\xde\xe2"
        "\x21\x33\x0f\x43\x5e\xc5\x15\x23\x3c\xd7\x1e\x1e\x6f\x42\x56\x88\xd1\xde\x51\x8c\x50\xcd\x56\x0a\x22\xb2\x61\x12\x09\xaa\x5b\xce"
        "\xb9\xc0\xf4\x9c\x4c\x49\xf0\x64\xe2\xa1\x28\x83\x60\xf8\x8f\x4c\xda\x0d\x8b\x47\x95\x70\xa0\xcb\x57\x3d\xce\x15\x9c\x69\x37\x30"
        "\x44\x6f\x36\x3a\x92\xa9\x68\x80\x81\x64\x51\xed\xd7\x1c\xcc\xaa\x21\x2d\x72\x0a\x27\x80\xd5\xb0\x0b\xb2\x38\x25\x0a\x2f\x38\x07"
        "\x4c\x36\x70\xbf\xc5\x17\xa4\xea\x15\x59\x4a\xda\x72\x42\x46\x24\x22\x52\x1a\x28\x83\x34\x62\xa2\x87\xff\x99\xa0\x7c\xcc\xf0\xd8"
        "\xeb\xf7\x02\xf8\xa9\x23\xd5\x01\xaa\x02\xb5\x67\x8d\x30\x6d\x05\x0d\x80\x30\x16\xf3\xe7\xeb\xbe\xd8\x65\xce\xc8\x29\x86\x8c\xa1"
        "\xc7\x4d\xf7\xa4\x1c\x70\x1d\x67\x45\x13\xda\x58\x16\x66\x85\x17\xc3\x28\x0c\xba\xc0\xcc\x9b\x37\xbf\xf2\x94\xf9\xf5\x58\xa8\xf2"
        "\xc4\x56\xd9\x02\x34\xba\x14\x09\x23\x52\xfb\x46\x2f\x8c\x15\x56\x31\xfb\x25\x6c\x72\x4e\xe3\x8f\x60\x4a\xc3\xbd\x67\x96\x12\x89"
        "\x6f\x98\x8d\xcb\x5a\x03\x81\xcb\x59\x18\xba\x4b\x87\x86\x87\x32\xac\xce\x5f\x2c\x3f\xb8\xc6\xfe\xd0\x23\x0e\x33\x89\x66\x55\x92"
        "\xce\x0b\x9d\x28\xf0\x5c\x0a\x82\xf2\xe1\x75\x52\x52\x08\xab\x10\xd8\x85\xdf\xb1\xd8\xe0\x8e\x0e\x6e\x84\x9c\x7e\xd0\x91\x02\x11"
        "\x55\x7d\x85\xa4\x71\x31\xe7\x7a\xae\x5d\xc1\xb0\xa9\x32\x39\xf4\xcf\x5c\x40\x2c\xd1\x41\xff\x00\x29\xda\xcd\x7b\x53\x6c\xeb\x23"
        "\x1f\x94\x88\x4b\xac\x0c\xaa\x26\xc8\xb1\xb5\xcd\x3a\x4f\x9a\x7d\xad\x26\x30\x91\x69\x26\x91\xea\x91\x27\xdd\xab\xee\xdf\x98\xa6"
        "\x79\xc8\xa3\x37\x52\xf3\x7b\xf9\xf3\x09\x7f\x9c\x41\xfb\x6b\x0e\x2a\x47\xdd\x83\xf5\x4e\x9d\x7c\x90\xdf\xf3\x59\xe7\xc9\xfb\x08"
        "\xdb\x25\x04\xe1\x66\x62\x0b\xa8\xbd\x27\x89\xbc\xa0\x9c\x95\x3c\xd3\xf6\x44\x35\x47\x8b\x7b\x08\x27\xc2\xe8\x4a\x54\x40\x5e\xbf"
        "\xea\x7d\xc9\x4e\x81\x4c\xe3\xc6\x86\xef\xf1\xe6\x71\x67\xf4\x74\x7f\x59\xb2\xec\x07\x37\x05\x01\xe8\xd6\xa3\xed\x68\xa4\x41\x2b"
        "\x67\x3a\x7d\x89\x61\xfb\x02\x70\xf6\xd9\x76\x7f\x28\xfe\x75\xb2\x57\x75\x9e\x80\x15\xe4\xf4\xc8\x97\x3a\xb9\x00\x4a\x34\x8a\x78"
        "\x72\x51\xad\xf7\xe2\x2a\x3d\xd4\x92\x57\x06\x76\xfc\x15\x58\x6c\xfe\xf1\xc4\x9a\x14\xae\xa9\xeb\x15\xc8\xcb\x38\xa9\xcb\xcc\xe4"
        "\x20\x30\x24\xc5\x30\x78\x8b\x93\xe9\x4e\x2c\x4c\x07\x2d\xf1\xc1\xe3\xb7\xcc\xde\xc4\x93\x4b\xe3\xf9\x08\x7a\x6f\xb2\xc4\xef\x25"
        "\x32\x69\x4a\x04\xa6\xe8\x59\xe6\x9c\xdc\x17\xd3\x9e\x14\xf7\x9f\xd3\xaf\xeb\x90\x4d\x02\x8e\x90\xdc\xe3\xdd\x43\x9d\xbf\x24\x7c"
        "\x0c\xac\xd1\xaf\x7a\x1f\xa7\xb5\x00\xdb\x73\x7c\x94\x2f\xab\xe2\x64\xcc\xc3\xe2\xe5\xee\x2e\x8c\x13\x19\xb5\x13\x4a\x95\x05\x6c"
        "\xd5\x2f\x52\x44\x63\x44\x09\xcc\x99\x25\x94\x62\x98\x58\xdf\xb1\x9e\xc2\x7d\x41\x91\x8c\x4e\xc9\x8e\x73\x2e\x3b\xf6\x57\xaa\xbb"
        "\x03\x97\x01\x54\xdb\x09\x9e\xf8\xc4\xe5\xef\x32\xdc\xa1\xec\x23\x3c\x6d\x42\x0d\xa4\x87\xfb\x39\x08\xd8\x64\xcf\x70\x13\x0d\x58"
        "\x30\x03\xa5\x27\xd6\x32\x0d\x26\x6e\x50\x1e\x4b\xe2\x50\x0b\xed\x19\x54\x85\x34\x1f\xee\x98\x4a\x24\xbb\x13\xbc\x4d\xe0\xc6\xb8"
        "\x2b\x32\x10\xf7\xbd\xdc\xb1\x4e\x78\x8b\x90\x43\x58\x44\x7d\xeb\x68\x3b\x3b\x86\xad\xb6\xef\xa8\x25\x7f\xc3\x94\x29\xf7\xe0\xf9"
        "\x54\x2a\x1e\x9a\xde\xf3\x07\xd5\xe7\x19\x26\x7c\x85\x34\xfa\x72\x70\xf0\xe1\xa1\x88\x45\x22\x5f\xe8\xad\x19\x18\x9e\xeb\xdc\x18"
        "\xa7\xf8\x85\x05\xe3\xd4\x60\x69\xda\xeb\xc3\x2d\x6c\x16\x5e\x3d\x03\x14\xcc\x6a\x2f\xcb\x09\xd2\x74\x8b\x91\xc7\xa4\x3d\xc6\xd8"
        "\x01\x5c\xb2\x31\x8e\x30\xb4\xb2\x9b\x43\x75\x10\x32\x18\xb1\x04\x9a\x06\xcb\x42\x40\x8c\x91\x86\x58\xb5\x1a\x6d\xb3\xc7\x00\xc0"
        "\xa1\x95\x53\xc1\xb4\x95\x24\x71\x50\xd3\x48\xeb\x22\xd7\x84\x51\xbf\x50\x84\x54\x1c\x24\x8b\x49\xb9\xc0\xa6\x60\x49\x99\x9b\xd9"
        "\xd7\x52\x89\xd9\x33\xab\x5e\x26\x17\xbc\x14\xc1\x87\xc9\x35\x01\x22\xf6\xf9\xc7\x68\x2f\xd7\x81\xc7\x43\x23\xa8\x44\xa0\xbb\x56"
        "\x5a\x5d\xe2\xeb\xdc\x85\xc2\xf5\xe0\x8f\x39\x0c\x3d"
;

void getLicenses(sf::Array<sf::License> &licenses)
{
	sf::SmallArray<sf::License, 32> localLicenses;

    sf::Array<char> decompressed;
    decompressed.resizeUninit(uncompressedSize);
	size_t realSize = sp_decompress_buffer(SP_COMPRESSION_ZSTD, decompressed.data, decompressed.size, compressedLicenses, sizeof(compressedLicenses) - 1);
    sf_assert(realSize == decompressed.size);

    jsi_value *val = jsi_parse_memory(decompressed.data, decompressed.size, nullptr);
    sf_assert(val);
    sp::readJson(val, localLicenses);
    jsi_free(val);

    for (sf::License &l : localLicenses) {
		licenses.push(std::move(l));
    }
}

}