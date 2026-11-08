#include "bx/math.h"
#include "obx/Entity.h"
#include "obx/Renderer.h"
#include "obx/SphereMesh.h"
#include "stb_image.h"
#include <iostream>
#include <memory>
#include <vector>

namespace {

std::vector<uint8_t> generate_checkerboard(int width, int height) {
    std::vector<uint8_t> pixels(width * height * 4);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const bool checker = ((x / 32) + (y / 32)) % 2;

            const uint8_t color = checker ? 255 : 30;

            const size_t i = (y * width + x) * 4;

            pixels[i + 0] = color;
            pixels[i + 1] = color;
            pixels[i + 2] = color;
            pixels[i + 3] = 255;
        }
    }

    return pixels;
}

} // namespace

int main() {
    std::cout << "Build system working?! " << std::endl;

    const float earthRadiusKm = 6371.0f;
    const float geoAltitudeKm = 35786.0f;
    const float orbitDistKm = earthRadiusKm + geoAltitudeKm;

    const float siderealDaySec = 86164.0f;
    const float earthAngularSpeed = 2.0f * 3.14159265358979f / siderealDaySec;

    const float timeScale = 1000.0f;

    obx::Renderer renderer = obx::Renderer(1280, 720);

    obx::SphereMesh earthMesh =
        obx::SphereMesh(bx::Vec3{0.0f, 0.0f, 0.0f}, bx::Vec3{0.0f, 0.0f, 0.0f}, 6, earthRadiusKm);

    auto earthEntity = std::make_shared<obx::Entity>(earthMesh);
    renderer.addEntity(earthEntity);

    int w = 0, h = 0, channels = 0;
    stbi_uc *img = stbi_load("assets/earth.jpg", &w, &h, &channels, 4);
    if (img != nullptr) {
        std::cout << "Loaded earth texture: " << w << "x" << h << std::endl;
        earthEntity->loadTexture(img, w, h);
        stbi_image_free(img);
    } else {
        std::cout << "WARNING: assets/earth.jpg not found (run ./build/OrbitalX "
                     "from the repo root). Using checkerboard fallback."
                  << std::endl;
        auto fallback = generate_checkerboard(512, 256);
        earthEntity->loadTexture(fallback.data(), 512, 256);
    }

    renderer.camera.position = {0.0f, 0.0f, orbitDistKm};
    renderer.camera.rotation = {0.0f, bx::toRad(180.0f), 0.0f};

    while (!renderer.windowShouldClose()) {
        float simTime = static_cast<float>(glfwGetTime()) * timeScale;

        float earthAngle = simTime * earthAngularSpeed * renderer.imgui_value;
        earthEntity->mesh.setRotation({0.0f, earthAngle, 0.0f});

        renderer.render();
    }

    return 0;
}
