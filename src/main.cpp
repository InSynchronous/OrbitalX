#include "bx/math.h"
#include "imgui.h"
#include "libsgp4/DateTime.h"
#include "obx/Color.h"
#include "obx/gfx_engine/ConeMesh.h"
#include "obx/gfx_engine/Entity.h"
#include "obx/gfx_engine/ObjMesh.h"
#include "obx/gfx_engine/OrbitLineMesh.h"
#include "obx/gfx_engine/Renderer.h"
#include "obx/gfx_engine/SphereMesh.h"
#include "obx/physics/CelestialBody.h"
#include "obx/physics/NBodySim.h"
#include "obx/physics/NSateliteSim.h"
#include "obx/physics/Passpredict.h"
#include "obx/physics/Satelite.h"

#include "stb_image.h"
#include <chrono>
#include <cmath>
#include <ctime>
#include <iostream>
#include <memory>
#include <utility>
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
const float issAltitudeKm = 400.0f;
float orbitDistKm = earthRadiusKm + geoAltitudeKm;

const float siderealDaySec = 86'164.0f;
const float lunarSiderealMonthSec = 2'360'591.0f;
const float earthAngularSpeed = 2.0f * 3.14159265358979f / siderealDaySec;
const float moonAngularSpeed = 2.0f * 3.14159265358979f / lunarSiderealMonthSec;

const float moonRadiusKm = 1'737.4f;

const float METERS_TO_KM = 1.0f / 1000.0f;

const float DRAGON_RENDER_SCALE = METERS_TO_KM;

static int speed_mul = 1000; // 1000x speed
static double FIXED_DT = 1;  // 1 second

static double moonx = 0;
static double moonz = 0;

static float followDistanceKm = 0.05f; // 50 meters behind the dragon

static float focalLengthMm = 50.0f;
static float sensorHeightMm = 37.7f;

static float roll = 0.0f;
static float pitch = 0.0f;
static float yaw = 0.0f;

static float dragon_roll = 0.0f;
static float dragon_pitch = 0.0f;
static float dragon_yaw = 0.0f;

static int satelite_index = 0;

static libsgp4::DateTime timeNow;
static libsgp4::DateTime simEpoch; // TLE epoch, never changes
static double simSeconds = 0.0;    // seconds since simEpoch
static bool warpToNowRequested = false;
static bool warpToPassOverRequested = false;

static float latitude = 37.7749, longitude = -122.4194;
std::list<obx::physics::PassDetails> passList;

static obx::gfx_engine::Camera c = {

    {0.0f, 0.0f, 0.0f},                                  // position
    {bx::toRad(pitch), bx::toRad(-yaw), bx::toRad(roll)} // orientation
};

enum class CameraSensor { Custom, SonyFullFrame, SonyAPS_C, SonyMicroFourThirds, GOES16GLM };

static CameraSensor selectedSensor = CameraSensor::SonyFullFrame;

static const char *sensorNames[] = {"Custom", "Sony Full Frame", "Sony APS-C", "Micro Four Thirds",
                                    "GOES-16 GLM"};

enum class CameraPosition {
    Custom,
    FollowDragon,
    LowEarthOrbit,
    PolarNorth,
    PolarSouth,
    Geostationary,
    Debug,
};
static CameraPosition selectedPosition = CameraPosition::Geostationary;
static const char *positionNames[] = {"Custom",     "FollowDragon",  "LowEarthOrbit", "PolarNorth",
                                      "PolarSouth", "Geostationary", "DEBUG"};

typedef struct {
    std::shared_ptr<obx::physics::Satelite> sateliteSimulation;
    std::shared_ptr<obx::gfx_engine::Entity> sateliteEntity;
    std::shared_ptr<obx::gfx_engine::Entity> markerSphereEntity;
    std::shared_ptr<obx::gfx_engine::Entity> orbitLineEntity;
    std::shared_ptr<std::vector<bx::Vec3>> orbitPoints;
} OrbitalSatelite;

std::vector<OrbitalSatelite> orbitalSatelites;

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
        orbitDistKm = earthRadiusKm + geoAltitudeKm;
        c.position = {0.0f, 0.0f, -orbitDistKm};
        roll = pitch = yaw = 0.0f;
        break;

    case CameraPosition::LowEarthOrbit:
        orbitDistKm = earthRadiusKm + issAltitudeKm;
        c.position = {0.0f, 0.0f, -orbitDistKm};
        roll = pitch = yaw = 0.0f;
        break;

    case CameraPosition::PolarNorth:
        orbitDistKm = earthRadiusKm + geoAltitudeKm;

        c.position = {0.0f, orbitDistKm, 0.0f};

        pitch = -90.0f;
        yaw = 0.0f;
        roll = 0.0f;
        break;

    case CameraPosition::PolarSouth:
        orbitDistKm = earthRadiusKm + geoAltitudeKm;

        c.position = {0.0f, -orbitDistKm, 0.0f};

        pitch = 90.0f;
        yaw = 0.0f;
        roll = 0.0f;
        break;

    case CameraPosition::Debug:
        c.position = {0.0f, 0.0f, -30.0f};
        roll = pitch = yaw = 0.0f;
        break;

    case CameraPosition::FollowDragon:
        return;

    case CameraPosition::Custom:
        return;
    }

    c.rotation = {bx::toRad(pitch), bx::toRad(-yaw), bx::toRad(roll)};
}

void imgui_handle() {
    ImGui::Begin("Simulation Box");

    ImGui::Text("Simulation");
    ImGui::TextUnformatted(timeNow.ToString().c_str());
    if (ImGui::Button("Warp to Now (UTC)")) {
        warpToNowRequested = true;
    }
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
        c.rotation = {bx::toRad(pitch), bx::toRad(-yaw), bx::toRad(roll)};
    }

    ImGui::BeginDisabled(selectedPosition != CameraPosition::FollowDragon);
    ImGui::InputFloat("Follow Distance (km)", &followDistanceKm, 0.01f, 1.0f, "%.3f");
    ImGui::EndDisabled();

    ImGui::Separator();

    ImGui::Text("Mouse: %.0f, %.0f", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);

    ImGui::End();

    ImGui::Begin("Info Box");
    ImGui::Text("Moon X Z: %.0f, %.0f", moonx, moonz);
    ImGui::End();

    // Dragon Container:
    ImGui::Begin("Satelite Manager");
    ImGui::Text("Selected Satelite:");
    ImGui::InputInt("(index)", &satelite_index);
    ImGui::Text("Spacecraft Orientation");
    ImGui::InputFloat("Roll", &dragon_roll, 0.1f, 1.0f, "%.2f");
    ImGui::InputFloat("Pitch", &dragon_pitch, 0.1f, 1.0f, "%.2f");
    ImGui::InputFloat("Yaw", &dragon_yaw, 0.1f, 1.0f, "%.2f");
    ImGui::End();

    // Passpredict container:
    ImGui::Begin("Predict Passes");
    ImGui::Text("Position");
    ImGui::InputFloat("Latitude", &latitude, 0.1f, 1.0f, "%.5f");
    ImGui::InputFloat("Longitude", &longitude, 0.1f, 1.0f, "%.5f");

    if (ImGui::Button("Warp to next predicted passover (UTC)")) {
        warpToPassOverRequested = true;
    }

    ImGui::End();
}

std::shared_ptr<obx::gfx_engine::Entity> createCelestialBody(obx::gfx_engine::Renderer &renderer,
                                                             const bx::Vec3 &position, float radius,
                                                             const std::string &texturePath) {
    auto mesh = std::make_unique<obx::gfx_engine::SphereMesh>(position, bx::Vec3{0.0f, 0.0f, 0.0f},
                                                              32, radius);

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

OrbitalSatelite createSateliteBody(obx::gfx_engine::Renderer &renderer,
                                   obx::physics::NSateliteSim &simulation, libsgp4::Tle tle,
                                   float r, float g, float b, float a) {

    auto mesh = std::make_unique<obx::gfx_engine::ObjMesh>(
        bx::Vec3{0.0f, 0.0f, 0.0f}, bx::Vec3{0.0f, 0.0f, 0.0f}, "assets/models/dragon.obj",
        DRAGON_RENDER_SCALE);
    auto dragonEntity = std::make_shared<obx::gfx_engine::Entity>(std::move(mesh));
    renderer.addEntity(dragonEntity);
    auto fallback = generate_checkerboard(512, 256);
    dragonEntity->loadTexture(fallback.data(), 512, 256);

    // marker sphere
    auto trackingMarkerMesh = std::make_unique<obx::gfx_engine::SphereMesh>(
        bx::Vec3{0.0f, 0.0f, 0.0f}, bx::Vec3{0.0f, 0.0f, 0.0f}, 16, 1.0f);
    auto trackingMarkerEntity =
        std::make_shared<obx::gfx_engine::Entity>(std::move(trackingMarkerMesh));
    const std::vector<uint8_t> markerTexture = {
        static_cast<uint8_t>(r * 255), static_cast<uint8_t>(g * 255), static_cast<uint8_t>(b * 255),
        static_cast<uint8_t>(a * 255)};
    trackingMarkerEntity->loadTexture(markerTexture.data(), 1, 1);
    renderer.addEntity(trackingMarkerEntity);

    auto dragon = std::make_shared<obx::physics::Satelite>(tle, timeNow);
    simulation.addSatelite(dragon);

    // orbit
    auto orbitPoints = std::make_shared<std::vector<bx::Vec3>>();
    auto orbitMesh = std::make_unique<obx::gfx_engine::OrbitLineMesh>(
        bx::Vec3{0, 0, 0}, bx::Vec3{0, 0, 0}, orbitPoints, r, g, b, a);
    auto orbitEntity = std::make_shared<obx::gfx_engine::Entity>(std::move(orbitMesh));
    renderer.addEntity(orbitEntity);

    return {dragon, dragonEntity, trackingMarkerEntity, orbitEntity, orbitPoints};
}

// credit: wikipedia
double julianDate(int year, int month, int day, int hour, int minute, double second) {
    if (month <= 2) {
        year--;
        month += 12;
    }

    const int A = year / 100;
    const int B = 2 - A + A / 4;

    const double dayFraction = (hour + (minute + second / 60.0) / 60.0) / 24.0;

    return std::floor(365.25 * (year + 4716)) + std::floor(30.6001 * (month + 1)) + day + B -
           1524.5 + dayFraction;
}

// also credit wikipedia
double calculateGMST(double jd) {
    const double T = (jd - 2451545.0) / 36525.0;

    double degrees = 280.46061837 + 360.98564736629 * (jd - 2451545.0) + 0.000387933 * T * T -
                     (T * T * T) / 38710000.0;

    degrees = std::fmod(degrees, 360.0);

    if (degrees < 0.0)
        degrees += 360.0;

    return bx::toRad(static_cast<float>(degrees));
}

bx::Vec3 latLongToSphere(double latitude, double longitude, float radius) {
    double lat = bx::toRad(latitude);
    double lon = bx::toRad(longitude);

    return bx::Vec3{static_cast<float>(radius * std::cos(lat) * std::cos(lon)),
                    static_cast<float>(radius * std::sin(lat)),
                    static_cast<float>(radius * std::cos(lat) * std::sin(lon))};
}

bx::Vec3 rotatePointEarth(bx::Vec3 point, float earthRotation) {
    float mtx[16];
    bx::mtxRotateY(mtx, earthRotation);

    return bx::mul(point, mtx);
}

// distance from a ground station to a satellite at altitude h seen at elevation eps
float slantRangeKm(float R, float h, float eps) {
    const float r = R + h;
    return std::sqrt(r * r - R * R * std::cos(eps) * std::cos(eps)) - R * std::sin(eps);
}

int main() {
    std::cout << "Build system working?! " << std::endl;

    double accumulator = 0.0;
    double previousTime = glfwGetTime();

    // Initialize Renderer
    obx::gfx_engine::Renderer renderer = obx::gfx_engine::Renderer(1280, 720);
    renderer.addImguiCallback(&imgui_handle);

    // Create 3D Objects
    auto earthEntity =
        createCelestialBody(renderer, {0.0f, 0.0f, 0.0f}, earthRadiusKm, "assets/images/earth.jpg");
    auto moonEntity = createCelestialBody(renderer, {-384'399.0f, 0.0f, 0.0f}, moonRadiusKm,
                                          "assets/images/moon.jpg");

    bx::Vec3 stationLocal = latLongToSphere(latitude, longitude, earthRadiusKm);
    float minElevRad = bx::toRad(10.0f);
    float slantKm = slantRangeKm(earthRadiusKm, issAltitudeKm, minElevRad);

    auto losConeMesh = std::make_unique<obx::gfx_engine::ConeMesh>(
        bx::Vec3{0.0f, 0.0f, 0.0f}, bx::Vec3{0.0f, 0.0f, 0.0f}, slantKm * std::sin(minElevRad),
        slantKm * std::cos(minElevRad), 48, stationLocal, bx::normalize(stationLocal));

    obx::gfx_engine::ConeMesh *losCone = losConeMesh.get();

    auto losConeEntity = std::make_shared<obx::gfx_engine::Entity>(std::move(losConeMesh));
    auto coneTex = generate_checkerboard(512, 256);
    losConeEntity->loadTexture(coneTex.data(), 512, 256);
    renderer.addEntity(losConeEntity);

    // Create simulation objects
    auto earth =
        std::make_shared<obx::physics::CelestialBody>(5.9722e24, bx::Vec3{0.0f, 0.0f, 0.0f});
    auto moon = std::make_shared<obx::physics::CelestialBody>(
        7.342e22, bx::Vec3{-384'399'000.0f, 0.0f, 0.0f}, bx::Vec3{0, 0, 0},
        bx::Vec3{0, 0, -1018.0f});

    // Initialize simulation
    obx::physics::NBodySim planetSimulation;

    planetSimulation.addBody(earth);
    planetSimulation.addBody(moon);

    obx::physics::NSateliteSim sateliteSimulation;
    libsgp4::Tle tle("A0882",
                     "1 A0882U 26226A   26317.40012392  .00008768  00000-0  16503-3 0  9993",
                     "2 A0882  51.6324 281.7005 0007366  20.9423 339.1866 15.49350588  6871");
    simEpoch = tle.Epoch();
    simSeconds = 0.0;
    timeNow = simEpoch;

    orbitalSatelites.emplace_back(
        createSateliteBody(renderer, sateliteSimulation, tle, 0.0, 1.0, 0.0, 1.0));

    libsgp4::Tle tleTiangong(
        "48274", "1 48274U 21035A   26317.54115615  .00018078  00000-0  20631-3 0  9992",
        "2 48274  41.4674 134.3818 0004387 302.2376  57.8037 15.61773849316490");
    orbitalSatelites.emplace_back(
        createSateliteBody(renderer, sateliteSimulation, tleTiangong, 0.0, 0.0, 1.0, 1.0));

    auto warpTo = [&](const libsgp4::DateTime &t) {
        const double target =
            (julianDate(t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second()) -
             julianDate(simEpoch.Year(), simEpoch.Month(), simEpoch.Day(), simEpoch.Hour(),
                        simEpoch.Minute(), simEpoch.Second())) *
            86400.0;

        if (target <= simSeconds || FIXED_DT <= 0.0)
            return;

        while (simSeconds < target) {
            const double dt = std::min(FIXED_DT, target - simSeconds);
            planetSimulation.update(dt);
            sateliteSimulation.update(dt);
            simSeconds += dt;
        }
        accumulator = 0.0;
    };

    auto regeneratePasses = [&]() {
        passList = orbitalSatelites[satelite_index].sateliteSimulation->generatePassList(
            libsgp4::CoordGeodetic{latitude, longitude, 0.05}, 7, 60.0f);
    };

    auto nextAos = [&](libsgp4::DateTime &out) {
        for (const auto &p : passList) {
            if (p.aos > timeNow) {
                out = p.aos;
                return true;
            }
        }
        return false;
    };

    // orbit mesh
    auto regenerateOrbitalTrajectories = [&]() {
        for (auto &dragon : orbitalSatelites) {
            dragon.orbitPoints->clear();

            const double meanMotionRevsPerDay = dragon.sateliteSimulation->getTle().MeanMotion();
            const double orbitPeriodSeconds = 86400.0 / meanMotionRevsPerDay;

            const int samples = 180;
            dragon.orbitPoints->reserve(samples + 1);

            for (int i = 0; i <= samples; ++i) {
                double t = orbitPeriodSeconds * double(i) / double(samples);

                libsgp4::DateTime sampleTime = timeNow.AddSeconds(t);
                libsgp4::Eci eci = dragon.sateliteSimulation->getPosition(sampleTime);
                libsgp4::Vector pos = eci.Position();

                dragon.orbitPoints->push_back({static_cast<float>(pos.x), static_cast<float>(pos.z),
                                               static_cast<float>(pos.y)});
            }

            dragon.orbitPoints->back() = dragon.orbitPoints->front();
        }
    };

    regenerateOrbitalTrajectories();
    regeneratePasses();

    float lastLat = latitude;
    float lastLon = longitude;

    // Main Loop
    setCameraPosition(selectedPosition);
    setCameraSensor(selectedSensor);

    while (!renderer.windowShouldClose()) {
        double currentTime = glfwGetTime();
        double frameTime = currentTime - previousTime;
        previousTime = currentTime;

        frameTime = std::min(frameTime, 0.25);

        if (warpToNowRequested) {
            warpToNowRequested = false;

            const auto now = std::chrono::system_clock::now();
            const auto nowSeconds = std::chrono::floor<std::chrono::seconds>(now);
            const std::chrono::year_month_day utcDate{
                std::chrono::floor<std::chrono::days>(nowSeconds)};
            const std::chrono::hh_mm_ss utcTime{nowSeconds -
                                                std::chrono::floor<std::chrono::days>(nowSeconds)};

            const double targetSimSeconds =
                (julianDate(static_cast<int>(utcDate.year()),
                            static_cast<unsigned>(utcDate.month()),
                            static_cast<unsigned>(utcDate.day()),
                            static_cast<int>(utcTime.hours().count()),
                            static_cast<int>(utcTime.minutes().count()),
                            static_cast<double>(utcTime.seconds().count())) -
                 julianDate(simEpoch.Year(), simEpoch.Month(), simEpoch.Day(), simEpoch.Hour(),
                            simEpoch.Minute(), simEpoch.Second())) *
                86400.0;

            if (targetSimSeconds > simSeconds && FIXED_DT > 0.0) {
                while (simSeconds < targetSimSeconds) {
                    const double dt = std::min(FIXED_DT, targetSimSeconds - simSeconds);
                    planetSimulation.update(dt);
                    sateliteSimulation.update(dt);
                    simSeconds += dt;
                }
                accumulator = 0.0;
            }
        }

        if (warpToPassOverRequested) {
            warpToPassOverRequested = false;

            libsgp4::DateTime aos;

            // the list may be stale or empty, so regenerate it from the current time if needed
            if (!nextAos(aos)) {
                regeneratePasses();
            }

            if (nextAos(aos)) {
                warpTo(aos);
            } else {
                std::cout << "No passes in the next 7 days" << std::endl;
            }
        }

        // Complete the amount of time necessary for the time between last frame scaled to the
        // current timescale
        // eg. 60fps -> 0.0167 * time scale = amount of time to simulate -> divide by fixed_dt to
        // get simulation frame count
        accumulator += frameTime * speed_mul;

        while (accumulator >= FIXED_DT) {
            planetSimulation.update(FIXED_DT);
            sateliteSimulation.update(FIXED_DT);

            accumulator -= FIXED_DT;
            simSeconds += FIXED_DT;
        }

        timeNow = simEpoch.AddSeconds(simSeconds);

        auto sat = orbitalSatelites[satelite_index];

        std::cout << obx::Color::Bold << obx::Color::Cyan << "[" << timeNow.ToString() << "]"
                  << obx::Color::Reset << " " << obx::Color::Bold << obx::Color::Magenta
                  << "(DRAGON)" << obx::Color::Reset << " " << obx::Color::Gray << "│ "
                  << obx::Color::Reset << obx::Color::Bold << obx::Color::Yellow << "X "
                  << obx::Color::Reset << std::fixed << std::setprecision(3)
                  << sat.sateliteSimulation->getPosition().x << "  " << obx::Color::Bold
                  << obx::Color::Green << "Y " << obx::Color::Reset
                  << sat.sateliteSimulation->getPosition().y << "  " << obx::Color::Bold
                  << obx::Color::Blue << "Z " << obx::Color::Reset
                  << sat.sateliteSimulation->getPosition().z << '\n';

        // offset by earth's position
        const bx::Vec3 satKm = sat.sateliteSimulation->getPosition();
        const float altKm = bx::length(satKm) - earthRadiusKm;

        // follow dragon camera
        c.useLookDir = (selectedPosition == CameraPosition::FollowDragon);

        if (c.useLookDir) {
            const bx::Vec3 radial = bx::normalize(satKm);
            c.position = bx::add(satKm, bx::mul(radial, followDistanceKm));
            c.lookDir = bx::mul(radial, -1.0f);
            c.up = {0.0f, 1.0f, 0.0f};
        }

        // set camera sync with imgui
        renderer.camera = c;

        // @todo fix this its stupid
        renderer.camera.fov = bx::toDeg(2.0f * atan(sensorHeightMm / (2.0f * focalLengthMm)));

        // simulated time
        double simTime = simSeconds;

        // simulated earth/moon spin
        float earthAngle = simTime * earthAngularSpeed;
        float moonAngle = simTime * moonAngularSpeed;

        double jd = julianDate(timeNow.Year(), timeNow.Month(), timeNow.Day(), timeNow.Hour(),
                               timeNow.Minute(), timeNow.Second());

        earthAngle = calculateGMST(jd);

        if (latitude != lastLat || longitude != lastLon) {
            latitude = std::clamp(latitude, -90.0f, 90.0f);
            longitude = std::clamp(longitude, -180.0f, 180.0f);

            stationLocal = latLongToSphere(latitude, longitude, earthRadiusKm);
            losCone->rebuild(slantKm * std::sin(minElevRad), slantKm * std::cos(minElevRad),
                             stationLocal, bx::normalize(stationLocal));

            lastLat = latitude;
            lastLon = longitude;

            // regenerate pass list for dragon
            regeneratePasses();

            std::cout << "GENERATED PASS LIST OF LENGTH: " << passList.size() << std::endl;
        }

        losConeEntity->mesh->setRotation({0.0f, earthAngle, 0.0f});

        earthEntity->mesh->setRotation({0.0f, earthAngle, 0.0f});

        // I lwk do not care about the moon's gravitational pull on the earth it is too annoying to
        // move the camera and everything to be
        // earthEntity->mesh->setPosition(bx::mul(earth->getPosition(), METERS_TO_KM));

        moonEntity->mesh->setRotation({0.0f, moonAngle, 0.0f});
        moonEntity->mesh->setPosition(bx::mul(moon->getPosition(), METERS_TO_KM));

        for (auto &dragon : orbitalSatelites) {
            auto dragonKm = dragon.sateliteSimulation->getPosition();
            dragon.sateliteEntity->mesh->setPosition(dragonKm);
            dragon.sateliteEntity->mesh->setRotation(
                {bx::toRad(dragon_pitch), bx::toRad(-dragon_yaw), bx::toRad(dragon_roll)});

            float cameraDistance = bx::length(bx::sub(c.position, dragonKm));
            int viewportWidth = 1280;
            int viewportHeight = 720;
            if (1) {
                // if (cameraDistance > 100.0f && viewportHeight > 0) {

                float markerRadiusPixels = 3.0f;
                float markerRadius = 2.0f * cameraDistance *
                                     std::tan(bx::toRad(renderer.camera.fov) * 0.5f) *
                                     markerRadiusPixels / static_cast<float>(viewportHeight);
                dragon.markerSphereEntity->mesh->setPosition(dragonKm);
                dragon.markerSphereEntity->mesh->setScale(
                    {markerRadius, markerRadius, markerRadius});
            } else {
                dragon.markerSphereEntity->mesh->setScale({0.0f, 0.0f, 0.0f});
            }

            // regenerate
            regenerateOrbitalTrajectories();

            // @todo: figure out a better way than this for abstract classes
            static_cast<obx::gfx_engine::OrbitLineMesh *>(dragon.orbitLineEntity->mesh.get())
                ->rebuild(bx::Vec3{0, 0, 0}, bx::Vec3{0, 0, 0}, dragon.orbitPoints);
        }

        // for imgui
        moonx = moon->getPosition().x;
        moonz = moon->getPosition().z;

        renderer.render();
    }

    return 0;
}
