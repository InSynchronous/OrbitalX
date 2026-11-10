#pragma once

#include "Satelite.h"
#include <memory>
#include <vector>
namespace obx::physics {

class NSateliteSim {
  private:
    std::vector<std::shared_ptr<Satelite>> satelites;

  public:
    NSateliteSim() {}
    void addSatelite(std::shared_ptr<Satelite> satelite) {
        satelites.push_back(std::move(satelite));
    }
    void update(float delta_time);
};

} // namespace obx::physics
