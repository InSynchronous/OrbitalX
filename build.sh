#!/usr/bin/env bash

cmake -B build

mkdir -p assets/shaders/spirv

./submodules/bgfx/.build/linux64_gcc/bin/shadercRelease \
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


cmake --build build
