#pragma once

#include "ISimulated.h"
#include "PhysicsConstants.h"
#include <memory>
#include <vector>
namespace obx::physics {

class Simulation {
  private:
    std::vector<std::shared_ptr<ISimulated>> bodies;

    double calculateGravitationalForce(double m1, double m2, double r) {
        return G * (m1 * m2) / (r * r);
    }

  public:
    Simulation() {}
    void addSimulationEntity(std::shared_ptr<ISimulated> body) {
        bodies.push_back(std::move(body));
    }
    void update(float delta_time);
};

} // namespace obx::physics
