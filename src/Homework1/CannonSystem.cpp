#include "CannonSystem.h"
#include "Integrators.h"
#include <cmath>


CannonSystem::CannonSystem(float mass, float barrelLength,
    float powder, float azimuth, float elevation, int method)
    : mass_(mass), barrelLength_(barrelLength), method(method)
{
    // Initial Calculation
    const float F = powder * 10000.0f;
    const float v0 = sqrt(2.0f * F * barrelLength / mass);
 
    const float phi = glm::radians(azimuth);
    const float theta = glm::radians(elevation);
    
    // Initial State
    State initial;
    
    initial.position = glm::vec3(0.0f);
    initial.velocity = glm::vec3(
        v0 * cos(theta) * cos(phi),
        v0 * sin(theta),
        v0 * cos(theta) * sin(phi)
    );
    state_ = initial;
    trajectory_.push_back(initial.position);
}

std::vector<glm::vec3> CannonSystem::getTrajectory() const {
    return trajectory_;
}

// dS/dt
CannonSystem::State CannonSystem::derivative(const State& s) const {
    const float k = 50.0f; 
    State dsdt;
    dsdt.position = s.velocity;
    dsdt.velocity = glm::vec3(
        -k/mass_ * s.velocity.x,
        -9.81f - k/mass_ * s.velocity.y,
        -k/mass_ * s.velocity.z
    );
    return dsdt;
}

void CannonSystem::simulate(float dt, int method) {
    State current = state_;
    while (current.position.y >= 0.0f) {
        switch (method) {
        case 0: current = Integrators::eulerStep(current, dt, *this); break;
        case 1: current = Integrators::midPointStep(current, dt, *this); break;
        case 2: current = Integrators::rk4Step(current, dt, *this); break;
        }
        trajectory_.push_back(current.position);
    }
}


