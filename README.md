# OrbitalX
A spacecraft orbital analysis and simulation technology demonstrator written in C++ with the bgfx graphics abstraction layer.

## Building from scratch
This is a very in-depth process and if you haven't done advanced build systems before, this can be very daunting.

Clone this project, recursively:

```sh
git clone git@github.com:InSynchronous/OrbitalX.git --recurse-submodules
```

> [!WARNING]
> If you use a questionable antivirus, it will trigger wrongly about "OrbitalX/submodules/bx/tools/bin/darwin/bin2c", claiming it's `behavioral_detection_30_execution_and_evasion`
> This is completely harmless and isn't even used by the OrbitalX project at all. Check online if you want. BX is a very popular library, literally used in Minecraft Bedrock Edition itself.

Build bgfx on linux:
```sh
cd submodules/bgfx
make linux-gcc-release64
```

Build bgfx on MacOS:
```sh
cd submodules/bgfx
make osx-arm64-release
```

This will take an insane amount of time. Be aware.

Ensure `CMake` and `GLFW` are installed.

Linux:
```sh
./build-linux.sh
```

MacOS:
```sh
./build-osx.sh
```

You should now have an executable in build:
```sh
./build/OrbitalX
```
