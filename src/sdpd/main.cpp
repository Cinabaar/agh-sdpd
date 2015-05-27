#include <iostream>
#include <mpi.h>
#include <fstream>
#include "format.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "Controller.h"
#include "glm/glm.hpp"
#include "CellGroup.h"
#include "Client.h"
#include <random>
#include "SDPDCalculations.h"

using namespace rapidjson;
using std::cout;
using std::endl;

/*int main()
{
    ParticleWienerRng rng;
    std::mt19937 mt;
    std::ofstream files[4];
    files[0].open("wiener0.data");
    files[1].open("wiener1.data");
    files[2].open("wiener2.data");
    files[3].open("wiener3.data");

    glm::mat3 W(0);
    float V = 0;
    for(int i=0;i<300;i++)
    {
        auto inc = rng.getWienerIncrements(mt(), i, 0, 1, 0.033);
        W+=inc.first;
        V+=inc.second;
        files[0]<<i<<" "<<W[0][0]<<endl;
        files[1]<<i<<" "<<W[1][1]<<endl;
        files[2]<<i<<" "<<W[2][2]<<endl;
        files[3]<<i<<" "<<V<<endl;
    }
    return 0;
}*/

int main(int argc, char **argv) {
    int my_rank;
    int numprocs;
    MPI_Init (&argc, &argv);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank (MPI_COMM_WORLD, &my_rank);

    if(my_rank==0)
    {
        Document config;
        std::ifstream in("config.json");
        std::string contents((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());

        const char* conf = contents.c_str();
        config.Parse(conf);
        Value val;
        int count = config["count"].GetInt();
        std::string dist = config["distribution"].GetString();
        float M = config["M"].GetDouble();
        float N = config["N"].GetDouble();
        float kB = config["kB"].GetDouble();
        float K = config["K"].GetDouble();
        float n = config["n"].GetDouble();
        float z = config["z"].GetDouble();
        float h = config["h"].GetDouble();
        float T0 = config["T0"].GetDouble();
        float totalTime = config["totalTime"].GetDouble();
        float timeStep = config["timeStep"].GetDouble();
        glm::vec3 ldf;
        glm::vec3 rub;
        val = config["ldf"];
        for(int i=0; i<val.Size(); i++)
        {
            ldf[i]= (float) val[i].GetDouble();
        }
        val = config["rub"];
        for(int i=0; i<val.Size(); i++)
        {
            rub[i]= (float) val[i].GetDouble();
        }
        Controller controller(count, kB, K, n, z, N, M, h, T0, totalTime, timeStep, ldf, rub, numprocs-1);
        if(!controller.initialize())
        {
            MPI_Finalize ();
            std::cout<<"Unable to initialize. Need more than 1 process."<<std::endl;
            return 0;
        }
        controller.run();
    }
    else
    {
        std::vector<float> data(10000000);
        MPI_Status status;
        MPI_Recv(&data[0], data.size(), MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        int length;
        MPI_Get_count(&status, MPI_INT, &length);
        data.reserve(length);
        
        float m(data[0]), M(data[1]), N(data[2]), kB(data[3]), K(data[4]), n(data[5]), z(data[6]); 
        glm::vec3 lbf(data[7], data[8], data[9]);
        glm::vec3 rub(data[10], data[11], data[12]);
        float h = data[13];
        float totalTime = data[14];
        float timeStep = data[15];
        CellGroup mainGroup(data[20], data[16], data[17], data[18], h, lbf, rub);
        int iter = 19;
        const auto temp_iter = iter;
        iter++;
        //std::cout<<mainGroup<<std::endl;
        for(int group=0; group<data[temp_iter]; group++) {
            CellGroup cellGroup(data[iter], data[16], data[17], data[18], h, lbf, rub);
            iter++;
            const auto temp_iter = iter;
            iter++;
            for (int c = 0; c < data[temp_iter]; c++) {
                Cell cell(data[iter], cellGroup.id);
                int c_x = cell.id % cellGroup.h_x;
                int c_y = (cell.id % (cellGroup.h_x * cellGroup.h_y)) / cellGroup.h_x;
                int c_z = (cell.id / (cellGroup.h_x * cellGroup.h_y));
                cell.h = h;
                cell.x = c_x;
                cell.y = c_y;
                cell.z = c_z;
                iter++;
                const auto temp_iter = iter;
                iter++;
                for (int p = 0; p < data[temp_iter]; p++) {
                    Particle part(data[iter], glm::vec3(data[iter + 1], data[iter + 2], data[iter + 3]), glm::vec3(data[iter + 4],
                                  data[iter + 5], data[iter + 6]), data[iter + 7]);
                    iter = iter + 8;
                    cell.particles.push_back(std::move(part));
                }
                cellGroup.cells[cell.id] = std::move(cell);
            }
            if (cellGroup.id == mainGroup.id) {
                mainGroup = std::move(cellGroup);
            }
            else {
                mainGroup.initializeNeighborGroup(cellGroup);
            }
        }
        for(auto& c : mainGroup.cells) {
            mainGroup.initializeCellNeighbors(c.second);
        }
        mainGroup.sortCellsByDistanceToNeighbor();
        SDPDCalculations calculations(lbf, rub, N, n, m, z, K, kB, h, totalTime, timeStep);
        Client client(mainGroup, lbf, rub, move(calculations));
        client.run(totalTime, timeStep);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize ();
    return 0;
}

