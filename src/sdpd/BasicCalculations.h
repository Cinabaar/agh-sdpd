//
// Created by dawid on 20/05/15.
//
#pragma once


#include "Cell.h"
#include "CellGroup.h"

class BasicCalculations {

    vec3 lbf, rub;
public:
    BasicCalculations(vec3 lbf, vec3 rub) : lbf(lbf), rub(rub) {}
    void calculate(CellId &cellId, CellGroup &cellGroup);


    void integrate(Particle &particle, float deltaTime);

    void edgeCondition(vec3 oldPosition, vec3& newPosition, vec3& newVelocity);
};

