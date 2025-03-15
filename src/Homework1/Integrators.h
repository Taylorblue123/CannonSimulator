
#pragma once
#include "CannonSystem.h"

namespace Integrators {
    CannonSystem::State eulerStep(const CannonSystem::State& s, float dt,
        const CannonSystem& system) {
        auto dsdt = system.derivative(s);
        return {
            s.position + dsdt.position * dt,
            s.velocity + dsdt.velocity * dt
        };
    }

    CannonSystem::State midPointStep(const CannonSystem::State& s, float dt, const CannonSystem& system) {
        auto k1 = system.derivative(s);

        glm::vec3 midPointPos = s.position + k1.position * (dt / 2.0f);
        glm::vec3 midPointVel = s.velocity + k1.velocity * (dt / 2.0f);
        CannonSystem::State midPoint = { midPointPos, midPointVel };

        auto k2 = system.derivative(midPoint);

        return {
            s.position + k2.position * dt,
            s.velocity + k2.velocity * dt
        };
    }

    CannonSystem::State rk4Step(const CannonSystem::State& s, float dt, const CannonSystem& system) {
        auto k1 = system.derivative(s);

        glm::vec3 midPointPos = s.position + k1.position * (dt/2.0f);
        glm::vec3 midPointVel = s.velocity + k1.velocity * (dt/2.0f);
        CannonSystem::State midPoint = {midPointPos, midPointVel};

        auto k2 = system.derivative(midPoint);

        midPointPos = s.position + k2.position * (dt/2.0f);
        midPointVel = s.velocity + k2.velocity * (dt/2.0f);
        midPoint = {midPointPos, midPointVel};

        auto k3 = system.derivative(midPoint);

        midPointPos = s.position + k3.position * dt;
        midPointVel = s.velocity + k3.velocity * dt;
        midPoint = {midPointPos, midPointVel};

        auto k4 = system.derivative(midPoint);

        return {
            s.position + (k1.position + 2.0f * k2.position + 2.0f * k3.position + k4.position) * (dt/6.0f),
            s.velocity + (k1.velocity + 2.0f * k2.velocity + 2.0f * k3.velocity + k4.velocity) * (dt/6.0f)
        };
    }

}