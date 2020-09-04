# spear

## Building

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
