#include "SDPDCalculations.h"
using glm::distance;
void SDPDCalculations::integrate( Particle &particle, float deltaTime )
{
    particle.S += particle.dS;
    particle.v += particle.dv;
    vec3 oldPos = particle.r;
    particle.r += particle.v * deltaTime;
    edgeCondition(oldPos, particle.r, particle.v);
}

void SDPDCalculations::calculateDensities(Cell& cell)
{
    for(auto& pi : cell.particles)
    {
        for(auto& c : cell.neighbors)
        {
            if(c == nullptr) continue;
            for(auto& pj : c->particles)
            {
                pi.d += W(pi, pj); 
            }
        }
        pi.T = N / (2*PI) * pi.d * exp(pi.S - 2); //34
        pi.P = pi.d * pi.T; //34
    } 
}

void SDPDCalculations::calculateIncrements( Cell& cell, float dt)
{
    //for(auto& pi : cell.particles)
    //    std::cout<<pi.id<<" "<<pi.d<<" "<<pi.S<<" "<<m*pi.d<<" "<<pi.T<<"  "<<pi.P<<std::endl;
    vec3 mdvi(0,0,0);
    float TdSi=0;
    for(auto& pi : cell.particles)
    {
        for(auto& c : cell.neighbors)
        {
            if(c==nullptr) continue;    
            for(auto& pj : c->particles)
            {
                if(distance(pi.r, pj.r) >= h) continue;
                auto increments = wienerRng.getWienerIncrements(seed, tick, pi.id, pj.id, dt);
                mat3& dW = increments.first;
                mat3 _tr = tr(dW) * mat3(1);
                float& dV = increments.second;
                if(pi.id > pj.id)
                    dV = -dV;
                mat3 _dWW = dWW(dW, _tr);

                vec3 mdvi1, mdvi2, mdvi3, mdvi4;
                float tdsi1, tdsi2, tdsi3, tdsi4, tdsi5;
                //std::cout<<dW[0][0]<<" "<<dW[0][1]<<" "<<dW[0][2]<<" "<<dW[1][0]<<" "<<dW[1][1]<<" "<<dW[1][2]<<" "<<dW[2][0]<<" "<<dW[2][1]<<" "<<dW[2][2]<<std::endl;
                //std::cout<<_dWW[0][0]<<" "<<_dWW[0][1]<<" "<<_dWW[0][2]<<" "<<_dWW[1][0]<<" "<<_dWW[1][1]<<" "<<_dWW[1][2]<<" "<<_dWW[2][0]<<" "<<_dWW[2][1]<<" "<<_dWW[2][2]<<std::endl;
                //std::cout<<_tr[0][0]<<" "<<_tr[0][1]<<" "<<_tr[0][2]<<" "<<_tr[1][0]<<" "<<_tr[1][1]<<" "<<_tr[1][2]<<" "<<_tr[2][0]<<" "<<_tr[2][1]<<" "<<_tr[2][2]<<std::endl;
                //std::cout<<dV<<std::endl;
                //std::cout<<a(pi, pj)<<" "<<b(pi, pj)<<" "<<d(pi, pj)<<" "<<A(pi, pj)<<" "<<B(pi, pj)<<" "<<C(pj, pj)<<" "<<W(pi, pj)<<" "<<F(pi, pj)<<" "<<distance(pi.r, pj.r)<<std::endl;
                mdvi1 = ((pi.P / (pi.d*pi.d)) + (pj.P / (pj.d * pj.d)))*F(pi, pj)*(pi.r-pj.r)*dt;
                mdvi2 = (1-d(pi,pj))*a(pi, pj)*(pi.v - pj.v)*dt;
                mdvi3 = (1-d(pi,pj))*(a(pi, pj)/3.0f + b(pi,pj))*eedotv(pi, pj)*dt;
                mdvi4 = matDotVec(A(pi, pj)*_dWW + B(pi, pj)*(1/3.0f)*_tr, e(pi, pj));

                tdsi1 = 0.5f*(1-d(pi, pj) - (pj.T/ (pi.T + pj.T))*(kB/Ci))*(a(pi,pj)*vdotv(pi, pj)+(a(pi, pj)/3.0f+b(pi,pj))*edotvsq(pi, pj))*dt;
                tdsi2 = 2.0f*kB/m*(pi.T*pj.T/(pi.T+pj.T))*(10.0/3*a(pi, pj) + b(pi, pj))*dt;
                tdsi3 = 2*K*F(pi, pj)/(pi.d*pj.d)*(pi.T-pj.T)*dt;
                tdsi4 = 2*K*kB/Ci*F(pi, pj)/(pi.d*pj.d)*pj.T*dt;
                tdsi5 = -0.5 * matDotDotMat((A(pi,pj)*_dWW + B(pi,pj)*(1.0f/3)*_tr), vecVec(e(pi, pj), pi.v - pj.v)) + C(pi, pj)*dV;
                mdvi = mdvi1 + mdvi2 + mdvi3 + mdvi4;
                TdSi = tdsi1 + tdsi2 + tdsi3 + tdsi4 + tdsi5;
                //std::cout<<fmt::format("({0}, {1}, {2}) = ({3}, {4}, {5}) + ({6}, {7}, {8}) + ({9}, {10}, {11}) + ({12}, {13}, {14})",
                //                       mdvi[0], mdvi[1], mdvi[2], mdvi1[0], mdvi1[1], mdvi1[2], mdvi2[0], mdvi2[1],
                //                       mdvi2[2], mdvi3[0], mdvi3[1], mdvi3[2], mdvi4[0], mdvi4[1], mdvi4[2])<<std::endl;
                //std::cout<<fmt::format("{0} = {1} + {2} + {3} + {4} + {5}", TdSi, tdsi1, tdsi2, tdsi3, tdsi4, tdsi5)<<std::endl;
            }

        }
        pi.dv = mdvi / m;
        pi.dS = TdSi / pi.T;
        mdvi = vec3(0);
        TdSi = 0;
    }
}


float SDPDCalculations::a( Particle &i, Particle &j )
{
    return (5.0*n/3.0 - z) * (F(i,j) / (i.d*j.d));
}

float SDPDCalculations::b(Particle& i, Particle& j)
{
    return 5*(n/3.0 + z)*(F(i,j) / (i.d * j.d)) - a(i,j)/3.0;
}
float SDPDCalculations::c(Particle& i, Particle& j)
{
    return 2*K*F(i,j)/(i.d * j.d);
}

float SDPDCalculations::A(Particle& i, Particle& j)
{
    auto Aij = 8.0*kB*((i.T*j.T) / (i.T + j.T))*(5*n/3.0 - z)*(F(i,j) / (i.d * j.d));
    return pow(Aij, 0.5);
}

float SDPDCalculations::B(Particle& i, Particle& j)
{
    auto Bij = 8.0 * kB * ((i.T * j.T) / (i.T + j.T)) * (5*n/3.0 + 8*z) * (F(i,j) / (i.d + j.d));
    return pow(Bij, 0.5);
}

float SDPDCalculations::C(Particle& i, Particle& j)
{
    auto Cij = 4*K*kB*i.T*j.T*(F(i,j) / (i.d*j.d));
    return pow(Cij, 0.5);
}

float SDPDCalculations::F(Particle& i, Particle& j)
{
    return F(distance(i.r, j.r));
}
        
float SDPDCalculations::F(float r)
{
    if(r>=h) return 0;
    return 315.0 / (4*PI*h*h*h*h*h) * (1.0 - r/h) * (1.0 - r/h);
}

vec3 SDPDCalculations::r(Particle& i, Particle& j)
{
    return i.r - j.r; 
}

float SDPDCalculations::d(Particle& i, Particle& j)
{
    return (i.T*j.T)/((i.T+j.T)*(i.T+j.T))*(kB/Ci + kB/Ci);
}

vec3 SDPDCalculations::e(Particle& i, Particle& j)
{
    return (i.r - j.r) / distance(i.r, j.r);
}

vec3 SDPDCalculations::eedotv(Particle& i, Particle& j)
{
    auto eij = e(i,j);
    auto vij = i.v - j.v;
    auto eeij = vecVec(move(eij), move(eij));
    return matDotVec(move(eeij), move(vij));
}

float SDPDCalculations::vdotv(Particle& i, Particle& j)
{
    auto vij = i.v - j.v;
    return vecDotVec(move(vij), move(vij));
}

float SDPDCalculations::edotvsq(Particle& i, Particle& j)
{ 
    auto eij = e(i, j); 
    auto vij = i.v - j.v;
    auto edotv = vecDotVec(move(eij), move(vij));
    return edotv*edotv;
}

float SDPDCalculations::tr(mat3& dW)
{
    return dW[0][0] + dW[1][1] + dW[2][2];
}

mat3 SDPDCalculations::dWW(mat3& dW, mat3& tr)
{
    mat3 t;
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            t[i][j] = dW[i][j] + dW[j][i];
    return 0.5f*t - (1/3.0f)*tr;
}

vec3 SDPDCalculations::matDotVec(mat3&& mat, vec3&& vec)
{
    vec3 res;
    res[0] = mat[0][0] * vec[0] + mat[0][1] * vec[1] + mat[0][2] * vec[2];
    res[1] = mat[1][0] * vec[0] + mat[1][1] * vec[1] * mat[1][2] * vec[2];
    res[2] = mat[2][0] * vec[0] + mat[2][1] * vec[1] * mat[2][2] * vec[2];
    return res;
}

float SDPDCalculations::matDotDotMat(mat3&& l, mat3&& r)
{
    float res = l[0][0]*r[0][0] + l[0][1]*r[1][0] + l[0][2]*r[2][0] +
        l[1][0]*r[0][1] + l[1][1]*r[1][1] + l[1][2]*r[2][1] +
        l[2][0]*r[0][2] + l[2][1]*r[1][2] + l[2][2]*r[2][2];
    return res;
}

mat3 SDPDCalculations::vecVec(vec3&& l, vec3&& r)
{
    mat3 res;
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            res[i][j] = l[i]*r[j];
    return res;
}

float SDPDCalculations::vecDotVec(vec3&& l, vec3&& r)
{
    float res = l[0]*r[0]+l[1]*r[1]+l[2]*r[2];
    return res;
}

float SDPDCalculations::W(Particle& i, Particle& j)
{
    return W(distance(i.r, j.r));
}

float SDPDCalculations::W(float r)
{
    if(r>=h) return 0;
    return (105.0f / (16*PI*h*h*h)) * (1+3*(r/h)) * (1-(r/h)) * (1-(r/h)) * (1-(r/h));
}

void SDPDCalculations::edgeCondition(vec3& oldPosition, vec3& newPosition, vec3& newVelocity)
{ 
    bool didBump=true;
    vec3 inters;
    float t;
    vec3 vect = newPosition - oldPosition, newVect;
    for(int i=0;i<3 && didBump==true;i++)
    {
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
    }
    if(didBump == true)
    {
        newPosition.x = newPosition.x > rub.x ? rub.x : newPosition.x; 
        newPosition.x = newPosition.x < lbf.x ? lbf.x : newPosition.x;
        newPosition.y = newPosition.x > rub.y ? rub.y : newPosition.y;
        newPosition.y = newPosition.x < lbf.y ? lbf.y : newPosition.y;
        newPosition.z = newPosition.x > rub.z ? rub.z : newPosition.z;
        newPosition.z = newPosition.x > lbf.z ? lbf.z : newPosition.z;
        newVelocity = vec3(0);
    }
}
