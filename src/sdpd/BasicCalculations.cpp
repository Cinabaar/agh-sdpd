//
// Created by dawid on 20/05/15.
//

#include "BasicCalculations.h"

void BasicCalculations::calculate(CellId &cellId, CellGroup &cellGroup) {
    Cell& cell = cellGroup.cells[cellId];
    for(auto& particle : cell.particles) {
        particle.dv = particle.v;
        auto center = (rub - lbf).y/2;
        //particle.dT = (particle.v.y > 0) - (particle.v.y < 0)*(particle.r.y - center)*20;
    }
}

void BasicCalculations::integrate(Particle &particle, float deltaTime)
{
    vec3 oldPosition = particle.r;
    particle.r += particle.dv * deltaTime;
    edgeCondition(oldPosition, particle.r, particle.v);
    particle.S = (500*particle.r.y*particle.r.y*particle.r.y - 750* particle.r.y*particle.r.y +370*particle.r.y)/3;
}

void BasicCalculations::edgeCondition(vec3 oldPosition, vec3& newPosition, vec3& newVelocity) {
    bool didBump;
    vec3 inters;
    float t;
    vec3 vect = newPosition - oldPosition, newVect;
    do {
        didBump = false;
        if (newPosition.x < lbf.x) {
            newPosition.x += 2.0f * (lbf.x - newPosition.x);

            inters.x = lbf.x;
            t = (inters.x - oldPosition.x) / vect.x;
            inters.y = oldPosition.y + vect.y * t;
            inters.z = oldPosition.z + vect.z * t;
            newVect = newPosition - inters;
            newVelocity = normalize(newVect) * glm::length(newVelocity);
            didBump = true;
        }
        else if (rub.x < newPosition.x) {
            newPosition.x -= 2.0f * (newPosition.x - rub.x);

            inters.x = rub.x;
            t = (inters.x - oldPosition.x) / vect.x;
            inters.y = oldPosition.y + vect.y * t;
            inters.z = oldPosition.z + vect.z * t;
            newVect = newPosition - inters;
            newVelocity = normalize(newVect) * length(newVelocity);

            didBump = true;
        }
        else if (newPosition.y < lbf.y) {
            newPosition.y += 2.0f * (lbf.y - newPosition.y);

            inters.y = lbf.y;
            t = (inters.y - oldPosition.y) / vect.y;
            inters.x = oldPosition.x + vect.x * t;
            inters.z = oldPosition.z + vect.z * t;
            newVect = newPosition - inters;
            newVelocity = normalize(newVect) * length(newVelocity);

            didBump = true;
        }
        else if (rub.y < newPosition.y) {
            newPosition.y -= 2.0f * (newPosition.y - rub.y);

            inters.y = rub.y;
            t = (inters.y - oldPosition.y) / vect.y;
            inters.x = oldPosition.x + vect.x * t;
            inters.z = oldPosition.z + vect.z * t;
            newVect = newPosition - inters;
            newVelocity = normalize(newVect) * length(newVelocity);

            didBump = true;
        }
        else if (newPosition.z < lbf.z) {
            newPosition.z += 2.0f * (lbf.z - newPosition.z);

            inters.z = lbf.z;
            t = (inters.z - oldPosition.z) / vect.z;
            inters.x = oldPosition.x + vect.x * t;
            inters.y = oldPosition.y + vect.y * t;
            newVect = newPosition - inters;
            newVelocity = normalize(newVect) * length(newVelocity);

            didBump = true;
        }
        else if (rub.z < newPosition.z) {
            newPosition.z -= 2.0f * (newPosition.z - rub.z);

            inters.z = rub.z;
            t = (inters.z - oldPosition.z) / vect.z;
            inters.x = oldPosition.x + vect.x * t;
            inters.y = oldPosition.y + vect.y * t;
            newVect = newPosition - inters;
            newVelocity = normalize(newVect) * length(newVelocity);

            didBump = true;
        }
        vect = newVect;
        oldPosition = inters;
    } while(didBump == true);
}
