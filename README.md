# Spear "engine"

Game engine powering https://dealers-dungeon.com/demo/

Repository structure:
- [src/sf](/src/sf): Standard base library: Compiler abstraction, data structures, synchronization primitives, etc.
- [src/sp](/src/sp): More game-focused utilities
- [src/client](/src/client): Most of the game engine is here, it's fairly non-generic
- [src/server](/src/server): Backend for the game server, gameplay logic lives here

Further related:
- [bqqbarbhg/sp-tools](https://github.com/bqqbarbhg/sp-tools): Content pipeline tools invoked by the editor
- [bqqbarbhg/bq_websocket](https://github.com/bqqbarbhg/bq_websocket): WebSocket implementation for networking
- [ufbx/ufbx](https://github.com/ufbx/ufbx): FBX importer library started for this project, now grown into its own thing

## Building

**NOTE: Actually building something useful from this repository requires private assets!**
The code is available for reference and there might be some neat reusable parts eg. `src/sf` for
a "standard library" with reflection support.

### Windows

Download premake from https://premake.github.io/ and use it to generate MSVC solutions.
All other dependencies are bundled either as source or binary.

### Linux
Run the following commands, in order:
```shell
./misc/install-linux-deps.sh
./misc/install-linux-premake5.sh 
./premake5 gmake
cd proj/
make config=debug_x64
```

## License

Original source code is under the **zlib/libpng license**, see [misc/sp-licenses.txt](misc/sp-licenses.txt)
for licenses for the dependencies.

```
Copyright (c) 2021 Samuli Raivio

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
```
