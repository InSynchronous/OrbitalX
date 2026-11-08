#include "NBodySim.h"

void obx::physics::NBodySim::update(float delta_time) {
    for (size_t i = 0; i < bodies.size(); i++) {
        for (size_t j = i + 1; j < bodies.size(); j++) {
            auto &b1 = bodies[i];
            auto &b2 = bodies[j];

            double r = bx::distance(b1->getPosition(), b2->getPosition());

            double magnitude = calculateGravitationalForce(b1->getMass(), b2->getMass(), r);

            bx::Vec3 direction = bx::normalize(bx::sub(b2->getPosition(), b1->getPosition()));

            Force force(bx::mul(direction, magnitude));

            b1->applyForce(force);
            force.vector = bx::mul(force.vector, -1.0f);
            b2->applyForce(force);
        }
    }

    for (auto &body : bodies) {
        body->step(delta_time);
    }
}
