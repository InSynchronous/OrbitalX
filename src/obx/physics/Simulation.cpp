#include "Simulation.h"

void obx::physics::Simulation::update(float delta_time) {
    for (auto &body : bodies) {
        body->step(delta_time);
    }
}

/*
void obx::physics::simulation::update(float delta_time) {
    for (size_t i = 0; i < bodies.size(); i++) {
        for (size_t j = i + 1; j < bodies.size(); j++) {
            auto &b1 = bodies[i];
            auto &b2 = bodies[j];

            double r = bx::distance(b1->getposition(), b2->getposition());

            double magnitude = calculategravitationalforce(b1->getmass(), b2->getmass(), r);

            bx::vec3 direction = bx::normalize(bx::sub(b2->getposition(), b1->getposition()));

            force force(bx::mul(direction, magnitude));

            b1->applyforce(force);
            force.vector = bx::mul(force.vector, -1.0f);
            b2->applyforce(force);
        }
    }

    for (auto &body : bodies) {
        body->step(delta_time);
    }
}
*/
