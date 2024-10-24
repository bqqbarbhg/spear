#include "SpLicense.h"

#include "ext/sp_tools_common.h"
#include "sp/Json.h"

namespace sp {

// Generated by misc/gen-licenses.py sp-licenses.txt
static const uint32_t uncompressedSize = 19496;
static const char compressedLicenses[] =
	"\x28\xb5\x2f\xfd\x60\x28\x4b\x85\x83\x00\x5a\x84\x9c\x17\x31\xc0\xa8\x88\xea\x01\x98\x54\x2b\xc4\xa8\x2d\x83\x79\xf8\x71\xc1\xaa"
	"\xc3\x63\x13\xe7\x1f\xa4\x53\xaf\xcf\xd5\x7d\x04\xbc\x80\xc2\x21\x08\x42\x01\xb4\xe6\x85\x12\x3b\x14\x5c\x57\x51\x14\x45\x01\x08"
	"\x6e\x01\x72\x01\x6a\x01\x0e\xc4\x63\x6b\x72\x12\x1c\x37\x90\x83\x44\x8e\xf4\xbc\x08\x72\xc8\xee\x51\x91\x4a\x12\xe1\xca\x91\x24"
	"\x4f\x64\x55\x4e\x84\x9d\xc7\x64\x45\x8f\x09\x2b\x92\x22\x07\x07\xe2\xa9\x1c\x84\x1f\x14\x3b\x64\x28\xcb\x8a\x20\x91\xc4\x16\x35"
	"\x31\x33\x0f\x53\x04\x49\xa4\x67\x8a\x98\x79\x2e\xb2\x26\xa7\x8a\xa4\x08\x24\xe2\x8a\xe2\xf7\x5c\x15\x31\xc0\xf1\xc4\xe4\x38\x5c"
	"\xc0\xe7\x2c\x20\x65\x49\x1c\xa2\xca\x9a\xa0\xc9\xe1\x57\x39\x48\xa2\x20\xd8\x82\x78\x88\xaa\x89\x08\xf5\xb0\x83\xb0\x78\xc2\x81"
	"\x21\x07\x45\x12\xa8\x2a\xc2\xae\x72\x24\xac\x07\xf5\xf8\x45\x10\xd6\xbf\xdb\x29\x5b\x7b\xcb\xa5\xd4\xa7\xf0\xc5\x7d\x6b\x66\x2b"
	"\xf7\xe5\x5d\x7f\xe3\x9e\xf0\xe5\x6d\xfd\x3b\x67\x4f\x4f\xe3\xb4\x7d\x63\xc6\xb6\xb2\xad\x0c\xd7\x3b\xdb\x64\xbd\xb8\xe5\x76\xf9"
	"\x6c\x6c\x67\x33\x52\x79\x39\x65\xfe\xc9\xb4\x9c\xb7\x7a\x45\x9d\x44\x9f\xf1\xb6\xe5\xfb\xdb\x55\x25\xa5\xf2\x73\x00\x0c\xe1\x76"
	"\xc6\x3f\x2f\xaf\x92\x2b\x3e\x2d\x95\xc2\x7d\xaf\x59\x51\xad\xc6\x57\xdf\x3b\x33\xdb\x1a\x37\x64\xa2\x9c\x94\x79\xd5\x8c\xef\x56"
	"\x6d\x65\xec\x4e\x0b\x33\x9f\xd9\x4a\xe1\x8b\x8d\x3e\x27\x2d\xed\x56\x9f\x7d\x32\xb3\xd7\x79\xb7\xc9\xb4\xf7\xe7\xd5\x95\x43\x87"
	"\xcd\xd5\x15\x96\x9f\x29\x75\xb8\x91\x1e\x31\x43\x51\xeb\xa4\x65\x7c\x30\x73\xbd\xbf\xad\xe6\x60\x80\xe0\x40\x03\x37\x24\x83\x2f"
	"\x56\x99\xf6\xa2\xd5\xe7\x52\xe8\x04\x16\xb7\x29\xad\x0f\xbb\xc7\x89\x05\x02\x4f\xe1\x11\xeb\xdf\x22\x7d\x1c\x09\x18\x7a\x0e\xda"
	"\xe5\x2e\x98\x59\x5c\x36\x5e\x1e\xef\x1f\xb0\x60\x08\x8b\xce\xa2\x2f\x18\x40\x20\x6d\xdb\x5e\x74\xab\x96\xca\xd5\xd4\x52\x9d\x42"
	"\x46\xa6\x44\x4c\xe3\xad\xd6\xb2\xd5\x0b\x47\xb8\xba\xad\xcf\xca\x0d\x02\x66\xe6\x0b\x75\xd8\xf6\x3e\xd5\xb4\x28\xb3\x2a\xbf\x6a"
	"\x76\xd3\xda\x6b\xf3\xb3\x2b\x31\x8b\x57\xd3\x49\xbc\x68\x63\x6f\x37\x4a\xaf\x77\x45\x73\x74\x99\x54\xcb\x26\x85\x66\x16\x7e\x3a"
	"\x3e\x2e\x4c\xca\x95\x4a\x29\xff\xb3\x25\xb8\xed\xe5\x4e\xc5\x35\x86\xc9\xa4\x15\x68\xa5\xa8\xcf\x01\xc4\xaa\x18\x8c\xdd\x08\xe1"
	"\xce\xe6\xa5\xb1\x2b\xc9\x71\x10\xb3\x9b\x28\x08\x55\x3d\x98\xc7\x71\xc8\x54\x92\xea\x31\x45\x13\x34\x39\x64\x22\x20\xac\xc8\xd9"
	"\xaa\xf1\xb5\xf1\xea\x9e\x30\xa5\x11\x7b\xeb\xca\x9e\xe0\x76\xa3\x64\x15\xb3\xfc\xa4\xfd\xcc\x33\xd8\x64\xd3\x9a\x61\xea\x9d\xd9"
	"\xa4\xd7\x46\xa5\x4f\x96\x6d\x9f\x43\x1a\xfb\x6a\x2c\xe0\x5b\x40\x06\x7f\xd3\x49\xe1\x38\x65\xfd\x8f\x94\xea\x30\x4e\xea\x5f\x6a"
	"\xc5\x46\xac\xdb\x52\x98\xb9\xfa\x7a\xb4\x15\xbe\x4c\x9a\x7f\x72\x84\x01\x60\x6e\xd0\xef\x2d\xfb\xde\xc6\x64\xdb\x07\x7f\xa4\x15"
	"\x93\x73\x6a\x48\x8f\xc3\xb0\x93\x29\x7d\x96\x4d\x77\x98\x69\x60\xd1\x40\x3c\x51\x24\x92\x00\x39\xfc\x88\x98\x89\x24\x8e\xc4\x8b"
	"\x1a\xee\x45\x12\x94\x7b\x26\xf7\x10\x49\x0f\x6e\x62\xcb\x7a\x9e\x53\x39\x08\x09\x66\x26\x2a\x6a\xa2\x20\x47\xa2\x31\x51\x91\x15"
	"\x91\x54\x95\xb3\xa2\x17\xe1\x39\x16\xe3\x9e\xe4\x51\x92\xc3\x08\xcf\x3d\x91\x84\xca\x3d\x88\xa2\x1e\xd8\x59\x12\x2b\xb2\x9e\x83"
	"\xe4\x41\x5c\x25\x79\x18\x0e\x9f\x7b\x70\x20\x0e\xc4\x92\x02\xdc\x63\x26\x2a\xc7\x45\x8f\x8b\x3c\x66\xa4\xc7\x7a\x5c\x15\x41\x4e"
	"\x72\x70\x9c\x83\x24\x05\x8c\x7d\xbd\xcf\x2f\x7c\x5b\x6c\xc2\x65\xfc\xb5\x41\x70\x36\xad\xce\x6c\xdf\xac\xe5\x6f\x13\x22\x30\x8d"
	"\xbc\x0d\x45\x8c\x3a\x64\x9c\x57\x31\xa3\xe0\xa9\x18\xdf\x2e\x0e\x2d\xe6\xa6\xb1\xe8\xf0\x63\x8d\xf4\x5b\x8b\xf2\x6f\x4f\x5e\xff"
	"\x63\x37\x0e\xdb\x63\x25\x5a\x8d\xac\xca\xe6\x6a\x17\x2b\x07\x60\x6c\x75\x82\xe5\xb4\x8e\x1d\x38\x16\x3c\x97\xd8\xd2\x7b\xde\x1a"
	"\xe5\x68\x2a\xcc\x67\x9a\xc6\x9f\x96\x8b\x56\x8c\xb9\xfa\x23\x5f\xad\x25\x05\xc7\xfc\x78\x02\xaa\x98\xbf\x67\xec\x6e\xca\x8b\xb0"
	"\xda\x63\x7f\x43\x81\x42\xc4\x06\xb8\x98\x61\x86\x1d\xcc\xe0\xc1\x03\x04\xee\x89\xde\x8b\x20\x95\x8b\x54\x6c\x4f\xbc\xef\x63\xda"
	"\xd3\x23\xd4\x15\xd4\x6e\x92\xe1\x29\x43\xc5\xd6\x26\x36\x16\x8b\x85\x81\x01\x82\x76\x2f\xe6\x50\xc1\x81\xcd\x23\x01\xcc\x48\x80"
	"\x54\x07\x9a\x06\x6e\x4c\x46\xcd\xb1\xde\x0e\x06\xcb\xfb\x91\x1d\xdc\xf6\xa2\xfd\xc0\xd7\xac\xa4\x77\x03\x39\x61\x87\xec\x90\x1b"
	"\xdb\x97\x6f\x76\xbb\x1b\xe5\x72\xe7\x76\x5b\xc3\xde\xa7\x8c\x7c\x27\xff\x2d\xdf\xce\xbe\x1c\xc6\x3b\x29\xed\x3d\xa1\x90\x60\x87"
	"\xdd\xcc\xbc\x25\xd2\x0a\xd3\x2b\x55\xb6\xbd\x7e\xda\x6d\xc2\x7b\xec\xec\x32\x71\x78\x75\x26\xf2\xfb\x64\xc6\x56\x66\x18\x2d\x6b"
	"\x2e\xad\x3f\x36\x6d\x3b\xf1\xf6\xdb\xd6\x46\x07\xf6\xd8\xcf\x67\xcc\x2a\xc4\x11\xca\x8e\xa6\x3c\xb9\xcf\x0c\x3a\x6b\x4a\xf9\xea"
	"\xa7\x66\x15\xe6\xf6\x90\x45\x7c\xa0\xe1\x40\x83\x43\x41\x83\xc1\x9b\xdb\xfa\x64\xd4\xdc\x2a\x59\xc4\x36\x36\x54\xb3\xb6\x09\x30"
	"\xd9\x14\x36\x66\x7c\xa1\xae\xbd\xed\x8a\xbe\x0d\xcd\xdc\x16\x08\x09\x34\xb0\x68\x5c\xc4\x91\x48\x92\x7b\x8e\x03\x4b\xa8\xca\xe1"
	"\x4d\x3c\xd1\xc3\x89\x22\xa8\x69\x92\x22\x08\xba\xd8\xf8\xed\x6f\x6b\xd4\xec\xbc\x5c\xdb\x9a\x6c\x4a\xbb\xe2\xd3\x52\x7c\x7e\xd7"
	"\x8b\x5b\x6e\xff\x91\xa1\xad\x69\xf5\x7e\x33\x0e\x5b\xb6\xbd\x3a\xeb\xbe\xee\xf7\xd7\xbf\x51\xe3\xc6\x83\x06\x0d\x0a\xf8\x79\x54"
	"\xdc\xfd\x0c\x87\xdf\xb8\x74\x04\xf6\x1c\x6d\x23\x8c\x88\x17\x59\xc0\xfb\xf3\x99\xf6\x6a\x1d\xcf\x8f\x91\xf8\x5b\x8d\xe0\x48\x31"
	"\x21\xcc\xf5\xb8\xbd\xfd\xc9\xcf\x02\x32\xb0\x1b\x9b\x9b\x38\x29\xea\xc4\x0b\x1a\x88\xb3\xde\xc4\xa3\xaa\x27\x82\xf0\x74\xa1\xb6"
	"\x6a\x5a\xdf\xb9\xbd\x36\x25\x3f\x8f\x83\x9c\xab\x58\x92\x3e\xeb\xfb\x97\xaf\x56\x4e\x08\xfb\xf6\xb8\xc1\xa1\x43\x87\x0e\x1d\x3a"
	"\x74\xe8\xf0\x59\x0e\x1d\x12\xe0\xb0\xeb\x5b\x3c\x32\x70\x68\xa7\xad\xde\x11\x6e\x16\x0c\xd0\x56\x63\xe6\xc1\x0e\x17\x0b\xb8\x80"
	"\x16\xfd\xaf\x0c\x6e\xee\xa4\xa0\xb5\x5a\x50\xab\xb4\x2b\xfa\x72\x58\x6d\xc4\xe1\x40\x80\x04\x1a\x89\x25\xd3\x03\x02\x0e\x3d\xde"
	"\xeb\x9b\x6e\x26\x73\xc5\x23\x26\xe9\x24\xd2\x19\x2b\x22\x5f\x28\x9d\x38\x64\x87\xec\x90\x1d\xb2\x43\x87\x0e\x1d\x3a\x5c\x58\x50"
	"\x40\x01\x09\x35\xc1\x4f\x54\xb9\x46\xbd\xb2\x6d\x83\x52\xdb\x32\xde\x32\x25\x14\x38\x20\x44\xda\xd6\xe1\xb7\xcf\xad\xda\xc9\x97"
	"\xd2\x74\x61\x81\x2f\x5b\x2d\x0b\x87\x97\x89\x9c\x98\xc6\x7f\x89\x54\xb3\xdb\x8d\x4d\x49\xe0\xb0\x69\x70\xd8\x20\x70\xc8\x0e\x1b"
	"\x3e\xa9\x57\x5b\x59\xdb\xee\x73\x76\xe3\x7f\x67\x4c\xe3\xe6\x1e\xff\xda\xf8\xb7\x12\x84\x7f\xa8\x23\x58\x0a\x19\x27\x91\x2a\x99"
	"\x01\x00\x00\x08\x00\x63\x90\x00\x28\x24\x26\x98\x0f\xc7\x22\xdb\xa4\x07\xe3\x52\x53\x4a\x30\xa8\x24\x31\x18\x32\x10\x20\x82\x04"
	"\x06\x18\x00\x00\x30\x03\x91\x04\x00\x00\x42\x22\x48\x00\xf5\xba\x32\xe0\x9e\x59\x93\xea\x82\x4b\x0d\x90\x5b\x53\x87\xab\x61\x31"
	"\x39\x24\x6d\x8c\xab\xae\xce\xd5\x51\xa7\xca\x97\x15\x4a\x41\x7c\x10\xa3\x3d\x1e\x87\x1f\xeb\x6a\x07\xc9\x22\x4e\x33\x4f\x80\x59"
	"\x39\xae\x60\xe7\x88\x69\xa5\x37\x94\x42\xbf\x74\xa6\xa3\x12\x9f\x3d\x6b\xf8\xea\x6a\xfc\x30\xa8\x25\xe4\xd9\x64\xc1\x39\xd0\xbe"
	"\x13\xfd\x6d\x33\xdc\xe8\x62\x9e\x2e\xb8\xdb\x7d\x0b\x95\x62\x77\x3a\x9c\x23\x61\x79\xf1\x26\xf8\x19\x4d\x92\x8b\x2d\x22\x0c\x93"
	"\x85\xea\xf3\xc8\x8b\xff\x70\x51\xf8\xcb\x5d\x76\x36\xe2\x75\x4f\x92\x05\x73\xab\x8c\x1a\x04\x24\x73\x94\x90\x6c\x7d\x18\x1f\xeb"
	"\x17\x34\x55\x3b\x2c\x7f\x35\x3d\x89\x62\xd1\x69\x2c\xdd\x2a\xf0\xe9\x10\xa5\x62\xd9\x86\x0c\xc3\x58\xcb\xc6\x05\xfc\x5a\x31\x25"
	"\xe1\x5a\x95\xb8\xc8\xa0\xaf\x97\xcb\xc7\x48\x0e\x19\xd6\xc3\x54\x9c\x64\xbf\xfc\xbf\x3c\xd0\xbc\xd4\x31\x3e\xf7\xe5\x54\xad\x4b"
	"\xd8\x16\x6c\x11\x7e\x37\xb6\x45\x51\xb9\x77\xb0\x9b\x4c\xfe\xd0\xe1\xab\xcb\x7f\xfa\x2d\x14\x4c\x4d\x01\xe3\xef\x50\x41\x8a\x23"
	"\xea\xee\x94\xcc\xa7\xd7\xa3\x02\x3d\xfe\xb0\xe2\x60\xbf\x9a\xc5\x15\xb9\xf4\xc0\xa0\x6a\xb6\x8b\x0a\xc8\x7e\x9d\xab\x2d\x44\xd1"
	"\xff\xde\x82\x33\xe3\x1e\xea\xcf\x4e\x8e\x67\xd8\xb8\x5f\x06\x9a\xe0\xb8\xd2\x16\x94\xb7\xd7\xa0\x9d\xaf\x4f\x7a\x18\x6a\x54\x14"
	"\x8b\x2b\xa9\x7c\xd8\x39\x9f\xdf\x8a\x31\xfd\xae\x5e\xa9\x04\x03\x0a\x58\x38\x4f\xe9\xa5\x42\x2d\x64\xf6\x67\xf8\x28\xff\xad\x3a"
	"\x39\x79\xd9\xc6\x94\xab\x9a\x4d\xa2\xc5\x3c\x4a\xd0\x4a\x26\x64\xae\x1e\x5a\x65\xc3\x5c\x48\x2d\x3a\x50\x4c\x08\xd1\xb8\x5d\x9f"
	"\xe1\xb5\x73\x9c\x7c\x67\x8f\x32\xe7\xe6\xf6\x9b\x63\xa2\xe7\xfa\x2d\x5c\x4e\x17\xaf\x73\x0e\x82\xc4\x34\x36\x1a\x38\x2b\x77\xb8"
	"\x74\x77\xf7\xc3\x37\x85\x74\x31\xb1\x12\x93\x0c\x42\xfb\x85\x08\xb9\x7b\x75\x42\x46\x87\x78\x7d\x0b\x95\x25\xb9\x81\x82\xcd\x86"
	"\x03\xed\xa9\x30\x47\xb3\x1f\x21\x55\x38\x7c\x0a\x4c\x98\xca\x14\x8a\x08\xa4\x16\x92\x07\x71\x67\x06\x9c\x4b\x20\xb8\x55\x61\x67"
	"\xdd\x34\x21\x92\x54\xf4\xa1\x38\xb7\x17\xfe\xcb\x5c\x4b\xcc\xd2\x13\xb1\xc3\xe8\x69\xa4\x35\x45\x88\x76\xe6\x37\xd7\xee\x71\x6c"
	"\x77\xd6\x9e\xbb\x4b\x5f\x59\x40\x57\xff\x21\x70\x48\xd9\xe1\x59\x20\x88\x2e\xf3\x65\x0c\x08\xe4\x18\x29\xe3\xf8\x63\x92\xe9\x03"
	"\x8f\x05\x04\x83\xfe\x91\xc9\x2b\x25\xa6\xec\xc1\x15\xbd\x06\xb7\x81\x58\x84\x3f\x18\xac\xb9\x6c\x1a\x81\x06\x7e\x58\x36\x42\xdd"
	"\xfb\x79\x81\x1d\x78\x11\x85\x0e\xe9\x56\xd0\x29\x5f\x15\x57\xd0\xa1\xd1\x41\x25\x76\x9b\x85\xa8\xa4\x6e\xb7\x00\x2f\xd9\x8f\x68"
	"\xa8\x47\x55\x3f\x2a\x6c\xd5\xa4\xe5\x26\xbc\x25\x95\x77\xdf\xd6\x13\x9e\x84\xe7\x5d\x90\x45\xcd\x68\x68\x16\xe6\x2c\x0d\x07\xcc"
	"\xd0\xe8\x07\xd8\xa1\xb3\x11\x9c\x37\x48\x4d\x18\xc7\xc3\x08\x07\xe4\x10\x49\xf7\x29\x2b\xdf\x69\xa4\x61\x78\x6b\x60\x37\xb8\x24"
	"\x73\x14\x17\x61\xf2\x83\x67\x4b\xe0\xa2\x09\x7e\x4f\x70\xa9\x80\x43\x04\x57\x05\x45\x15\x8f\x64\x85\x6c\x93\x21\xac\x95\x84\xc3"
	"\xcf\x6c\x93\xc8\x0a\xfb\xb2\x57\x53\xee\xa1\x61\xf2\x89\x6e\x24\x00\x35\x2b\xbf\xcd\x8c\x16\x72\x76\xd4\xb6\x1e\x8a\xe1\x3a\xd8"
	"\x72\xc6\x88\x28\x71\xa2\xe8\x19\xbc\x99\x0e\xf2\x98\x80\xa3\xd1\x01\x59\xc6\xd2\xd1\x79\x10\xa5\x1e\x0a\x32\xcf\x36\x04\xcf\x23"
	"\xdf\xc7\x29\xc9\xb4\xa8\x6e\x5e\x9d\x20\xff\xd4\x60\x09\x17\x6d\x4f\x4e\x24\xb3\x52\x92\x4d\xc1\x4b\xd9\xef\x4e\x22\xa9\xf1\xf6"
	"\xd6\x21\xaf\xf2\x45\x5e\x6a\x18\xa2\x22\x21\x20\xb1\x4a\xa0\x24\xdd\x66\xf0\xdf\x28\x10\x34\x59\x96\x70\xfc\x26\x8f\x8b\x99\x34"
	"\xeb\x9e\x54\x2a\xac\xbc\x2c\x17\x54\xb2\x7b\x41\x69\x48\xe0\xf7\x13\x6e\xe8\x2f\x4d\xea\xe7\x66\x87\x9a\x22\x11\x75\x0c\x20\x73"
	"\xc5\x5b\xc9\xfa\x64\x1f\x7a\xe8\x71\x63\x3d\xb0\x86\x21\xf4\x0a\x89\xa8\x2c\xc2\x88\x92\x5f\x89\xb2\x25\x88\x9d\xe6\x89\xc6\x62"
	"\xfb\x7a\x10\xfa\x51\x94\xd6\x04\x45\x49\xd7\xd7\x51\x4c\x72\x19\x37\xdd\x51\x5a\x41\x25\xae\x4f\x76\xaa\x58\x2e\x79\xac\xf2\xa1"
	"\x3f\xb6\xe4\xfe\x91\xbc\x2c\xa0\x36\x3a\x6d\x42\xcd\x28\x93\x50\x1b\x6f\x92\x3d\xbc\x2b\x00\x01\x77\xc2\xf0\xd8\x81\x1c\x93\xf4"
	"\x3b\x7c\x07\xc8\x42\x2c\x92\x72\x3a\x0c\x8d\xd1\x53\x0c\x05\xa7\x4a\xc5\x07\x45\x91\x02\x67\x05\x70\x30\x3d\x9d\x38\xbf\x64\xa7"
	"\xdc\x2a\x24\xbd\x3c\x1c\x82\xf9\x77\x3a\xf9\xc2\x25\xe0\xf7\xee\x4b\x38\x13\x2f\x20\x10\x29\x68\xad\x3a\xd6\x4a\x60\x07\x9e\x5d"
	"\xf5\x06\xa6\x16\xeb\x02\x2a\xbf\x63\x9e\xa3\x15\xa7\x02\x18\xce\x70\x10\x18\x4b\x54\x48\xb9\x5c\x90\x6d\x7a\xf6\xaf\x65\x86\x09"
	"\x56\x30\x4b\xc0\x7f\xd7\x47\xf6\x6f\x5d\x23\x20\x8c\xe9\x3c\x6f\xc7\x89\x59\xc9\xbd\x37\x5f\x31\x8c\x26\xf3\x01\x64\xad\x19\xf8"
	"\x57\xb2\x0a\x1a\x2a\x12\xbf\x8c\x5e\x39\x63\x35\x64\xab\x53\x35\xe7\xac\xa5\x74\xad\xea\xdf\x72\xae\xea\x76\xdc\xa9\x80\xbd\x10"
	"\x0e\x85\x1d\x78\xdc\x1e\x3e\xf1\xe7\x28\xea\x30\x69\x35\x70\x7b\xac\x80\x40\x1d\xdf\xbd\x7e\x09\x06\xda\x13\xf9\xde\xaf\x25\x3b"
	"\x4f\x1a\x18\x52\x89\xd0\x6d\xec\x21\x1c\x35\x26\x8f\x48\x40\x3a\x91\x3b\x90\x94\x26\xb0\x53\xf8\xc1\x49\xf3\xb8\x98\x54\xa2\x30"
	"\x2a\x01\xbc\x30\x0a\xa3\x52\xd5\xe5\xa7\xa0\x6d\xa8\x09\x30\xe9\x22\x4e\xa8\x20\x2e\x16\x17\x5a\x12\x41\x70\x49\x1d\x64\x73\xf0"
	"\x16\x5d\x14\x4c\xea\x8d\x7b\xd2\x59\x9e\xca\x35\xb7\x2d\x17\xad\xc8\x0d\xc7\x94\xe0\x50\x9b\xdf\x8b\xb1\x20\x75\x36\x2f\xba\xcb"
	"\xfa\x78\xb8\xee\x34\x87\x27\x0b\x2a\xd7\x7b\x40\xd8\x7c\x2d\x9e\x2b\xea\xe2\xd1\xe7\xc5\xa6\x3b\xf6\xd2\x65\x14\x1a\xb3\x52\x09"
	"\x22\xe7\xc0\xd0\x02\x15\x03\x5f\xda\xe3\x5f\x82\x56\x04\x90\x8c\x37\xe3\x70\xa1\x6c\x85\xf8\xd3\x21\xa4\x0d\x4e\xbc\x3f\xf6\x64"
	"\x76\xcc\xc7\xb5\x71\xa6\x3d\x60\x05\xf1\xa6\xca\x7d\xda\xca\xd4\xc0\xba\x47\x5f\x81\x8c\x00\xde\x8b\x0d\x7e\x09\x45\x10\x43\x0d"
	"\x39\x01\xba\x83\xbc\xe8\x09\x6c\xc1\x72\x81\x1b\x80\x5b\x71\x16\x24\xa0\x5d\x99\x04\xa4\x5f\xc4\xd8\x0d\x23\x05\x25\x3d\x28\xc6"
	"\xcd\x9a\x09\x69\xda\xad\x39\x19\xfe\xe7\x05\x97\xe1\xbb\x6b\x49\x11\x51\x93\xba\x4c\xb6\x14\x72\x20\x0b\x32\x71\xf3\xdf\x75\xb7"
	"\xa7\x4f\x17\x2a\x1d\xb9\x61\x7f\x77\x0d\x43\xe2\xbb\x94\x67\x42\x74\xea\x52\x60\x0e\x0b\x08\xd7\x1e\x31\x6a\x3a\x1c\x63\x1f\x0c"
	"\xad\x04\xd7\x85\xf6\xd1\x62\xf4\xd4\x7f\x3d\xa8\xc7\x5a\x21\x76\xc5\x20\x08\x6c\xfb\x87\xbe\x39\x01\x11\xe0\xe1\xb7\x7c\x09\x08"
	"\x6f\x5c\x2a\xf2\xa0\x54\xc8\xfc\x34\x05\x49\xed\x98\x2a\xd3\x1c\x5a\x8d\xd5\x98\x10\xde\x83\x5c\x41\x91\x6a\x10\x55\x8d\x1d\x35"
	"\xde\xfc\x46\x5b\x6e\x45\xf8\x1d\x9a\x66\x18\xd4\xc7\xa4\xf7\x88\x32\x6e\xce\xe1\xae\xd6\x19\x76\x53\x2b\xf8\xc8\xa8\x5f\x4b\xe5"
	"\xa0\xcf\x0e\x2e\x7d\xa6\xf0\x8d\xb3\x7d\xc7\xf1\xe1\x8a\x56\xd6\x3e\xd4\xec\x7f\x6b\x01\x7e\xa3\x75\x33\x3e\x8c\xe9\x3a\xed\x1d"
	"\x8b\x6a\x58\x90\xf9\x3e\x56\xc8\xa5\xc2\x8c\x7e\xc4\x60\xa6\x4f\xea\x1a\x65\x49\x6a\x58\xad\x8a\x9b\x5d\xae\xb3\xde\xd6\x73\x03"
	"\x5f\x97\xc7\x94\xd7\xd4\x84\x8a\xb1\xcc\x9a\x21\xe1\xb7\xf9\xfe\x12\x88\x6c\x65\xe6\x6b\xb7\xbc\x1e\xd3\xf2\xff\x65\xee\xdb\xe1"
	"\x16\x2b\x6d\x74\x44\x82\x64\x37\xaa\xd4\x3c\xc6\xcd\x28\x7e\xb5\xaa\x89\x22\x68\xdd\x43\x8f\x08\xb4\xcf\xcc\x9d\x11\x7b\x7f\x46"
	"\xe1\x75\xbd\x64\x74\xc1\x50\x98\x92\x70\xcb\xd6\x8a\x42\x20\xd6\x9e\x6c\x6c\x49\x48\xa6\x12\x81\xe1\x30\xb1\xb1\xa6\xd8\x72\xc1"
	"\x47\x6e\x38\x0b\xc5\x5c\x43\x68\x04\xc2\x39\x53\xac\xe8\x83\x0c\x26\x13\xe1\x6e\x21\x6f\x7e\x3b\x94\xac\x05\x38\x8c\x78\xae\x7d"
	"\x93\x9b\x68\x35\xbf\xd2\xf0\x51\x2e\xa8\xb4\xaf\x2a\x22\x35\x41\xc2\x07\x41\x89\x16\xca\xce\x51\xa6\xde\x49\xea\x84\x10\x4e\x08"
	"\x15\xb1\x31\x06\xf9\x93\x01\xd8\x00\x6f\xb4\x08\xb7\x0e\xf2\xb5\x19\x77\x0a\xee\xf4\xda\x30\x3b\x67\xe3\x3e\x2b\xc9\x26\x09\x36"
	"\xd4\x38\x92\x46\xb5\x5f\xaf\x30\x17\x20\x6d\xe6\x84\x49\x10\x59\x53\x00\x64\x9a\x65\xa1\x17\x40\x18\x98\xf7\x01\xd8\xe2\x4b\xa4"
	"\xfb\x15\x49\x4a\xf8\x74\x5c\x4b\x24\x56\x4c\x6a\x80\x09\xa0\xc6\xc4\x96\xff\x96\xa0\x69\xf3\xd0\xc8\x69\x50\xe1\xbf\xce\x56\xc7"
	"\x5c\x05\xa0\x60\x1d\xb7\xa9\x4e\x68\x9c\x85\x81\x0a\x7f\x9a\xd0\x17\x2d\xcc\xe9\xdc\xcc\x91\xe9\xf5\xe4\x59\x5f\xa4\xcc\xb6\x8e"
	"\x0f\xa9\x09\x14\xcb\xb7\x67\x05\xa6\x61\xbc\x13\x76\xa2\xe6\x35\xf8\xf4\xd9\x76\x55\x59\xcf\x8c\xaa\x9e\x6d\xa5\x72\x40\x13\x67"
	"\x05\x8b\xd1\x66\xdf\xe7\x18\x0b\x31\xf4\xd1\xab\x1e\x10\x97\x61\xb2\xac\xb1\xac\x90\xc6\x70\x73\xda\xfc\xa3\x21\x05\x2e\xbb\x35"
	"\x18\xbe\xfe\xe2\xd2\x5d\x3e\x34\x70\x28\xbe\x37\xff\x23\x3f\x78\x6b\xfd\xd1\x46\x0c\x2a\x79\x48\x55\xd4\xe4\x7f\xb6\x28\x56\xbc"
	"\x94\x29\xe1\x4f\xb2\x13\xe0\x84\xb0\x0a\x02\x3f\x89\x76\x2c\x36\xce\xd0\x59\x8a\x0a\xd3\x3c\x74\x83\x85\xd6\xaa\x17\x85\xa2\x84"
	"\x8c\x53\x9b\xc3\xa6\xf8\xd0\x54\x86\x9c\xde\xcf\xeb\x91\x3f\x74\x82\x7f\xc4\x28\x22\x35\xd9\x4d\x3d\x78\xbf\xae\x83\xf0\x58\x81"
	"\x50\x35\x26\xc7\xfe\x76\xf9\x79\x69\xec\xe3\x69\x62\x12\x55\xcd\x44\xa0\x1e\x75\x92\xea\xd4\x34\x5b\x03\x98\x87\xc2\x0f\x0d\xf4"
	"\x8c\x62\xae\x11\xe6\x4b\x7e\x1c\x04\xf7\x47\x3c\xa8\x1b\xb5\x86\xd5\x83\x3a\x0f\x20\xfb\xe7\x07\x9e\x87\x37\x22\x5c\x91\xb0\x65"
	"\xf7\x7a\x76\x08\xb5\x0a\x49\x28\x16\x2c\x68\x25\x96\xfb\x13\xe5\x54\xe6\x3e\xc2\x59\x30\xc6\x09\x8b\x97\x77\xac\x3a\xae\x65\x67"
	"\xdd\xf5\xba\xef\x3c\xeb\x10\x6a\x8e\x3f\x71\xe6\xb1\x9c\xf1\xa4\xa4\xdc\x59\x8c\x0e\xde\x06\x8d\x91\x7e\xa3\xcc\x68\x5b\x06\xc1"
	"\x9c\x38\xf4\xad\x0c\xb9\x65\x43\x17\x66\xf3\xd6\x87\x43\xbe\xa5\xf6\x50\xe7\x08\x30\xa8\x9c\xbc\xf2\x4a\xe7\x4f\xa0\x31\x2a\x5b"
	"\xe8\x61\x01\x53\xef\xca\x55\xee\xb1\x16\xfe\x14\x98\x12\xae\x31\x69\xb5\x8c\x47\x6e\x49\x21\x1c\x55\xbd\xba\x65\x65\x2c\xea\x22"
	"\x66\xf2\x13\x48\x12\xa5\x61\xb0\x32\xb3\xa6\xdb\x71\x61\xe2\x6d\x89\x56\x8e\x07\x67\xfd\x4d\x58\x18\x9a\x72\x9d\x28\xce\x9b\x88"
	"\xfb\xa5\x44\xb1\x54\x0f\x03\xa6\xbe\x2c\x71\x9c\xfd\x12\x93\x3e\xa4\x40\xf3\x9c\x7e\x52\x87\x08\x48\x00\x5f\x48\x46\xf2\x0e\x9a"
	"\x0e\xc2\x13\x8d\x0c\x3c\x11\x02\x7a\x3b\xa6\x57\x81\x65\xcf\x69\x1f\x97\xd6\xe4\x01\x93\x1f\x16\xe7\x76\xf5\x45\x89\x10\xa7\x76"
	"\x50\x99\x8c\x84\xbe\x6a\x1e\xd2\x87\x26\x93\x22\x9f\xb3\x91\x80\xc4\x80\xc0\x6a\x88\x6d\x15\x4a\x17\x72\xce\xa0\x29\x1b\xf1\x70"
	"\x65\xf1\xf8\x31\x05\xae\x03\x48\x01\x50\x5b\x07\x9e\xd9\x49\xb7\x87\x65\xce\xa3\x23\x7f\xfb\x09\x45\xac\x51\xfa\xe1\x30\x07\x3c"
	"\x6e\xad\xe7\x67\x5b\x0d\x06\x40\x35\x94\x24\x0c\xd0\x54\x35\xb1\xd4\x23\x54\x22\x35\xf4\x13\x7d\x45\xd5\x95\xee\xb9\xd9\x55\xda"
	"\xb0\x85\xa0\xb0\x8f\xee\xf8\x3b\xc8\x0f\x28\xff\xde\x31\x31\x1d\x61\x41\x41\x66\x2d\xc4\x62\x76\x02\x3d\xc2\xae\x64\x55\x76\xe1"
	"\x00\x93\xf4\xbb\xe9\x00\xaf\x0c\x8f\x65\xe9\xe3\xd2\x44\x66\xff\x67\x7d\x86\x63\x52\x49\x08\x9c\x2f\x14\x7c\x7c\xc4\x28\x52\x41"
	"\xe4\x7a\xfa\xa5\x0c\x56\xce\x1a\x9c\x60\x0e\xcd\x7f\xea\x18\xbe\x06\x62\xda\xa4\x8f\xac\x86\xbd\xc5\xbb\xc6\xe8\x10\x66\xdf\x13"
	"\x56\xc6\x69\x1d\x62\x74\x31\x25\x0f\xd1\x6e\x0d\x6e\x8f\x91\x71\x28\x54\x63\x31\x07\xec\x21\x9c\xd5\x48\x4a\xd0\x06\x5a\xd6\x25"
	"\xb3\x8a\xa4\x88\x18\xdb\xc6\xd3\x8c\xde\x6c\x0e\x62\xf3\x15\x07\x5b\x46\x12\x4e\x6a\x1a\x51\x1a\x35\x3f\xad\xb6\x87\x88\xf8\x98"
	"\xff\x66\xeb\x4c\x9b\x31\x92\x28\x35\xa3\x8a\x25\x40\xec\xa4\xb2\xed\x65\xba\xa2\x91\xa6\x92\xc4\x56\x0d\x1e\x27\x5a\xfe\x4f\x35"
	"\x5f\x03\x07\x70\x0f\x9a\xf3\x42\x11\xbf\xb3\xaa\xc5\x83\xf3\x2f\x6f\x94\xf3\x45\xf9\x1f\x19\x90\xee\x03"
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
