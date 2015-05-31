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
    float S, dS, d, T, P;
    

public:
    Particle& operator=(Particle const&) = delete;
    Particle& operator=(Particle&&) = default;
    Particle(const Particle&) = delete;
    Particle(Particle &&) = default;


Particle(int id, vec3 r, vec3 v, float S) : id(id), r(r), v(v), S(S), dv(vec3(0,0,0)), dS(0), d(0), T(0), P(0) {}
    friend std::ostream& operator<< (std::ostream& stream, const Particle& particle) {

        stream<<fmt::format("Particle: {0} ({1}, {2}, {3}) {4}", particle.id, particle.r.x, particle.r.y, particle.r.z, particle.S);
        return stream;
    }
};

