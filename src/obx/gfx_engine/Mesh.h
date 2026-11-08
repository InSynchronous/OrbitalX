#pragma once

#include "bx/math.h"
#include <bgfx/bgfx.h>
namespace obx::gfx_engine {

class Mesh {
  protected:
    bgfx::IndexBufferHandle indexBuffer;
    bgfx::VertexBufferHandle vertexBuffer;
    bgfx::VertexLayout layout;

    bx::Vec3 position;
    bx::Vec3 rotation;

    friend class Renderer;

  public:
    Mesh(bx::Vec3 position, bx::Vec3 rotation) : position(position), rotation(rotation) {
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();
    }
    void setPosition(bx::Vec3 p) { position = p; }
    void setRotation(bx::Vec3 r) { rotation = r; }

    virtual void upload() = 0;
};
} // namespace obx::gfx_engine
