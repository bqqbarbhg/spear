#include "SpLicense.h"

#include "ext/sp_tools_common.h"
#include "sp/Json.h"

namespace sp {

// Generated by misc/gen-licenses.py sp-licenses.txt
static const uint32_t uncompressedSize = 8688;
static const char compressedLicenses[] =
        "\x28\xb5\x2f\xfd\x60\xf0\x20\x25\x44\x00\x9a\x50\xa0\x0e\x2b\xd0\x6e\xa8\x0a\xe8\xa0\x15\xa3\xc9\xea\x3b\xd2\x6e\xe4\x35\x7f\x64"
        "\xc1\x72\x82\x21\xd1\x29\x96\x55\x52\xe8\xeb\x81\x6e\xe5\xa4\xa5\x10\xa8\x1d\x93\x07\xf8\xe2\x01\xbe\xf7\xd6\x00\xec\x00\xdf\x00"
        "\x7c\x69\x56\xe9\x19\x6b\xa6\x76\x3b\x5b\xbf\xd8\x4a\xed\x95\x92\x21\x3b\xa3\xac\x7c\x63\xf6\x4a\x5f\xe9\x80\xdb\x9c\x6d\x17\xc5"
        "\xce\xd7\x86\xb5\x64\x54\xce\x64\x9c\x17\xd6\xf3\xb7\xd2\xf1\x25\x2f\xed\x0a\x6a\xc5\x96\xf1\x72\xfd\xb5\x1b\x46\xaf\x09\xbd\x80"
        "\x2f\xb8\x9c\xdd\x4a\xac\xe9\x75\x4a\x4b\x0b\x9d\x9c\x2f\x26\x2b\x68\xd5\x8e\xf1\xbd\x32\xab\xac\x9d\x2b\x17\x2f\x39\x91\x18\xcd"
        "\x7e\x37\x5a\xe9\x2b\x57\x5a\xee\x6c\x56\xe9\xe4\x97\x1a\x5b\x5a\x7c\xd5\x8d\xda\xb7\xfc\xdf\x54\xde\xcd\x92\xf5\x5a\x89\xb1\xd4"
        "\x55\x57\x0c\x8c\x2b\xea\xed\xe7\x64\xce\xb8\x1a\xde\x51\x50\x6a\xf1\x15\x1d\x77\xb7\xd9\x72\xd5\x1e\x10\x14\x20\x00\x71\x87\xab"
        "\xe1\xb7\x42\x91\xf5\xa2\xd1\xe6\x5a\xe0\x09\x8b\xcb\x73\xd2\xe6\xdd\x71\x62\x21\xc1\x52\x74\xac\xb0\xdd\xe2\x6c\x1e\x0c\xdc\x99"
        "\x1a\xda\xe5\x2e\xee\x16\x58\x8d\x58\xc7\x6b\x23\x2c\xbe\x60\xc1\x59\xec\x05\x06\x09\x01\xa7\xa8\x66\x62\xa4\xf7\x90\xb8\x9b\x90"
        "\x22\x28\x72\x18\x09\x07\x28\x82\x54\x44\x22\x49\x8f\x82\x13\x83\x89\xa1\xdc\x9b\x24\x53\xf4\x74\xc5\x60\x7a\x0d\x7c\x20\xbd\xc9"
        "\x89\xa2\x88\x3f\x4a\x42\x41\x6a\x6a\x48\x26\xc8\x93\x24\x4d\x43\x57\xd3\x8b\x60\x20\x8c\x73\x89\x83\x37\xb9\x09\x89\x79\xb0\x79"
        "\x91\xe9\xee\x69\xa8\xe6\x49\xd0\x79\x4f\x83\x79\x8d\xe7\x61\x1e\xd3\x29\xe8\x3d\x78\x67\x38\x0f\x02\x99\xa7\x99\xa0\xd3\xd5\xdf"
        "\x14\x45\x24\x49\xe4\x12\xf3\x48\x1a\x48\x49\x4c\xe4\x8c\x03\xa2\xa0\x63\x82\x82\x0f\x6c\x30\x8f\x46\x7a\x90\x3b\xa2\x77\xe5\x22"
        "\x8a\x82\x10\xf8\x9c\x8a\xa0\xdf\x45\x40\xd0\x91\x3c\x0d\x04\xef\xa6\x07\x29\x88\x91\xe0\x13\x81\xc0\x4b\xd1\xa7\xe9\x49\x11\x08"
        "\xef\x13\x10\xd3\xd5\xa2\x9a\x98\x45\x15\x51\x92\x06\x94\x28\xc8\x81\x98\x4f\xd2\x73\x9e\xe8\x71\xea\x11\x01\xa5\x89\x48\xd4\xfc"
        "\x3d\xa8\x4f\x34\xee\x3c\x07\x3e\x88\xa4\xc8\x9f\xc4\x48\x50\x8e\x68\xfa\x04\x1f\x94\xb5\x5d\xe5\xc5\x9a\xb5\xdc\x09\xb5\x93\x0a"
        "\xec\x01\x15\x5b\xb7\x39\x6f\x73\x05\x1f\x5e\xe4\xd4\x32\x69\x81\x5d\x00\x2b\x1f\x6c\x0c\x20\xe7\x29\x74\x4e\xac\x95\x2c\x71\xae"
        "\x58\x33\x94\xd6\x06\x58\x72\x5e\x98\x94\x82\x4e\x0a\x20\x94\xd4\xf0\xca\x8d\x90\x73\x26\xef\x8c\x2c\x3d\xcc\x73\xfe\x07\x45\x90"
        "\x48\x72\xb8\x86\x71\x92\x44\x72\x40\x10\x72\xa0\xae\x5c\x83\x07\x05\x5f\xa5\xda\x6f\x8d\x6c\x7d\xce\x58\xb1\xd4\xbd\x98\x02\xb5"
        "\xa7\x18\x77\xdd\x26\x19\xad\x58\xac\x9c\x6c\x9d\x75\x15\x6b\xd3\xd8\x39\x7a\x65\x26\x27\xb6\xcd\xab\xe5\xaf\xa9\xab\x19\x19\x73"
        "\x58\x54\xd3\xa2\x6a\xb8\xe5\x29\x27\x8f\xf2\xa2\xd6\xc6\x09\xed\x2a\x5a\xd4\x5e\x28\xa5\xc6\x0a\x73\x9d\xdc\x61\x7c\x3b\x56\xca"
        "\x2f\x16\xdf\xca\x47\x0e\xe0\x4f\xc0\xb6\xf9\xe2\x7b\xb9\x72\x71\x75\xdc\xc6\x49\x2b\x97\x84\x1c\x9e\x86\x69\xa8\x96\x7c\x6c\x96"
        "\xc9\x66\xee\x32\x2c\x18\x8f\x06\x46\xe0\x03\x87\xb9\x03\xde\x41\x09\x23\x31\x41\x87\x37\x41\x89\xe8\x35\xd0\x8b\x80\x4f\xc4\x83"
        "\x02\x13\xde\xd5\xcf\xa8\x8c\xf5\x56\xb7\xbb\x4d\x2e\x57\x6e\x97\xb5\x8a\x6d\x79\xc5\x5f\xcb\x5a\xbe\x6c\x95\x8c\x75\x15\xaf\x9c"
        "\x70\xb3\x45\x21\xf1\xae\xec\x84\x3a\x6b\x89\xf3\x82\x6c\x0a\xbd\x22\x66\x65\xdd\x28\xd8\xec\xa1\x69\xbb\x78\x1b\x5b\xde\x55\x7a"
        "\xe7\xb4\x8c\x81\x69\x6c\x23\x4f\xae\x56\x73\x5f\xae\x35\x36\xde\x91\xa7\x96\x31\xa3\xce\x23\xf3\x0f\x3c\x6f\x5d\x7b\x04\x9c\xf1"
        "\x9c\xc7\xb8\xa1\xd9\x05\xb9\x45\x64\xf0\x04\x20\x08\x40\x3c\x1c\x70\x68\xf8\xdd\xba\xd2\xe6\x91\xdc\x28\x19\x7c\x8d\xec\x54\xd0"
        "\x8c\xeb\xc5\x40\xe5\x62\x8a\x1a\xb3\x5f\x66\x6b\xb6\x72\x05\xc6\xcc\x19\x10\x0c\xb9\x55\xe0\xca\x15\x9b\x1b\xb5\x50\x30\x9e\x16"
        "\xda\x14\x57\xfc\x04\x7e\xc6\x4b\xab\x65\xa3\x97\x33\x60\xae\x2d\xe9\x19\x05\xfe\xff\x32\xbb\xe2\x7a\xd9\x4e\x4d\x8b\x57\x46\x2f"
        "\xb6\x54\xab\x9b\xc6\x4d\x19\xa7\xb4\xb4\xb2\xc6\x5b\xa6\xc4\x2c\x62\x3c\xad\x88\x4d\x0e\x82\x32\xa8\x92\xb3\x9a\x83\x8c\xd0\x00"
        "\x0c\x00\x00\x40\x00\x32\x18\x00\x04\x83\x43\xa2\xa6\xd4\xed\x01\x92\x01\x45\xc5\x49\xa3\x0c\x23\x50\x20\x00\x01\x00\x00\x08\x00"
        "\x22\x22\x02\x14\x41\x01\xcc\x51\xc5\x7c\xdf\xcc\x03\xfe\x67\x8c\xf4\x88\x77\x3d\xc5\x5d\x2b\xd4\x36\x18\xd4\x0f\xb6\x5a\x21\xb4"
        "\x3e\x01\xfb\xb0\xe7\x5b\x76\x04\x15\x19\x4d\x45\x36\x57\xaa\x6c\xfc\x94\x06\x19\xe1\x0c\x94\xf9\x1f\xca\x6d\x45\x26\x18\xb7\xc9"
        "\x31\xa8\xbb\xf6\xab\x2a\xb3\xa3\x3f\x03\x37\x5a\xba\x49\x60\xe9\xb0\xc5\xf0\xd3\x47\x32\xbd\x65\x94\xd2\x1c\xcc\xdd\x9a\x18\x63"
        "\xd3\x16\xdc\x32\xf7\x4f\x5f\x39\x74\x67\x47\xb0\x3e\x9a\xf9\xdc\xb9\x25\x2e\x8e\xac\x3f\x54\x66\x07\x3a\xec\xcf\xf6\x00\x1e\x19"
        "\x2b\x55\xfa\xaa\x21\x4d\x5d\xf5\x57\x23\x47\x57\xf3\xdd\x43\x88\x5c\x85\x7b\x92\x8d\x17\x83\x19\x1b\xd4\x45\x7a\x89\xd8\xeb\x2e"
        "\xd5\x1a\x0e\x8a\x0a\xf3\x25\xd7\xb9\x84\xeb\xb9\x19\x5f\xb7\x64\xb2\x38\x82\x54\x4c\x79\xac\xb9\x95\xfd\xb6\xed\xf7\x38\x22\xdf"
        "\x26\xf3\x3f\x2f\x7f\xcd\x0c\xf8\x1f\x16\xee\x90\xe0\x96\x4a\x1a\xe8\x78\x12\x24\x6c\x34\xb2\xa6\x8c\xa3\x52\x9c\x65\x55\x4b\x8a"
        "\x58\xef\x9e\x24\x89\x10\xe4\x99\xa7\x33\x32\xf4\x39\x0a\xaf\xea\x05\xaa\x05\x03\x34\x65\xf2\x2d\xd0\x2b\x84\x01\x62\xda\xc9\x16"
        "\x0b\x09\xd1\x19\x11\xc6\xce\x14\xae\x35\x65\x9f\x1b\x7e\xe4\x83\x4b\x28\x33\x1f\x42\x23\xc8\x62\x86\x92\x43\x6f\x3b\xec\x27\x11"
        "\xf8\x16\x19\x92\xcc\x47\xc9\x07\x8b\x32\x42\xa6\xed\x47\x70\x13\xdc\xf3\x87\xb6\x11\x05\x8c\x0a\xe4\x95\x87\xc8\x24\x66\x76\xc1"
        "\xa6\x25\x9d\x13\x4d\xf0\x93\xd9\xc4\x40\x27\x14\x8a\x72\x30\x05\xcf\xcb\x94\x6e\x80\x18\xb5\xe1\xd6\x9d\x6f\xfd\x71\xa7\x20\x49"
        "\x27\x00\x33\x9b\xb3\xf1\x47\xd6\x4e\x43\x66\xa4\x94\x1a\xbe\x8e\x61\xea\x0d\x69\xd6\xc9\x9e\x3c\x54\x23\x03\x64\x22\xa5\x02\x2f"
        "\x90\x07\x4c\x69\xe0\xcf\xc5\x07\x24\xe3\x95\xe5\x94\x60\xf5\xbc\x98\xc8\x5a\x54\x38\x10\x05\xa4\x62\x42\x9b\xff\xe9\x43\x82\xc8"
        "\xb8\xb0\xb9\xf7\x40\x38\xae\xe3\xd4\xe1\x56\x05\xda\xdb\x1a\x6d\xda\x30\x1a\x2d\x61\x78\xcd\x1f\x2a\xff\x22\x89\xb9\x26\x07\x16"
        "\x32\x8e\x9e\x9a\x4e\x49\x09\xb3\x3a\x80\x17\x4d\xe8\x60\x39\x9b\x15\x30\x0d\x13\x1f\x74\x3d\xb3\x60\x4e\xfc\xca\x50\xf6\xeb\xb1"
        "\x53\x35\x2e\x5b\x69\x2b\xd0\xdc\xa5\xe0\xc6\x29\xb2\x7d\x01\xab\xac\x21\x1e\xc7\x44\x45\xe3\x3a\x61\x14\xc3\x3d\x9a\x99\x42\xc3"
        "\x44\xb8\x32\x6b\x78\x3c\xbc\xa5\xd2\x6d\x34\x34\x6f\x28\x54\x76\xfc\x3f\xe6\x83\x7d\xc4\x1f\x5a\xc4\x63\x93\x0f\xa1\x8a\xe9\xec"
        "\x8e\x88\xe2\xaa\x4b\x41\x2c\xdf\xbc\x4e\xae\x08\x61\x15\x90\x3d\xf8\x1d\x8b\x8d\x0e\x74\x1e\xe3\x8d\xf4\x85\x2e\x07\x48\xa6\xe2"
        "\x29\x94\x40\xc2\x39\x5f\x73\xb0\x95\xad\x4d\x25\xc8\x61\xed\x7c\x0c\x39\x06\x9d\xe9\x1f\x60\x8a\xc0\x66\xa9\x29\x50\x79\x58\x07"
        "\xad\x62\x3d\x56\x90\x54\x4b\xe4\x68\xdf\x62\x9d\xf7\xa1\x7d\x4f\xa6\x30\xb1\x6d\x06\xc5\xea\x11\x27\x31\xa1\x86\xdf\x0e\x36\x2f"
        "\x81\x9f\x1b\xe8\x11\xd4\x34\x2f\xdf\x7c\xa7\x1c\x77\xe0\xfe\x7c\x07\x21\xa2\xfe\x63\xbd\xa1\x6e\x10\xc8\x05\xf3\x19\xcf\x9b\xc0"
        "\x08\xbd\x49\xc8\x89\x5b\xa1\x1d\xa7\x56\x4f\x92\xcf\xc2\x49\x56\xf0\x4c\xdd\x13\xc8\x9c\x72\x5e\x2e\x9c\x89\x11\x35\x41\x61\x79"
        "\x3d\xd5\x35\x93\xbd\x07\x11\xd1\x7d\x0d\x56\x86\x2b\x8f\x53\xea\xa7\x1c\x55\xc9\xb2\x1b\xdc\x14\x84\x22\x33\x18\x69\x46\x03\x8c"
        "\x20\x39\xd8\xf4\xf9\x0c\xd7\x08\xc2\xba\x67\x13\xfb\xc3\xea\x6f\x85\x0d\xa9\x73\x00\x6c\x20\x27\x41\x8e\xd3\x49\x02\xbe\xd0\x23"
        "\xc2\xe5\x85\x64\xbd\x54\x39\x36\x3a\x4e\x2d\x6a\x60\x60\x0f\x5a\x57\x62\xf1\x8f\xa7\xac\x49\x75\x4c\xf9\x5e\xe1\xb9\x8c\xfe\xba"
        "\xec\x4c\x45\x02\x2b\x52\x09\x03\xf0\x70\x99\xee\x41\xc3\xf4\xb2\x25\x06\x30\x1e\xcd\xf0\x36\x51\x85\x68\x38\x17\x71\xff\x9b\xd5"
        "\xf1\x1b\x44\x39\xfa\xf2\x01\x53\x4e\x96\x3f\xe7\xc6\xc5\xbc\x47\xca\x9f\xeb\xe9\xc7\x3b\x64\x91\x80\x1b\x91\x24\xf5\x2e\xae\x0e"
        "\xdf\x13\x31\xc3\xa5\xa8\xd6\x5e\xe1\xf4\x31\x60\xc0\x1c\x0c\xe5\xd7\xaa\x9d\xb8\x3c\x04\x78\xb7\xab\x0b\x23\xa2\xa2\x76\x47\xe9"
        "\x96\x00\xa8\xfa\x44\xaa\x69\x04\x2b\xa9\x39\xc3\x84\x3c\xa6\x09\x2b\x96\x17\x59\xc8\x17\x8c\x65\x87\x14\xc1\xf1\xef\x32\x0d\x8f"
        "\x4f\xfd\x75\x48\x0b\x00\xb4\x65\xf3\x40\xcf\x45\xfc\x2c\xc3\x1f\x8a\x3e\x8a\xf3\x19\xd5\x68\x7b\x38\xad\x43\xe9\x8d\x7b\xea\x98"
        "\x69\x74\x00\x76\x88\x81\xb0\xf7\x94\x36\xb1\x60\x50\x2f\x59\x86\x24\xb2\x42\x50\x75\x16\xd2\xb8\x7f\x2a\x5a\xb3\xe1\x20\x90\xb8"
        "\x9b\xeb\xc6\x90\x01\xcb\xef\x05\x8f\x6d\xc2\x04\x82\x6c\x1c\x62\xf6\x9b\x80\xd6\xb0\x5b\xb6\x0a\xfd\x0e\x33\x49\x15\xa6\xde\xbc"
        "\x00\x5f\x89\xc9\x7b\x68\xc6\xd1\x4f\x5c\xdf\x4b\x98\x14\x15\xc2\xe9\x0b\xc1\xb9\x47\xfe\x22\xf2\x89\x24\x42\x1f\xcd\x20\x43\x4e"
        "\xb9\x7f\x9c\xff\x5f\x08\x8c\x5e\x03\x41\xbb\xef\x03\x2e\x6c\x1d\x5e\x4e\x46\x09\x2c\x6b\x0f\x4b\x09\xd1\xe4\x19\xe3\x85\xe9\x35"
        "\x60\xde\xa5\xdc\xe2\xc6\xb0\x51\xa8\x2f\x33\x27\x74\x60\x65\x25\x5a\x7d\x34\xe8\x72\x4c\x9d\x60\xa4\x82\x30\x57\x1a\xb7\x79\xdf"
        "\x93\x72\x08\x3a\x50\x24\xed\x7d\x48\x3c\xb5\x85\x24\x15\x6d\x9a\x34\xf5\x13\x45\x14\xe0\xf5\xc8\x9a\xbe\xd5\x24\xe9\x05\xf5\xc9"
        "\xb0\xb5\xe0\x61\xbb\xc9\xaf\x97\x9c\xe3\x23\x15\x70\x91\xc1\x46\x40\xc4\xe9\xfe\x7d\xef\xb5\x3c\xb8\x3c\x34\xa0\x8a\x1d\x74\x37"
        "\xa6\x4d\x4a\x3c\xd4\xbd\x2a\x5c\x9f\xfe\xa8\x8c\xd4\x03"
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
