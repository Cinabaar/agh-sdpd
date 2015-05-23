//
// Created by dawid on 17/05/15.
//

#include "Particle.h"

Particle::Particle(int id, vec3 r, vec3 v, float T) :
    id(id), r(r), v(v), T(T), dv(vec3(0,0,0)), dT(0)
{

}
