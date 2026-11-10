#include "NSateliteSim.h"

void obx::physics::NSateliteSim::update(float delta_time) {
    for (auto &satelite : satelites) {
        satelite->step(delta_time);
    }
}
