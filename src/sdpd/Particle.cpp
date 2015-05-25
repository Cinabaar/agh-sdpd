//
// Created by dawid on 17/05/15.
//

#include "Particle.h"

Particle::Particle(int id, vec3 r, vec3 v, float S) :
    id(id), r(r), v(v), S(S), dv(vec3(0,0,0)), dS(0)
{

}
