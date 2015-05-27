#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <random>
#include <glm/glm.hpp>

using std::vector;
using std::array;
using std::mt19937_64;
using glm::mat3;
using std::pair;
class ParticleWienerRng
{
    array<array<uint32_t, 4>, 4> keys {{ {{0xA3, 0x41, 0x31, 0x6C}}, {{0xC8, 0x01, 0x3E, 0xA4}},
                                         {{0xAD, 0x90, 0x77, 0x7D}},  {{0x7E, 0x95, 0x76, 0x1E}} }};
    mt19937_64 rng;
    std::normal_distribution<float> nd;
private:
    void teaEncrypt (uint32_t* v, uint32_t* k) {
        uint32_t v0=v[0], v1=v[1], sum=0, i;           /* set up */
        uint32_t delta=0x9e3779b9;                     /* a key schedule constant */
        uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
        for (i=0; i < 32; i++) {                       /* basic cycle start */
            sum += delta;
            v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
            v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
        }                                              /* end cycle */
        v[0]=v0; v[1]=v1;
    } 
public:
    ParticleWienerRng() : nd(0, 1) {}
    pair<mat3, float> getWienerIncrements(uint32_t seed, uint32_t tick, uint32_t p1, uint32_t p2, double deltaTime);        
        
};
    
