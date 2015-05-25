//
// Created by dawid on 17/05/15.
//

#pragma once


#include <iostream>
#include "format.h"
#include "glm/glm.hpp"
using glm::vec3;
class Particle {
public:
    int id;
    vec3 r, v, dv;
    float S, dS;

public:
    Particle(int id, vec3 r, vec3 v, float S);
    Particle& operator=(Particle const&) = delete;
    Particle& operator=(Particle&&) = default;
    Particle(const Particle&) = delete;
    Particle(Particle &&) = default;

    friend std::ostream& operator<< (std::ostream& stream, const Particle& particle) {

        stream<<fmt::format("Particle: {0} ({1}, {2}, {3}) {4}", particle.id, particle.r.x, particle.r.y, particle.r.z, particle.S);
        return stream;
    }
};

