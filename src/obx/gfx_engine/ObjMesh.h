#pragma once

#include "Mesh.h"
#include "Vertex.h"
#include <cstdint>
#include <string>
#include <vector>

namespace obx::gfx_engine {

class ObjMesh : public Mesh {
  private:
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    Vertex spherify(Vertex point);
    void loadFromObj(const std::string &path);

  public:
    ObjMesh(bx::Vec3 position, bx::Vec3 rotation, const std::string &path)
        : Mesh{position, rotation} {
        loadFromObj(path);
    }

    void upload() override;
};

} // namespace obx::gfx_engine
