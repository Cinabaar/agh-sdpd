//
// Created by dawid on 19/05/15.
//
#include <mpi.h>
#include "Client.h"

void Client::sendDataToController() {
    MPI_Send(&outputData[0], outputData.size()*5, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
}

void Client::generateOutputData() {
    outputData.clear();
    outputData.reserve(cellGroup.getParticleCount());
    for(auto& c : cellGroup.innerCells) {
        for(Particle& p : cellGroup.cells[c.first].particles)
        {
            outputData.push_back(std::array<float, 5> {(float)p.id, p.r.x, p.r.y, p.r.z, p.T});
        }
    }
}

void Client::run(float totalTime, float timeStep) {
    int tickNumber = 0;
    sendDataToController();
    size_t neighbor_count = cellGroup.neighbors_to_share_with.size();
    vector<vector<float>> buffers(neighbor_count);
    vector<MPI_Request> requests(neighbor_count);
    vector<MPI_Status> status(neighbor_count);
    vector<int> counts(neighbor_count);

    for(auto& buffer : buffers) {
        buffer.resize(1000000);
    }
    tickNumber = waitForTick();
    double prevTime = MPI_Wtime();
    double newtime;
    while(tickNumber != -1) {
        if(neighbor_count > 0)
        {
            auto dataToSend = createDataVectors();
            std::cout << cellGroup.id << "Data to send size: " << dataToSend.size() << std::endl;
            sendDataToNeighbors(dataToSend, requests);
            std::cout << cellGroup.id << "Sent data to neighbors" << std::endl;
        }
        auto edgeCellsStartAt = calculateInnerCells();
        std::cout<<cellGroup.id<<"Calculated inner cells. Edge cells start at: "<<edgeCellsStartAt<<std::endl;
        if(neighbor_count>0)
        {
            //std::cout<<cellGroup.id<<"waiting"<<std::endl;
            //MPI_Waitall(requests.size(), &requests[0], &status[0]);
            std::cout<<cellGroup.id<<"receiving"<<std::endl;
            receiveDataFromNeighbors(buffers, counts, requests, status);
            std::cout<<cellGroup.id<<"Parsing data from neighbors"<<std::endl;
            for(int i=0;i<buffers.size();i++)
            {
                parseData(buffers[i], counts[i]);
            }
        }
        std::cout<<cellGroup.id<<"Calculated outer cells"<<std::endl;
        calculateEdgeAndOuterCells(edgeCellsStartAt);
        std::cout<<cellGroup.id<<"Integrating"<<std::endl;
        integrate(timeStep);
        std::cout<<cellGroup.id<<"Generating output data"<<std::endl;
        generateOutputData();
        std::cout<<cellGroup.id<<"Sending data"<<std::endl;
        sendDataToController();
        std::cout<<cellGroup.id<<"Cleaning up"<<std::endl;
        clearOuterCells();
        tickNumber = waitForTick();
        std::cout<<cellGroup.id<<"Received tick: "<<tickNumber<<std::endl;

    }
}

vector<pair<int, vector<float>>> Client::createDataVectors()
{
    vector<pair<int, vector<float>>> dataToSend(cellGroup.neighbors_to_share_with.size());
    int it = 0;
    for(auto& neighborCell : cellGroup.neighbors_to_share_with)
    {
        dataToSend[it].first = neighborCell.first;
        dataToSend[it].second.push_back(neighborCell.second.size());
        for(auto& c : neighborCell.second) {
            Cell& cell = cellGroup.cells[c];
            dataToSend[it].second.push_back(c);
            dataToSend[it].second.push_back(cell.particles.size());
            for(auto& p: cell.particles)
            {
                dataToSend[it].second.push_back(p.id);
                dataToSend[it].second.push_back(p.r.x);
                dataToSend[it].second.push_back(p.r.y);
                dataToSend[it].second.push_back(p.r.z);
                dataToSend[it].second.push_back(p.v.x);
                dataToSend[it].second.push_back(p.v.y);
                dataToSend[it].second.push_back(p.v.z);
                dataToSend[it].second.push_back(p.T);
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
    int cellCount = (int) buffer[it++];
    for(int cellNum =0; cellNum <cellCount; cellNum++)
    {
        CellId cellId = (int) buffer[it++];
        int particleCount = (int) buffer[it++];
        for(int particleNum =0; particleNum <particleCount; particleNum++)
        {
            Particle p(buffer[it], glm::vec3(buffer[it+1], buffer[it+2], buffer[it+3]), glm::vec3(buffer[it+4], buffer[it+5], buffer[it+6]), buffer[it+7]);
            it+=8;
            cellGroup.cells[cellId].particles.push_back(std::move(p));
        }
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
    MPI_Recv(&tick, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
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

void Client::calculateEdgeAndOuterCells(int edgeCellsStartAt) {
    for(int i=edgeCellsStartAt; i < cellGroup.innerCells.size(); i++){
        calculations.calculate(cellGroup.innerCells[i].first, cellGroup);
    }
    for(int i=0;i<cellGroup.outerCells.size();i++) {
        if(cellGroup.outerCells[i].second.size() > 0 && cellGroup.outerCells[i].second[0].second > -1)
            break;
        calculations.calculate(cellGroup.outerCells[i].first, cellGroup);
    }
}

void Client::integrate(float deltaTime) {
    outputData.clear();
    outputData.reserve(cellGroup.getParticleCount());
    int i=0;
    for(auto& cell : cellGroup.cells)
    {   i++;
        for(auto particleIt = cell.second.particles.begin(); particleIt != cell.second.particles.end();)
        {
            auto& particle = *particleIt;
            calculations.integrate(particle, deltaTime);
            int c_x = (int) (particle.r.x / cellGroup.h);
            int c_y = (int) (particle.r.y / cellGroup.h);
            int c_z = (int) (particle.r.z / cellGroup.h);
            int cid = c_x + c_y * cellGroup.h_x + c_z * cellGroup.h_x * cellGroup.h_y;
            //std::cout<<fmt::format("{4} {0} {1} {2} {3}", c_x, c_y, c_z, cid, i)<<std::endl;
            if(cid != cell.first) {
                //std::cout<<"dupa"<<std::endl;
                auto tempIt = particleIt;
                particleIt++;
                cell.second.particles.erase(tempIt);
                //std::cout<<"cipa"<<std::endl;
                cellGroup.cells[cid].particles.push_back(std::move(particle));
                //std::cout<<"kupa"<<std::endl;
            }
            else{
                particleIt++;
            }
        }
    }
}

void Client::sendDataToNeighbors(DataToSend& dataToSend, vector<MPI_Request>& requests) {

    for(int i=0;i<dataToSend.size();i++)
    {
        auto& data = dataToSend[i];
        //std::cout<<cellGroup.id<<"Sending "<<data.second.size()<<" data to "<<data.first<<std::endl;
        MPI_Isend(&data.second[0], data.second.size(), MPI_FLOAT, data.first, 0, MPI_COMM_WORLD, &requests[i]);
    }
}

void Client::receiveDataFromNeighbors(vecvecfloat& buffers, vector<int>& counts, vector<MPI_Request>& requests, vector<MPI_Status>& status) {
    int it=0;
    for(auto& neighbor : cellGroup.neighbors_to_share_with)
    {
        //std::cout<<"receiving from "<<neighbor.first<<std::endl;
        MPI_Irecv(&buffers[it][0], 1000000, MPI_FLOAT, neighbor.first, 0, MPI_COMM_WORLD, &requests[it]);
    }

    //std::cout<<"waiting "<<std::endl;
    MPI_Waitall(requests.size(), &requests[0], &status[0]);
    for(int i=0;i<counts.size();i++)
    {
        MPI_Get_count(&status[i], MPI_FLOAT, &counts[i]);
    }
}
