//
// Created by dawid on 17/05/15.
//
#pragma once
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Particle.h"
#include "Cell.h"
#include "Controller.h"

using CellId = int;
using CellGroupId = int;
using Distance = int;
using std::vector;
using std::map;
using std::pair;
using CellDistancesToNeighborGroups =  vector<pair<CellId, vector<pair<CellGroupId, Distance >>>>;

class CellGroup {
friend class Controller;
public:
    CellGroupId id;

    int h_x, h_y, h_z;
    float h;

    map<CellId, Cell> cells;
    map<CellGroupId, vector<CellId>> neighbors_to_share_with;

    CellDistancesToNeighborGroups innerCells; //(Cell, Distance to all neighbors)
    CellDistancesToNeighborGroups outerCells;
public:
    CellGroup() = default;
    CellGroup(int id, int h_x, int h_y, int h_z, float h) : id(id), h_x(h_x), h_y(h_y), h_z(h_z), h(h) {};
    CellGroup(const CellGroup&) = delete;
    CellGroup& operator=(CellGroup const&) = delete;
    CellGroup(CellGroup&&) = default;
    CellGroup& operator=(CellGroup&&) = default;

    void initializeCellNeighbors(Cell &c);
    const int totalParticles() const
    {
        int sum=0;
        for(auto& c : cells)
        {
            sum+=c.second.particles.size();
        }
        return sum;
    }
    /*friend std::ostream& operator<< (std::ostream& stream, CellGroup& cellGroup) {
        stream<<fmt::format("Cell group id: {0}, cells: {1}, total particles {2})", cellGroup.id, cellGroup.cells.size(), cellGroup.totalParticles())<<std::endl;
        for(auto& innerCell : cellGroup.innerCells){
            stream<<fmt::format("id: {0} ", innerCell.first)<<std::endl;
            for(auto& n : innerCell.second) {
                stream<<fmt::format("group: {0}, distance: {1}", n.first, n.second)<<std::endl;
            }
        }
        std::cout<<"==========="<<std::endl;
        for(auto& outerCell : cellGroup.outerCells){
            stream<<fmt::format("id: {0} ", outerCell.first)<<std::endl;
            for(auto& n : outerCell.second) {
                stream<<fmt::format("group: {0}, distance: {1}", n.first, n.second)<<std::endl;
            }
        }
        for(auto& c : cellGroup.innerCells) {
            std::cout<<cellGroup.cells[c.first]<<std::endl;
        }
        return stream;
    }*/

    void initializeNeighborGroup(CellGroup &neighbor_cell_group);
    void sortCellsByDistanceToNeighbor();
    int getParticleCount();
};
