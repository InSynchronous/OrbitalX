#pragma once

#include "bx/math.h"
#include <bgfx/bgfx.h>
namespace obx {

class Mesh {
  protected:
    bgfx::IndexBufferHandle indexBuffer;
    bgfx::VertexBufferHandle vertexBuffer;
    bgfx::VertexLayout layout;

    bx::Vec3 position;

    friend class Renderer;

  public:
    Mesh(bx::Vec3 position) : position(position) {
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .end();
    }
    virtual void upload() = 0;
};
} // namespace obx
