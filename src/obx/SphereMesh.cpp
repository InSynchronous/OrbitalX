#include "SphereMesh.h"
#include "bx/math.h"

#include <bgfx/bgfx.h>
#include <cmath>
#include <cstdint>

namespace obx {

static bx::Vec3 spherify(float x, float y, float z, float radius) {
    const float x2 = x * x;
    const float y2 = y * y;
    const float z2 = z * z;

    const float sx =
        x * std::sqrt(1.0f - y2 / 2.0f - z2 / 2.0f + y2 * z2 / 3.0f);

    const float sy =
        y * std::sqrt(1.0f - z2 / 2.0f - x2 / 2.0f + z2 * x2 / 3.0f);

    const float sz =
        z * std::sqrt(1.0f - x2 / 2.0f - y2 / 2.0f + x2 * y2 / 3.0f);

    return {sx * radius, sy * radius, sz * radius};
}

void SphereMesh::upload() {
    vertices.clear();
    indices.clear();

    const int resolution = subdivisions + 1;

    // Six cube faces
    for (int face = 0; face < 6; ++face) {
        const uint16_t baseIndex = static_cast<uint16_t>(vertices.size());

        for (int y = 0; y <= resolution; ++y) {
            for (int x = 0; x <= resolution; ++x) {
                const float u =
                    2.0f * static_cast<float>(x) / resolution - 1.0f;

                const float v =
                    2.0f * static_cast<float>(y) / resolution - 1.0f;

                float px = 0.0f;
                float py = 0.0f;
                float pz = 0.0f;

                switch (face) {
                case 0:
                    px = 1.0f;
                    py = v;
                    pz = -u;
                    break;
                case 1:
                    px = -1.0f;
                    py = v;
                    pz = u;
                    break;
                case 2:
                    px = u;
                    py = 1.0f;
                    pz = -v;
                    break;
                case 3:
                    px = u;
                    py = -1.0f;
                    pz = v;
                    break;
                case 4:
                    px = u;
                    py = v;
                    pz = 1.0f;
                    break;
                case 5:
                    px = -u;
                    py = v;
                    pz = -1.0f;
                    break;
                }

                bx::Vec3 p = spherify(px, py, pz, radius);

                p.x += position.x;
                p.y += position.y;
                p.z += position.z;

                vertices.push_back(Vertex(p, 1, 1, 1));
            }
        }

        for (int y = 0; y < resolution; ++y) {
            for (int x = 0; x < resolution; ++x) {
                const uint16_t i = baseIndex + y * (resolution + 1) + x;

                indices.push_back(i);
                indices.push_back(i + resolution + 1);
                indices.push_back(i + 1);

                indices.push_back(i + 1);
                indices.push_back(i + resolution + 1);
                indices.push_back(i + resolution + 2);
            }
        }
    }

    vertexBuffer = bgfx::createVertexBuffer(
        bgfx::copy(vertices.data(),
                   static_cast<uint32_t>(vertices.size() * sizeof(Vertex))),
        layout);

    indexBuffer = bgfx::createIndexBuffer(
        bgfx::copy(indices.data(),
                   static_cast<uint32_t>(indices.size() * sizeof(uint16_t))));
}

} // namespace obx
