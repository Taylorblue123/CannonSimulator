#pragma once
#include <vector>
#include <glm/glm.hpp>

class CannonSystem {
public:
    CannonSystem(float mass, float barrelLength,
        float powder, float azimuth, float elevation, int method);

    std::vector<glm::vec3> getTrajectory() const;
    void simulate(float dt, int method);
    struct State {
        glm::vec3 position;
        glm::vec3 velocity;
    };
    State derivative(const State& s) const;
    int method;
private:

    float mass_;
    float barrelLength_;
    State state_;
    std::vector<glm::vec3> trajectory_;
};