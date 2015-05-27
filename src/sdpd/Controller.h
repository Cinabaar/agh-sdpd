//
// Created by dawid on 17/05/15.
//

#pragma once

#include <glm/glm.hpp>
#include "Cell.h"
#include "OutputDataPrinter.h"
#include <mpi.h>
class Controller {
private:
    int _count;
    float _kB;
    float _K;
    float _n;
    float _z;
    float _N;
    float _M;
    float _h;
    float T0;
    float totalTime;
    float timeStep;
    glm::vec3 _ldf;
    glm::vec3 _rub;
    int _slaves;

    vector<vector<float>> buffers;
    vector<glm::vec4> outputData;

    OutputDataPrinter printer;
public:
    Controller(int count, float kB, float K, float n, float z, float N, float M, float h, float T0, float totalTime, float timeStep,
               glm::vec3 ldf, glm::vec3 rub, int slaves);

    Controller(const Controller &) = delete;
    Controller & operator=(Controller const&) = delete;
    Controller(Controller &&) = delete;
    Controller & operator=(Controller &&) = delete;

    bool initialize();

    void waitForData(vector<MPI_Request>& requests, vector<MPI_Status>& status);
    void run();

    void sendTick(int tick, int seed);
};
