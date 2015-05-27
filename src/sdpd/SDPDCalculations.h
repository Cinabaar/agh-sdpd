#pragma once

#include "Cell.h"
#include "CellGroup.h"
#include "Particle.h"
#include <cmath>
#include "ParticleWienerRng.hpp"
using glm::mat3;
using std::move;
class SDPDCalculations
{
private:
    float n, m, N, z, K, kB, h, totalTime, timeStep;
    double PI{4*std::atan(1)};
    float Ci{3.0/2.0}; //Heat capacity at constant volume. Monoatomic gas 3/2, diatomic gas 5/2
    vec3 lbf, rub;
    uint32_t seed, tick;
    ParticleWienerRng wienerRng;
    void edgeCondition(vec3& oldPosition, vec3& newPosition, vec3& newVelocity);
    float W(float r);
    float W(Particle& i, Particle& j);
    float a(Particle& i, Particle& j);
    float b(Particle& i, Particle& j);
    float c(Particle& i, Particle& j);
    float A(Particle& i, Particle& j);
    float B(Particle& i, Particle& j);
    float C(Particle& i, Particle& j);
    float F(float r);
    float F(Particle& i, Particle& j);
    vec3 r(Particle& i, Particle& j);
    float d(Particle& i, Particle& j);
    vec3 e(Particle& i, Particle& j);
    float tr(mat3& dW);
    mat3 dWW(mat3& dW, mat3& tr);
    vec3 matDotVec(mat3&& mat, vec3&& vec);
    float matDotDotMat(mat3&& matl, mat3&& matr);
    mat3 vecVec(vec3&& vecl, vec3&& vecr);
    float vecDotVec(vec3&& vecl, vec3&& vecr);
    vec3 eedotv(Particle& i, Particle& j);
    float vdotv(Particle& i, Particle& j);
    float edotvsq(Particle& i, Particle& j);
    //TODO:
    mat3 dW(Particle& i, Particle& j);
    float dV(Particle& i, Particle& j);
public:
SDPDCalculations(vec3 lbf, vec3 rub, float N, float n, float m, float z, float K, float kB, float h, float totalTime, float timeStep) :
    lbf(lbf), rub(rub), N(N), n(n), m(m), z(z), K(K), kB(kB), h(h), totalTime(totalTime), timeStep(timeStep) {} 
    void setDataForRng(uint32_t seed, uint32_t tick) { this->seed = seed; this->tick = tick; }
    void calculateIncrements(Cell& cell, float deltaTime);
    void calculateDensities(Cell&  cell);
    void integrate(Particle& particle, float deltaTime);
};
