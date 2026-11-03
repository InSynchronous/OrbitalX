#include "bx/math.h"
#include "obx/Renderer.h"
#include "obx/SphereMesh.h"
#include "stb_image.h"
#include <iostream>
#include <memory>

std::vector<uint8_t> generate_checkerboard(int width, int height) {
    std::vector<uint8_t> pixels(width * height * 4);

    for (uint16_t y = 0; y < height; ++y) {
        for (uint16_t x = 0; x < width; ++x) {
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

int main() {
    std::cout << "Build system working?! " << std::endl;

    float earthRadiusKm = 6371.0f;
    float earthGM = 398600.4418f;
    float altitudeKm = 4008.0f;
    float timeScale = 60.0f;

    bx::Vec3 earthCenter = {0.0f, 0.0f, 0.0f};
    float orbitDist = earthRadiusKm + altitudeKm;
    float orbitSpeed = bx::sqrt(earthGM / (orbitDist * orbitDist * orbitDist));
    float pitchDown = bx::toRad(80.0f);
    float roll = bx::toRad(20.0f);

    obx::Renderer renderer = obx::Renderer(1280, 720);
    renderer.addMesh(std::make_unique<obx::SphereMesh>(
        earthCenter, bx::Vec3{0.0f, 0.0f, 0.0f}, 6, earthRadiusKm));

    int w, h, channels;
    stbi_uc *img = stbi_load("assets/earth.jpg", &w, &h, &channels, 4);
    renderer.loadTexture(img, w, h);
    stbi_image_free(img);

    while (!renderer.windowShouldClose()) {
        float a = static_cast<float>(glfwGetTime()) * orbitSpeed * timeScale;

        renderer.camera.position = {earthCenter.x,
                                    earthCenter.y + orbitDist * bx::cos(a),
                                    earthCenter.z + orbitDist * bx::sin(a)};

        renderer.camera.rotation = {-(a + pitchDown), 0.0f, -roll};

        renderer.render();
    }

    return 0;
}
