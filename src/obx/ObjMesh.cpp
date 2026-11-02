
#include "ObjMesh.h"
#include "Vertex.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

void obx::ObjMesh::loadFromObj(const std::string &path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open OBJ file: " + path);
    }

    std::vector<bx::Vec3> positions;
    std::vector<obx::UVCoord> texCoords;

    std::string line;

    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string type;

        stream >> type;

        if (type == "v") {
            float x, y, z;
            stream >> x >> y >> z;

            positions.push_back({x, y, z});
        }

        else if (type == "vt") {
            float u, v;
            stream >> u >> v;

            texCoords.push_back({u, v});
        }

        else if (type == "f") {
            std::vector<uint16_t> face;
            std::string token;

            while (stream >> token) {
                // v, v/vt, v//vn, v/vt/vn

                std::istringstream indicesStream(token);
                std::string positionIndex;
                std::string uvIndex;

                std::getline(indicesStream, positionIndex, '/');
                std::getline(indicesStream, uvIndex, '/');

                int pi = std::stoi(positionIndex);

                if (pi < 0) {
                    pi = static_cast<int>(positions.size()) + pi + 1;
                }

                if (pi <= 0 || pi > static_cast<int>(positions.size())) {
                    throw std::runtime_error("Invalid OBJ vertex index");
                }

                obx::UVCoord uv = {0.0f, 0.0f};

                if (!uvIndex.empty()) {
                    int ti = std::stoi(uvIndex);

                    if (ti < 0) {
                        ti = static_cast<int>(texCoords.size()) + ti + 1;
                    }

                    if (ti <= 0 || ti > static_cast<int>(texCoords.size())) {
                        throw std::runtime_error("Invalid OBJ UV index");
                    }

                    uv = texCoords[ti - 1];
                }

                // each face reference gets its own vertex. this preserves UV
                // seams.
                if (vertices.size() >= 65535) {
                    throw std::runtime_error(
                        "OBJ file exceeds 16-bit index limit");
                }

                const bx::Vec3 position = positions[pi - 1];

                vertices.push_back(Vertex(position, uv));

                face.push_back(static_cast<uint16_t>(vertices.size() - 1));
            }

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
}

void obx::ObjMesh::upload() {
    vertexBuffer = bgfx::createVertexBuffer(
        bgfx::copy(vertices.data(),
                   static_cast<uint32_t>(vertices.size() * sizeof(Vertex))),
        layout);

    indexBuffer = bgfx::createIndexBuffer(
        bgfx::copy(indices.data(),
                   static_cast<uint32_t>(indices.size() * sizeof(uint16_t))));

    assert(bgfx::isValid(vertexBuffer));
    assert(bgfx::isValid(indexBuffer));
}
