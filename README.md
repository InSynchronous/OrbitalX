# OrbitalX
A spacecraft orbital analysis and simulation technology demonstrator written in C++ with the bgfx graphics abstraction layer.

## Building from scratch
This is a very in-depth process and if you haven't done advanced build systems before, this can be very daunting.

Clone this project, recursively:

```sh
git clone git@github.com:InSynchronous/OrbitalX.git --recurse-submodules
```

Build bgfx:

```sh
cd submodules/bgfx
make linux-gcc-release64
```

(you may substitute gcc for clang)

This will take an insane amount of time. Be aware.

### vulkan
Compile the shaders to SPIR-V
```sh
mkdir -p assets/shaders/spirv

submodules/bgfx/.build/linux64_gcc/bin/shadercRelease \
    -f shaders/vs_cube.sc \
    -o assets/shaders/spirv/vs_cube.bin \
    --type vertex \
    --platform linux \
    -p spirv \
    -i submodules/bgfx/src \
    --varyingdef shaders/varying.def.sc

./submodules/bgfx/.build/linux64_gcc/bin/shadercRelease \
    -f shaders/fs_cube.sc \
    -o assets/shaders/spirv/fs_cube.bin \
    --type fragment \
    --platform linux \
    -p spirv \
    -i submodules/bgfx/src \
    --varyingdef shaders/varying.def.sc

```

Ensure `CMake` and `GLFW` are installed.
```sh
cmake -B build
cmake --build build
```

You should now have an executible in build:
```sh
./build/OrbitalX
```
