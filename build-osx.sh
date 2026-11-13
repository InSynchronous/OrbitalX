#!/usr/bin/env bash
set -e

cmake -B build

mkdir -p assets/shaders/metal

SHADERC=./submodules/bgfx/.build/osx-arm64/bin/shadercRelease
BGFX=./submodules/bgfx

$SHADERC \
    -f shaders/vs_cube.sc \
    -o assets/shaders/metal/vs_cube.bin \
    --type vertex \
    --platform osx \
    -p metal \
    -i $BGFX/src \
    --varyingdef shaders/varying.def.sc

$SHADERC \
    -f shaders/fs_cube.sc \
    -o assets/shaders/metal/fs_cube.bin \
    --type fragment \
    --platform osx \
    -p metal \
    -i $BGFX/src \
    --varyingdef shaders/varying.def.sc

$SHADERC \
    -f shaders/vs_imgui.sc \
    -o assets/shaders/metal/vs_imgui.bin \
    --type vertex \
    --platform osx \
    -p metal \
    -i $BGFX/src \
    --varyingdef shaders/varying_imgui.def.sc

$SHADERC \
    -f shaders/fs_imgui.sc \
    -o assets/shaders/metal/fs_imgui.bin \
    --type fragment \
    --platform osx \
    -p metal \
    -i $BGFX/src \
    --varyingdef shaders/varying_imgui.def.sc

$SHADERC \
    -f shaders/vs_line.sc \
    -o assets/shaders/metal/vs_line.bin \
    --type vertex \
    --platform osx \
    -p metal \
    -i $BGFX/src \
    --varyingdef shaders/varying_line.def.sc

$SHADERC \
    -f shaders/fs_line.sc \
    -o assets/shaders/metal/fs_line.bin \
    --type fragment \
    --platform osx \
    -p metal \
    -i $BGFX/src \
    --varyingdef shaders/varying_line.def.sc

cmake --build build
