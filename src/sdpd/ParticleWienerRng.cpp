#include "ParticleWienerRng.hpp"

pair<mat3, float> ParticleWienerRng::getWienerIncrements( uint32_t seed, uint32_t timeStep, uint32_t p1, uint32_t p2, double deltaTime)
{
    mat3 res;
    if(p1 > p2) std::swap(p1, p2);
    array<uint32_t, 2> v{{p1, p2}};
    array<uint32_t, 4> k = keys[timeStep%4];
    k[0] = timeStep;
    k[1] = seed;
    teaEncrypt(&v[0], &k[0]);
    uint64_t val;
    (&val)[0] = v[0];
    (&val)[1] = v[1];
    rng.seed(val);
    float sqdt = pow(deltaTime, 0.5);
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            res[i][j] = sqdt * nd(rng);
    return std::make_pair(res, sqdt * nd(rng));
}
