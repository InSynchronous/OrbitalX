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

    std::vector<bx::Vec3> pos;
    std::vector<uint32_t> tris;

    for (int face = 0; face < 6; ++face) {
        const uint32_t baseIndex = static_cast<uint32_t>(pos.size());

        for (int y = 0; y <= resolution; ++y) {
            for (int x = 0; x <= resolution; ++x) {
                const float u =
                    2.0f * static_cast<float>(x) / resolution - 1.0f;
                const float v =
                    2.0f * static_cast<float>(y) / resolution - 1.0f;

                float px = 0.0f, py = 0.0f, pz = 0.0f;

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

                pos.push_back(spherify(px, py, pz, radius));
            }
        }

        for (int y = 0; y < resolution; ++y) {
            for (int x = 0; x < resolution; ++x) {
                const uint32_t i = baseIndex + y * (resolution + 1) + x;

                tris.push_back(i);
                tris.push_back(i + resolution + 1);
                tris.push_back(i + 1);

                tris.push_back(i + 1);
                tris.push_back(i + resolution + 1);
                tris.push_back(i + resolution + 2);
            }
        }
    }

    constexpr float pi = 3.14159265358979f;

    for (size_t t = 0; t < tris.size(); t += 3) {
        bx::Vec3 p[3] = {
            pos[tris[t + 0]],
            pos[tris[t + 1]],
            pos[tris[t + 2]],
        };
        float u[3], v[3];
        bool pole[3];

        for (int k = 0; k < 3; ++k) {
            v[k] =
                0.5f - std::asin(bx::clamp(p[k].y / radius, -1.0f, 1.0f)) / pi;
            u[k] = 0.5f + std::atan2(p[k].z, p[k].x) / (2.0f * pi);
            pole[k] = std::fabs(p[k].x) + std::fabs(p[k].z) < 1e-4f * radius;
        }

        float lo = 2.0f, hi = -1.0f;
        for (int k = 0; k < 3; ++k) {
            if (pole[k])
                continue;
            lo = std::min(lo, u[k]);
            hi = std::max(hi, u[k]);
        }

        if (hi - lo > 0.5f) {
            for (int k = 0; k < 3; ++k) {
                if (!pole[k] && u[k] < 0.5f)
                    u[k] += 1.0f;
            }
        }

        for (int k = 0; k < 3; ++k) {
            if (!pole[k])
                continue;
            float sum = 0.0f;
            int count = 0;
            for (int j = 0; j < 3; ++j) {
                if (pole[j])
                    continue;
                sum += u[j];
                ++count;
            }
            if (count > 0)
                u[k] = sum / count;
        }

        for (int k = 0; k < 3; ++k) {
            indices.push_back(static_cast<uint16_t>(vertices.size()));
            vertices.push_back(Vertex(p[k], {u[k], v[k]}));
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
