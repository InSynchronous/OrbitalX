#include "bx/math.h"
#include "imgui.h"
#include "obx/gfx_engine/Entity.h"
#include "obx/gfx_engine/Renderer.h"
#include "obx/gfx_engine/SphereMesh.h"
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

static int imgui_value = 1;

void imgui_handle() {
    ImGui::Begin("Userspace box");
    ImGui::Text("Set Speed Multiplier:");
    ImGui::InputInt("##speed", &imgui_value);

    ImGui::Separator();
    ImGui::Text("Mouse: %.0f, %.0f", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);

    ImGui::End();
}

std::shared_ptr<obx::gfx_engine::Entity> createCelestialBody(obx::gfx_engine::Renderer &renderer,
                                                             const bx::Vec3 &position, float radius,
                                                             const std::string &texturePath) {
    auto mesh = std::make_unique<obx::gfx_engine::SphereMesh>(position, bx::Vec3{0.0f, 0.0f, 0.0f},
                                                              6, radius);

    auto entity = std::make_shared<obx::gfx_engine::Entity>(std::move(mesh));
    renderer.addEntity(entity);

    int w, h, channels;
    stbi_uc *img = stbi_load(texturePath.c_str(), &w, &h, &channels, 4);

    if (img) {
        entity->loadTexture(img, w, h);
        stbi_image_free(img);
    } else {
        std::cerr << "WARNING: Failed to load " << texturePath << '\n';
        auto fallback = generate_checkerboard(512, 256);
        entity->loadTexture(fallback.data(), 512, 256);
    }

    return entity;
}

int main() {
    std::cout << "Build system working?! " << std::endl;

    const float earthRadiusKm = 6'371.0f;
    const float geoAltitudeKm = 35'786.0f;
    const float orbitDistKm = earthRadiusKm + geoAltitudeKm;

    const float siderealDaySec = 86'164.0f;
    const float earthAngularSpeed = 2.0f * 3.14159265358979f / siderealDaySec;

    const float moonRadiusKm = 1'737.4f;

    const float timeScale = 1000.0f;

    obx::gfx_engine::Renderer renderer = obx::gfx_engine::Renderer(1280, 720);
    renderer.addImguiCallback(&imgui_handle);

    auto earthEntity =
        createCelestialBody(renderer, {0.0f, 0.0f, 0.0f}, earthRadiusKm, "assets/earth.jpg");

    auto moonEntity =
        createCelestialBody(renderer, {-10'000.0f, 0.0f, 0.0f}, moonRadiusKm, "assets/moon.jpg");

    renderer.camera.position = {0.0f, 0.0f, orbitDistKm};
    renderer.camera.rotation = {0.0f, bx::toRad(180.0f), 0.0f};

    while (!renderer.windowShouldClose()) {
        float simTime = static_cast<float>(glfwGetTime()) * timeScale;

        float earthAngle = simTime * earthAngularSpeed * imgui_value;

        // not really orbitting, just imagine you're at a point on geostationary orbit watching the
        // earth spin
        earthEntity->mesh->setRotation({0.0f, earthAngle, 0.0f});

        renderer.render();
    }

    return 0;
}
