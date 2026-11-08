#pragma once

#include "Mesh.h"
#include "bgfx/bgfx.h"
#include <memory>

namespace obx {

struct Entity {
    std::unique_ptr<Mesh> mesh;
    bgfx::TextureHandle texture;

    Entity(std::unique_ptr<Mesh> mesh) : mesh(std::move(mesh)), texture(BGFX_INVALID_HANDLE) {}

    void loadTexture(const uint8_t *pixels, uint16_t w, uint16_t h) {
        // push to gpu memory
        const bgfx::Memory *memory = bgfx::copy(pixels, w * h * 4);
        // make it a texture handle
        texture = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::RGBA8,
                                        BGFX_SAMPLER_V_CLAMP, memory);
    }
};

} // namespace obx
