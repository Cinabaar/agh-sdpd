//
// Created by dawid on 19/05/15.
//

#pragma once

#include <mpi.h>
#include "CellGroup.h"
#include "BasicCalculations.h"

using DataToSend = vector<pair<int, vector<float>>>;
using vecvecfloat = vector<vector<float>>;
class Client {

    vector<std::array<float, 5>> outputData;
    CellGroup cellGroup;
    BasicCalculations calculations;
public:
Client(CellGroup& group, vec3 lbf, vec3 rub) : cellGroup(std::move(group)), calculations(lbf, rub) {}
    void run(float totalTime, float timeStep);
    void generateOutputData();
    void sendDataToController(bool firstTime);

    int waitForTick();

    DataToSend createDataVectors();

    int calculateInnerCells();

    void calculateEdgeCells(int edgeCellsStartAt);

    void parseData(vector<float> &vector, int length);

    DataToSend integrate(float deltaTime);

    void sendDataToNeighbors(DataToSend& dataToSend, vector<MPI_Request>& requests);

    void clearOuterCells();

    void receiveDataFromNeighbors(vecvecfloat& buffers, vector<int>& counts, vector<MPI_Request>& request, vector<MPI_Status>& status);

};
