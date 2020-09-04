# spear

## Building

### Linux
Run the following commands, in order:
```shell
./misc/install-linux-deps.sh
./misc/install-linux-premake5.sh 
./premake5 gmake
cd proj/
make config=debug_x64
```
