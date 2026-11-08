#include "bx/math.h"
#include "imgui.h"
#include "obx/gfx_engine/Entity.h"
#include "obx/gfx_engine/Renderer.h"
#include "obx/gfx_engine/SphereMesh.h"
#include "obx/physics/CelestialBody.h"
#include "obx/physics/NBodySim.h"
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

const float earthRadiusKm = 6'371.0f;
const float geoAltitudeKm = 35'786.0f;
const float orbitDistKm = earthRadiusKm + geoAltitudeKm;

const float siderealDaySec = 86'164.0f;
const float lunarSiderealMonthSec = 2'360'591.0f;
const float earthAngularSpeed = 2.0f * 3.14159265358979f / siderealDaySec;
const float moonAngularSpeed = 2.0f * 3.14159265358979f / lunarSiderealMonthSec;

const float moonRadiusKm = 1'737.4f;

const float METERS_TO_KM = 1.0f / 1000.0f;

static int speed_mul = 1000; // 1000x speed
static double FIXED_DT = 1;  // 1 second

static double moonx = 0;
static double moonz = 0;

static float focalLengthMm = 134.0f;
static float sensorHeightMm = 37.7f;

static float roll = 0.0f;
static float pitch = 0.0f;
static float yaw = 0.0f;

static obx::gfx_engine::Camera c = {
    {0.0f, 0.0f, orbitDistKm},                                    // position
    {bx::toRad(-pitch), bx::toRad(180.0f - yaw), bx::toRad(roll)} // orientation
};

enum class CameraSensor { Custom, SonyFullFrame, SonyAPS_C, SonyMicroFourThirds, GOES16GLM };

static CameraSensor selectedSensor = CameraSensor::SonyFullFrame;

static const char *sensorNames[] = {"Custom", "Sony Full Frame", "Sony APS-C", "Micro Four Thirds",
                                    "GOES-16 GLM"};

enum class CameraPosition { Custom, Geostationary, PolarNorth, PolarSouth };
static CameraPosition selectedPosition = CameraPosition::Geostationary;
static const char *positionNames[] = {"Custom", "Geostationary", "PolarNorth", "PolarSouth"};

void setCameraSensor(CameraSensor sensor) {
    selectedSensor = sensor;

    switch (sensor) {
    case CameraSensor::SonyFullFrame:
        sensorHeightMm = 24.0f;
        break;

    case CameraSensor::SonyAPS_C:
        sensorHeightMm = 15.6f;
        break;

    case CameraSensor::SonyMicroFourThirds:
        sensorHeightMm = 13.0f;
        break;

    case CameraSensor::GOES16GLM:
        focalLengthMm = 134.0f;
        break;

    case CameraSensor::Custom:
        return;
    }
}

void setCameraPosition(CameraPosition position) {
    selectedPosition = position;

    switch (position) {
    case CameraPosition::Geostationary:
        c.position = {0.0f, 0.0f, orbitDistKm};
        roll = pitch = yaw = 0.0f;
        break;

    case CameraPosition::PolarNorth:
        c.position = {0.0f, orbitDistKm, 0.0f};

        pitch = -90.0f;
        yaw = 0.0f;
        roll = 0.0f;
        break;

    case CameraPosition::PolarSouth:
        c.position = {0.0f, -orbitDistKm, 0.0f};

        pitch = 90.0f;
        yaw = 0.0f;
        roll = 0.0f;
        break;

    case CameraPosition::Custom:
        return;
    }

    c.rotation = {bx::toRad(-pitch), bx::toRad(180.0f - yaw), bx::toRad(roll)};
}

void imgui_handle() {
    ImGui::Begin("Simulation Box");

    ImGui::Text("Simulation");
    ImGui::InputInt("Speed Multiplier", &speed_mul, 10);
    ImGui::InputDouble("Simulation Timestep (seconds)", &FIXED_DT, 0.1);

    ImGui::End();

    ImGui::Separator();

    ImGui::Begin("Camera Box");
    ImGui::Text("Camera Settings");

    int sensorIndex = static_cast<int>(selectedSensor);
    if (ImGui::Combo("Sensor", &sensorIndex, sensorNames, IM_ARRAYSIZE(sensorNames))) {
        setCameraSensor(static_cast<CameraSensor>(sensorIndex));
    }

    ImGui::BeginDisabled(selectedSensor != CameraSensor::Custom);
    ImGui::InputFloat("Sensor Height (mm)", &sensorHeightMm);
    ImGui::EndDisabled();
    ImGui::InputFloat("Lens (mm)", &focalLengthMm);

    ImGui::Separator();

    ImGui::Text("Position/Orientation");

    int positionIndex = static_cast<int>(selectedPosition);

    if (ImGui::Combo("Position", &positionIndex, positionNames, IM_ARRAYSIZE(positionNames))) {
        setCameraPosition(static_cast<CameraPosition>(positionIndex));
    }
    ImGui::Text("Camera Orientation (deg)");

    ImGui::BeginDisabled(selectedPosition != CameraPosition::Custom);

    bool orientationChanged = false;

    orientationChanged |= ImGui::InputFloat("Roll", &roll, 0.1f, 1.0f, "%.2f");
    orientationChanged |= ImGui::InputFloat("Pitch", &pitch, 0.1f, 1.0f, "%.2f");
    orientationChanged |= ImGui::InputFloat("Yaw", &yaw, 0.1f, 1.0f, "%.2f");

    ImGui::EndDisabled();

    if (orientationChanged) {
        c.rotation = {bx::toRad(-pitch), bx::toRad(180.0f - yaw), bx::toRad(roll)};
    }
    ImGui::Separator();

    ImGui::Text("Mouse: %.0f, %.0f", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);

    ImGui::End();

    ImGui::Begin("Info Box");
    ImGui::Text("Moon X Z: %.0f, %.0f", moonx, moonz);
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

    double accumulator = 0.0;
    double previousTime = glfwGetTime();

    obx::gfx_engine::Renderer renderer = obx::gfx_engine::Renderer(1280, 720);
    renderer.addImguiCallback(&imgui_handle);

    auto earthEntity =
        createCelestialBody(renderer, {0.0f, 0.0f, 0.0f}, earthRadiusKm, "assets/earth.jpg");

    auto moonEntity =
        createCelestialBody(renderer, {-384'399.0f, 0.0f, 0.0f}, moonRadiusKm, "assets/moon.jpg");

    renderer.camera.position = {0.0f, 0.0f, orbitDistKm};
    renderer.camera.rotation = {0.0f, bx::toRad(180.0f), 0.0f};

    auto earth =
        std::make_shared<obx::physics::CelestialBody>(5.9722e24, bx::Vec3{0.0f, 0.0f, 0.0f});

    auto moon = std::make_shared<obx::physics::CelestialBody>(
        7.342e22, bx::Vec3{-384'399'000.0f, 0.0f, 0.0f}, bx::Vec3{0, 0, 0},
        bx::Vec3{0, 0, -1018.0f});

    obx::physics::NBodySim simulation;

    simulation.addBody(earth);
    simulation.addBody(moon);

    while (!renderer.windowShouldClose()) {
        double currentTime = glfwGetTime();
        double frameTime = currentTime - previousTime;
        previousTime = currentTime;

        frameTime = std::min(frameTime, 0.25);

        accumulator += frameTime * speed_mul;

        while (accumulator >= FIXED_DT) {
            simulation.update(FIXED_DT);
            accumulator -= FIXED_DT;
        }

        renderer.camera.rotation = c.rotation;
        renderer.camera.position = c.position;

        renderer.camera.fov = bx::toDeg(2.0f * atan(sensorHeightMm / (2.0f * focalLengthMm)));

        double simTime = glfwGetTime() * speed_mul;
        float earthAngle = simTime * earthAngularSpeed;
        float moonAngle = simTime * moonAngularSpeed;

        earthEntity->mesh->setRotation({0.0f, earthAngle, 0.0f});
        earthEntity->mesh->setPosition(bx::mul(earth->getPosition(), METERS_TO_KM));

        moonEntity->mesh->setRotation({0.0f, moonAngle, 0.0f});
        moonEntity->mesh->setPosition(bx::mul(moon->getPosition(), METERS_TO_KM));

        moonx = moon->getPosition().x;
        moonz = moon->getPosition().z;

        renderer.render();
    }

    return 0;
}
