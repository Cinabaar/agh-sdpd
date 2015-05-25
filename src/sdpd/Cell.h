//
// Created by dawid on 17/05/15.
//
#pragma once

#include <array>
#include <vector>
#include <list>
#include "Particle.h"
#include "format.h"

using CellId = int;
using std::array;
using std::vector;
using std::list;
class Cell {

public:
    int id, x, y, z;
    float h;
    int cellGroupId;
    array<Cell*, 27> neighbors;
    std::list<Particle> particles;

    Cell() = default;
    Cell(int id, int cellGroupId) : id(id), cellGroupId(cellGroupId) {}

    Cell(const Cell&) = delete;
    Cell& operator=(Cell const&) = delete;
    Cell(Cell&&) = default;
    Cell& operator=(Cell&&) = default;

    /*friend std::ostream& operator<< (std::ostream& stream, Cell& cell) {
        stream<<fmt::format("Cell id: {0}, ({1}, {2}, {3}), particles: {4}", cell.id, cell.x, cell.y, cell.z, cell.particles.size())<<std::cout;
        for(auto& p : cell.particles) {
            stream<<p<<std::endl;
        }
        return stream;
    }*/




};

