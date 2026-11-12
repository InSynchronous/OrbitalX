#pragma once

#include "Mesh.h"
#include "bx/math.h"

namespace obx::gfx_engine {

class ConeMesh : public Mesh {
  public:
    ConeMesh(const bx::Vec3 &position, const bx::Vec3 &rotation, float height, float baseRadius,
             int segments, const bx::Vec3 &apex, const bx::Vec3 &axis);

    void upload() override;
    void rebuild(float height, float baseRadius, const bx::Vec3 &apex, const bx::Vec3 &axis);

  private:
    float height;
    float baseRadius;
    int segments;
    bx::Vec3 apex;
    bx::Vec3 axis;
};

} // namespace obx::gfx_engine
