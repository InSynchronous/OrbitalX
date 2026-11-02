#pragma once

#include "Mesh.h"
#include <cstdint>
#include <string>
#include <vector>

namespace obx {

class ObjMesh : public Mesh {
  private:
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    Vertex spherify(Vertex point);
    void loadFromObj(const std::string &path);

  public:
    ObjMesh(Vertex position, const std::string &path) : Mesh{position} {
        loadFromObj(path);
    }

    void upload() override;
};

} // namespace obx
