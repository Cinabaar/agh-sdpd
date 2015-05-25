//
// Created by dawid on 19/05/15.
//
#include <mpi.h>
#include "Client.h"

using std::cout;
using std::endl;

void Client::sendDataToController(bool firstTime) {

    static MPI_Request request;
    static MPI_Status status;
    if(!firstTime)
    {
        MPI_Wait(&request, &status);
    }
    outputData.clear();
    for(auto& c : cellGroup.innerCells) {
        for(Particle& p : cellGroup.cells[c.first].particles)
        {
            outputData.push_back(std::array<float, 5> {{(float)p.id, p.r.x, p.r.y, p.r.z, p.S}});
        }
    }
    MPI_Isend(&outputData[0], outputData.size()*5, MPI_FLOAT, 0, 2, MPI_COMM_WORLD, &request);
}


void Client::run(float totalTime, float timeStep) {
    int tickNumber = 0;
    sendDataToController(true);
    size_t neighbor_count = cellGroup.neighbors_to_share_with.size();
    vector<vector<float>> buffers(neighbor_count);
    vector<MPI_Request> requests(neighbor_count);
    vector<MPI_Status> status(neighbor_count);
    vector<int> counts(neighbor_count);

    for(auto& buffer : buffers) {
        buffer.resize(1000000);
    }
    tickNumber = waitForTick();
    double time = MPI_Wtime();
    while(tickNumber!=(int)(totalTime/timeStep)) {
        if(neighbor_count > 0)
        {
            auto dataToSend = createDataVectors();
            cout<<fmt::format("{0}. {1} {2}", cellGroup.id, "Gathering data to share: ", MPI_Wtime() - time)<<endl;
            time = MPI_Wtime();
            sendDataToNeighbors(dataToSend, requests);
            cout<<fmt::format("{0}. {1} {2}", cellGroup.id, "Sending data to neighbors: ", MPI_Wtime() - time)<<endl;
            time = MPI_Wtime();
        }
        auto edgeCellsStartAt = calculateInnerCells();
        cout<<fmt::format("{0}. {1} {2}", cellGroup.id, "Calculating inner cells: ", MPI_Wtime() - time)<<endl;
        time = MPI_Wtime();
        if(neighbor_count>0)
        {
            receiveDataFromNeighbors(buffers, counts, requests, status);
            cout<<fmt::format("{0}. {1} {2}", cellGroup.id, "Receiving data from neighbors: ", MPI_Wtime() - time)<<endl;
            time = MPI_Wtime();
            for(int i=0;i<buffers.size();i++)
            {
                parseData(buffers[i], counts[i]);
            }
            cout<<fmt::format("{0}. {1} {2}", cellGroup.id, "Parsing data from neighbors: ", MPI_Wtime() - time)<<endl;
            time = MPI_Wtime();
        }
        calculateEdgeCells(edgeCellsStartAt);
        cout<<fmt::format("{0}. {1} {2}", cellGroup.id, "Calculating edge cells: ", MPI_Wtime() - time)<<endl;
        time = MPI_Wtime();
        clearOuterCells();
        cout<<fmt::format("{0}. {1} {2}", cellGroup.id, "Clearing outer cells: ", MPI_Wtime() - time)<<endl;
        time = MPI_Wtime();
        auto particlesOoB = integrate(timeStep);
        cout<<fmt::format("{0}. {1} {2}", cellGroup.id, "Integrating: ", MPI_Wtime() - time)<<endl;
        time = MPI_Wtime();
        if(neighbor_count > 0)
        {
            sendDataToNeighbors(particlesOoB, requests);
            cout<<fmt::format("{0}. {1} {2}", cellGroup.id, "Sending data to neighbors2: ", MPI_Wtime() - time)<<endl;
            time = MPI_Wtime();
            receiveDataFromNeighbors(buffers, counts, requests, status);
            cout<<fmt::format("{0}. {1} {2}", cellGroup.id, "Receiving data from neighbors2: ", MPI_Wtime() - time)<<endl;
            time = MPI_Wtime();
            for(int i=0;i<buffers.size();i++)
            {
                parseData(buffers[i], counts[i]);
            }
            cout<<fmt::format("{0}. {1} {2}", cellGroup.id, "Parsing data from neighbors2: ", MPI_Wtime() - time)<<endl;
            time = MPI_Wtime();
        }
        sendDataToController(false);
        cout<<fmt::format("{0}. {1} {2}", cellGroup.id, "Sending data to controller: ", MPI_Wtime() - time)<<endl;
        time = MPI_Wtime();
        tickNumber = waitForTick();
        cout<<fmt::format("{0}. {1} {2}", cellGroup.id, "Waiting for tick: ", MPI_Wtime() - time)<<endl;
        time = MPI_Wtime();
    }
}

vector<pair<int, vector<float>>> Client::createDataVectors()
{
    vector<pair<int, vector<float>>> dataToSend(cellGroup.neighbors_to_share_with.size());
    int it = 0;
    for(auto& neighborCell : cellGroup.neighbors_to_share_with)
    {
        dataToSend[it].first = neighborCell.first;
        for(auto& c : neighborCell.second) {
            Cell& cell = cellGroup.cells[c];
            for(auto& p: cell.particles)
            {
                dataToSend[it].second.push_back(p.id);
                dataToSend[it].second.push_back(p.r.x);
                dataToSend[it].second.push_back(p.r.y);
                dataToSend[it].second.push_back(p.r.z);
                dataToSend[it].second.push_back(p.v.x);
                dataToSend[it].second.push_back(p.v.y);
                dataToSend[it].second.push_back(p.v.z);
                dataToSend[it].second.push_back(p.S);
            }
        }
        it++;
    }
    return dataToSend;
}

void Client::parseData(vector<float>& buffer, int length) {
    int it = 0;
    if(length <= 0)
        return;
    while(it < length)
    {
        Particle p(buffer[it], glm::vec3(buffer[it+1], buffer[it+2], buffer[it+3]), glm::vec3(buffer[it+4], buffer[it+5], buffer[it+6]), buffer[it+7]);
        it+=8;
        int cid = cellGroup.xyzToCellId(p.r.x, p.r.y, p.r.z);
        cellGroup.cells[cid].particles.push_back(std::move(p));
    }
}

void Client::clearOuterCells() {
    for(auto& c : cellGroup.outerCells) {
        cellGroup.cells[c.first].particles.clear();
    }
}


int Client::waitForTick()
{
    int tick;
    MPI_Status status;
    int rank;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Recv(&tick, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    return tick;
}

int Client::calculateInnerCells() {
    int i;
    for(i=0; i < cellGroup.innerCells.size(); i++){
        if(cellGroup.innerCells[i].second.size() > 0 && cellGroup.innerCells[i].second[0].second == 1)
            break;
        calculations.calculate(cellGroup.innerCells[i].first, cellGroup);
    }
    return i;
}

void Client::calculateEdgeCells(int edgeCellsStartAt) {
    for(int i=edgeCellsStartAt; i < cellGroup.innerCells.size(); i++){
        calculations.calculate(cellGroup.innerCells[i].first, cellGroup);
    }
}

DataToSend Client::integrate(float deltaTime) {
    vector<pair<int, vector<float>>> dataToSend(cellGroup.neighbors_to_share_with.size());
    int it = 0;
    for(auto& c : cellGroup.neighbors_to_share_with)
    {
        dataToSend[it].first = c.first;
        it++;
    }
    int i=0;
    for(auto& cell : cellGroup.cells)
    {   i++;
        for(auto particleIt = cell.second.particles.begin(); particleIt != cell.second.particles.end();)
        {
            auto& particle = *particleIt;
            calculations.integrate(particle, deltaTime);
            int cid = cellGroup.xyzToCellId(particle.r.x, particle.r.y, particle.r.z);
            if(cid != cell.first) {
                auto& newCell = cellGroup.cells[cid];
                if(newCell.cellGroupId == cellGroup.id)
                {
                    newCell.particles.push_back(std::move(particle));
                }
                else
                {
                    for(auto& d : dataToSend)
                    {
                        if(d.first == newCell.cellGroupId)
                        {
                            d.second.push_back(particle.id);
                            d.second.push_back(particle.r.x);
                            d.second.push_back(particle.r.y);
                            d.second.push_back(particle.r.z);
                            d.second.push_back(particle.v.x);
                            d.second.push_back(particle.v.y);
                            d.second.push_back(particle.v.z);
                            d.second.push_back(particle.S);
                        }
                    }
                }
                particleIt = cell.second.particles.erase(particleIt);
            }
            else{
                particleIt++;
            }
        }
    }
    return dataToSend;
}

void Client::sendDataToNeighbors(DataToSend& dataToSend, vector<MPI_Request>& requests) {

    for(int i=0;i<dataToSend.size();i++)
    {
        auto& data = dataToSend[i];
        MPI_Isend(&data.second[0], data.second.size(), MPI_FLOAT, data.first, 0, MPI_COMM_WORLD, &requests[i]);
    }
}

void Client::receiveDataFromNeighbors(vecvecfloat& buffers, vector<int>& counts, vector<MPI_Request>& requests, vector<MPI_Status>& status) {
    int it=0;
    for(auto& neighbor : cellGroup.neighbors_to_share_with)
    {
        MPI_Irecv(&buffers[it][0], 1000000, MPI_FLOAT, neighbor.first, 0, MPI_COMM_WORLD, &requests[it]);
        it++;
    }

    MPI_Waitall(requests.size(), &requests[0], &status[0]);
    for(int i=0;i<counts.size();i++)
    {
        MPI_Get_count(&status[i], MPI_FLOAT, &counts[i]);
    }
}
