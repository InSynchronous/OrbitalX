#include "ObjMesh.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

void obx::ObjMesh::loadFromObj(const std::string &path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open OBJ file: " + path);
    }

    std::string line;

    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string type;

        stream >> type;

        if (type == "v") {
            float x, y, z;
            stream >> x >> y >> z;

            vertices.push_back({x, y, z, 1, 1, 1, 1});
        }

        else if (type == "f") {
            std::vector<uint16_t> face;
            std::string token;

            while (stream >> token) {
                // v, v/vt, v//vn, v/vt/vn

                int index = std::stoi(token.substr(0, token.find('/')));

                if (index < 0) {
                    index = static_cast<int>(vertices.size()) + index + 1;
                }

                if (index <= 0 || index > static_cast<int>(vertices.size())) {
                    throw std::runtime_error("Invalid OBJ vertex index");
                }

                face.push_back(static_cast<uint16_t>(index - 1));
            }

            // sometimes its not a triangle and is some other shape
            // so you triangulate polygon using a triangle fan.
            for (size_t i = 1; i + 1 < face.size(); i++) {
                indices.push_back(face[0]);
                indices.push_back(face[i]);
                indices.push_back(face[i + 1]);
            }
        }
    }

    if (vertices.empty() || indices.empty()) {
        throw std::runtime_error("OBJ file contains no usable geometry: " +
                                 path);
    }

    // bgfx indicies is uint16_t 2^16 = 65535
    if (vertices.size() > 65535) {
        throw std::runtime_error("OBJ file exceeds 16-bit index limit");
    }
}

void obx::ObjMesh::upload() {
    for (auto &vertex : vertices) {
        vertex.r = position.r;
        vertex.g = position.g;
        vertex.b = position.b;
        vertex.a = position.a;
    }

    layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
        .end();

    vertexBuffer = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices.data(),
                      static_cast<uint32_t>(vertices.size() * sizeof(Vertex))),
        layout);

    indexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(
        indices.data(),
        static_cast<uint32_t>(indices.size() * sizeof(uint16_t))));

    assert(bgfx::isValid(vertexBuffer));
    assert(bgfx::isValid(indexBuffer));
}
