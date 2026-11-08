#pragma once

#include "CelestialBody.h"
#include <memory>
#include <vector>
namespace obx::physics {

class NBodySim {
  private:
    std::vector<std::shared_ptr<CelestialBody>> bodies;

    double calculateGravitationalForce(double m1, double m2, double r) {
        return G * (m1 * m2) / (r * r);
    }

  public:
    NBodySim() {}
    void addBody(std::shared_ptr<CelestialBody> body) { bodies.push_back(std::move(body)); }
    void update(float delta_time);
};

} // namespace obx::physics
