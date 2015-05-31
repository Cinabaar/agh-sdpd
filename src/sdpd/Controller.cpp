//
// Created by dawid on 17/05/15.
//
#include <cmath>
#include <vector>
#include <iostream>
#include "format.h"
#include "Controller.h"
#include "CellGroup.h"
#include <chrono>
#include <thread>

using namespace fmt;

Controller::Controller(int count, float kB, float K, float n, float z, float N, float M, float h, float T0, float totalTime, float timeStep,
                       glm::vec3 ldf, glm::vec3 rub, int slaves) : _count(count), _kB(kB), _K(K), _n(n), _z(z), _N(N), _M(M), _h(h), T0(T0),
                                                                   totalTime(totalTime), timeStep(timeStep), _ldf(ldf), _rub(rub), _slaves(slaves),
                                                                   buffers(slaves) 
{}

bool Controller::initialize() {
    if (_slaves < 1) {
        return false;
    }
    auto start = MPI_Wtime();
    int h_x, h_y, h_z;
    h_x = (int) std::ceil((_rub - _ldf).x / _h);
    h_y = (int) std::ceil((_rub - _ldf).y / _h);
    h_z = (int) std::ceil((_rub - _ldf).z / _h);

    int cells = h_x * h_y * h_z;
    std::vector<std::vector<int>> slave_cells(_slaves);

    int i = 0, j = 0, k = 0;
    int *ref_h, *ref_it;

    if (h_x > h_y && h_x > h_z) {
        ref_h = &h_x;
        ref_it = &k;
    }
    else if (h_y > h_x && h_y > h_z) {
        ref_h = &h_y;
        ref_it = &j;
    }
    else {
        ref_h = &h_z;
        ref_it = &i;
    }


    int cut = (int) std::round((*ref_h) / (float) _slaves);
    for (i = 0; i < h_z; i++) {
        for (j = 0; j < h_y; j++) {
            for (k = 0; k < h_x; k++) {
                auto current_slave = std::min((*ref_it) / cut, _slaves - 1);
                slave_cells[current_slave].push_back(k + j * h_x + i * h_x * h_y);
            }
        }
    }
    std::vector<CellGroup> cellGroups;
    for (int i = 0; i < _slaves; i++) {
        CellGroup cellGroup(i + 1, h_x, h_y, h_z, _h, _ldf, _rub);
        for (int &c : slave_cells[i]) {
            int c_x = c % h_x;
            int c_y = (c % (h_x * h_y)) / h_x;
            int c_z = (c / (h_x * h_y));
            Cell cell(c, cellGroup.id);
            cell.x = c_x;
            cell.y = c_y;
            cell.z = c_z;
            cell.h = _h;
            cellGroup.cells[c] = std::move(cell);
        }
        cellGroups.push_back(std::move(cellGroup));
    }

    std::mt19937 gen;
    std::uniform_real_distribution<double> rd(0.0f, 1.0f);
    
    float x = _rub.x - _ldf.x;
    float y = _rub.y - _ldf.y;
    float z = _rub.z - _ldf.z;
    
    int p_x = pow(_count * x * x / (y * z), 1.0/3);
    int p_y = pow(_count * y * y / (x * z), 1.0/3);
    int p_z = pow(_count * z * z / (x * y), 1.0/3);

    std::cout<<"Particle dimensions: "<<p_x<<" "<<p_y<<" "<<p_z<<std::endl;
    std::cout<<"Box dimensions: "<<h_x<<" "<<h_y<<" "<<h_z<<std::endl;
    std::cout<<"Cell width: "<<_h<<std::endl;
    _count = p_x*p_y*p_z;
    
    float itx=x/p_x/2.0;
    float ity=y/p_y/2.0;
    float itz=z/p_z/2.0;
    int it=0;
    for(int i=0;i<p_z;i++)
    {
        for(int j=0;j<p_y;j++)
        {
            for(int k=0;k<p_x;k++)
            {
                glm::vec3 r(itx, ity, itz);
                //std::cout<<r.x<<" "<<r.y<<" "<<r.z<<std::endl;
                itx+=x/p_x;
                glm::vec3 v(rd(gen), rd(gen), rd(gen));
                v = v / glm::length(v);
                //v = (3.0f * _kB * T0 * _N * _count / _M) * v;
                Particle p(it++, r, glm::vec3(0), 2);
                int c_x, c_y, c_z;
                c_x = (int) (r.x / _h);
                c_y = (int) (r.y / _h);
                c_z = (int) (r.z / _h);
                int cid = (c_x + c_y * h_x + c_z * h_x * h_y);

                for(CellGroup& group : cellGroups) {
                    if(group.cells.find(cid) != group.cells.end()) {
                        group.cells[cid].particles.push_back(std::move(p));
                    }
                }
    
            }
            itx = x/p_x/2.0;
            ity+=y/p_y;
        }
        itx = x/p_x/2.0;
        ity = y/p_y/2.0;
        itz+=z/p_z;
    }
        
    /*


    for (int i = 0; i < _count; i++)
    {
        float r_x = (_rub - _ldf).x * rd(gen) + _ldf.x;
        float r_y = (_rub - _ldf).y * rd(gen) + _ldf.y; 
        float r_z = (_rub - _ldf).z * rd(gen) + _ldf.z;
        glm::vec3 r(r_x, r_y, r_z);
        glm::vec3 v(rd(gen), rd(gen), rd(gen));
        v = v / glm::length(v);
        v = (3.0f * _kB * T0 * _N * _count / _M) * v;
        Particle p(i, glm::vec3(r.x, r.y, r.z), v, 2);
        int c_x, c_y, c_z;
        c_x = (int) (r.x / _h);
        c_y = (int) (r.y / _h);
        c_z = (int) (r.z / _h);
        int cid = (c_x + c_y * h_x + c_z * h_x * h_y);

        for(CellGroup& group : cellGroups) {
            if(group.cells.find(cid) != group.cells.end()) {
                group.cells[cid].particles.push_back(std::move(p));
            }
        }
    }*/
    std::vector<std::vector<float>> init_messages(_slaves);

    for(int i=0;i<_slaves;i++)
    {
        int group_count = (_slaves == 1) ? 1 : ((i == 0 || i == _slaves-1) ? 2 : 3);
        init_messages[i].push_back(_M/_count);
        init_messages[i].push_back(_M);
        init_messages[i].push_back(_N);
        init_messages[i].push_back(_kB);
        init_messages[i].push_back(_K);
        init_messages[i].push_back(_n);
        init_messages[i].push_back(_z);
        init_messages[i].push_back(_ldf.x);
        init_messages[i].push_back(_ldf.y);
        init_messages[i].push_back(_ldf.y);
        init_messages[i].push_back(_rub.x);
        init_messages[i].push_back(_rub.y);
        init_messages[i].push_back(_rub.z);
        init_messages[i].push_back(_h);
        init_messages[i].push_back(totalTime);
        init_messages[i].push_back(timeStep);
        init_messages[i].push_back(h_x);
        init_messages[i].push_back(h_y);
        init_messages[i].push_back(h_z);
        init_messages[i].push_back(group_count);
        init_messages[i].push_back(cellGroups[i].id);
        init_messages[i].push_back(cellGroups[i].cells.size());
        for(auto const& cell : cellGroups[i].cells)
        {
            init_messages[i].push_back(cell.first);
            init_messages[i].push_back(cell.second.particles.size());
            for(Particle const& part : cell.second.particles)
            {
                init_messages[i].push_back(part.id);
                init_messages[i].push_back(part.r.x);
                init_messages[i].push_back(part.r.y);
                init_messages[i].push_back(part.r.z);
                init_messages[i].push_back(part.v.x);
                init_messages[i].push_back(part.v.y);
                init_messages[i].push_back(part.v.z);
                init_messages[i].push_back(part.S);
            }
        }
        if(i>0)
        {
            init_messages[i].push_back(cellGroups[i-1].id);
            init_messages[i].push_back(cellGroups[i-1].cells.size());
            for(auto const& cell : cellGroups[i-1].cells) {
                init_messages[i].push_back(cell.first);
                init_messages[i].push_back(0);
            }
        }
        if(i<_slaves-1)
        {
            init_messages[i].push_back(cellGroups[i+1].id);
            init_messages[i].push_back(cellGroups[i+1].cells.size());
            for(auto const& cell : cellGroups[i+1].cells) {
                init_messages[i].push_back(cell.first);
                init_messages[i].push_back(0);
            }
        }
    }
    std::vector<MPI_Request> requests(_slaves);
    std::vector<MPI_Status> status(_slaves);
    for(i=0;i<_slaves;i++)
    {
        MPI_Isend(&(init_messages[i][0]), init_messages[i].size(), MPI_FLOAT, i+1, 0, MPI_COMM_WORLD, &requests[i]);
    }
    MPI_Waitall(_slaves, &requests[0], &status[0]);

    //std::cout<<mid - start<<" "<<end-mid<<std::endl;

    return true;
}

int tick = 0;
void Controller::waitForData(vector<MPI_Request>& requests, vector<MPI_Status>& status) {

    outputData = vector<glm::vec4>(_count);
    MPI_Waitall(_slaves, &requests[0], &status[0]);
    for(int i=0;i<_slaves;i++)
    {
        int length;
        MPI_Get_count(&status[i], MPI_FLOAT, &length);
        for(int j=0;j<length;j+=5){
            outputData[buffers[i][j]] = glm::vec4(buffers[i][j+1],buffers[i][j+2],buffers[i][j+3],buffers[i][j+4]);
        }
    }
}

void Controller::run() {
    vector<MPI_Request> requests(_slaves);
    vector<MPI_Status> status(_slaves);
    double time = MPI_Wtime();
    std::mt19937 rng;
    sendTick(tick++, rng());
    std::cout<<fmt::format("{0}. {1} {2}", "Controller", "Tick: ", MPI_Wtime() - time)<<std::endl;
    time = MPI_Wtime();
    while(tick*timeStep < totalTime)
    {
        for(int i=0;i<_slaves;i++)
        {
            buffers[i].reserve(_count*5);
            MPI_Irecv(&buffers[i][0], _count*5, MPI_FLOAT, i+1, 2, MPI_COMM_WORLD, &requests[i]);
        }
        std::cout<<fmt::format("{0}. {1} {2}", "Controller", "Receiving: ", MPI_Wtime() - time)<<std::endl;
        time = MPI_Wtime();
        sendTick(tick, rng());
        std::cout<<fmt::format("{0}. {1} {2}", "Controller", "Tick: ", MPI_Wtime() - time)<<std::endl;
        time = MPI_Wtime();
        waitForData(requests, status);
        std::cout<<fmt::format("{0}. {1} {2}", "Controller", "Waiting for data: ", MPI_Wtime() - time)<<std::endl;
        time = MPI_Wtime();
        printer.addData(outputData, fmt::format("output{0}.data", tick-1));
        std::cout<<fmt::format("{0}. {1} {2}", "Controller", "Printing data: ", MPI_Wtime() - time)<<std::endl;
        time = MPI_Wtime();
        printer.printData(tick-1); 
        tick++;
    }
    std::cout<<"Printing data"<<std::endl;
    printer.printData();
}

void Controller::sendTick(int tick, int seed) {
    array<int, 2> data{{tick, seed}};
    for(int i=0;i<_slaves;i++)
    {
        MPI_Send(&data[0], 2, MPI_INT, i+1, 1, MPI_COMM_WORLD);
    }
}
