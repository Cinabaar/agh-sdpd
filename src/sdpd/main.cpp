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

using namespace rapidjson;

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
//        std::ofstream file;
//        file.open("output.txt");
//        file << "CONSTANTS" << std::endl;
//        file << fmt::format("count (particle count) = {0}", count) << std::endl;
//        file << fmt::format("kB (Boltzmann constant) = {0}", kB) << std::endl;
//        file << fmt::format("K (thermal conductivity) = {0}", K) << std::endl;
//        file << fmt::format("n (shear viscocity) = {0}", n) << std::endl;
//        file << fmt::format("z (bulk viscocity) = {0}", z) << std::endl;
//        file << fmt::format("N (molecules per particle) = {0}", N) << std::endl;
//        file << fmt::format("M (total fluid mass) = {0}", M) << std::endl;
//        file << fmt::format("m (single particle mass) = {0}", M/count) << std::endl;
//        file << fmt::format("h (radius) = {0}", h) << std::endl;
//        file << fmt::format("ldf = ({0}, {1}, {2})", ldf[0], ldf[1], ldf[2]) << std::endl;
//        file << fmt::format("rub = ({0}, {1}, {2})", rub[0], rub[1], rub[2]) << std::endl;

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
        //std::cout<<my_rank<<" "<<length<<" "<<MPI_Wtime() - start<<std::endl;


        glm::vec3 lbf(data[0], data[1], data[2]);
        glm::vec3 rub(data[3], data[4], data[5]);
        float h = data[6];
        float totalTime = data[7];
        float timeStep = data[8];
        CellGroup mainGroup(data[13], data[9], data[10], data[11], h, lbf, rub);
        int iter = 12;
        const auto temp_iter = iter;
        iter++;
        //std::cout<<mainGroup<<std::endl;
        for(int group=0; group<data[temp_iter]; group++) {
            CellGroup cellGroup(data[iter], data[9], data[10], data[11], h, lbf, rub);
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
        Client client(mainGroup, lbf, rub);
        client.run(totalTime, timeStep);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize ();
    return 0;
}
