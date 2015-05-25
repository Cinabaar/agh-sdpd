//
// Created by dawid on 17/05/15.
//

#include "CellGroup.h"

int CellGroup::xyzToCellId( float x, float y, float z )
{
    int c_x, c_y, c_z;
    c_x = x / h;
    c_y = y / h;
    c_z = z / h;
    if(x == rub.x) c_x--;
    if(y == rub.y) c_y--;
    if(z == rub.z) c_z--; 
    int cid = c_x + c_y * h_x + c_z * h_x * h_y;    
    return cid;
}

void CellGroup::initializeCellNeighbors(Cell &c)
{
    array<int, 27> neighbors;
    for(int i=-1;i<2;i++)
    {
        for(int j=-1;j<2;j++)
        {
            for(int k=-1;k<2;k++)
            {
                neighbors[(k+1) + (j+1)*3 + (i+1)*9] = (c.x+k) + (c.y+j)*h_x + (c.z+i)*h_x*h_y;
            }
        }
    }
    for(int i=0;i<27;i++)
    {
        if(neighbors[i] < 0 || neighbors[i] >= h_x*h_y*h_z || neighbors[i]==c.id) {
            neighbors[i] = -1;
            continue;
        }
        int c_x = c.id % h_x;
        int c_y = (c.id % (h_x*h_y)) / h_x;
        int c_z = (c.id / (h_x * h_y));

        int n_x = neighbors[i] % h_x;
        int n_y = (neighbors[i] % (h_x*h_y)) / h_x;
        int n_z = (neighbors[i] / (h_x * h_y));

        if(abs(c_x-n_x)>1 || abs(c_y-n_y)>1 || abs(c_z-n_z)>1)
            neighbors[i] = -1;
    }
    for(int i=0;i<27;i++)
    {
        if(neighbors[i] == -1)
            c.neighbors[i] = nullptr;
        else
            c.neighbors[i] = &cells[neighbors[i]];
    }
    if(std::find_if(innerCells.begin(), innerCells.end(), [&](const pair<CellId, vector<pair<CellGroupId, Distance >>> el) {return el.first == c.id;}) == innerCells.end())
    {
        innerCells.push_back(std::make_pair(c.id, vector<pair<CellGroupId, Distance>>()));
    }
    
}

void CellGroup::initializeNeighborGroup(CellGroup &neighbor_cell_group) {
    for(auto& c : cells)
    {
        if(std::find_if(outerCells.begin(), outerCells.end(), [&](const pair<CellId, vector<pair<CellGroupId, Distance >>> el) {return el.first == c.first;}) != outerCells.end())
        {
            continue;
        }
        int min_distance = INT_MAX;
        for(auto& nc : neighbor_cell_group.cells)
        {
            int distance = std::max(std::max(std::abs(nc.second.x - c.second.x), std::abs(nc.second.y - c.second.y)), std::abs(nc.second.z - c.second.z));
            min_distance = std::min(min_distance, distance);
        }
        auto it = std::find_if(innerCells.begin(), innerCells.end(), [&](const pair<CellId, vector<pair<CellGroupId, Distance >>> el) {return el.first == c.first;});
        if(it == innerCells.end())
        {
            innerCells.push_back(std::make_pair(c.first, vector<pair<CellGroupId, Distance>>()));
            innerCells.back().second.push_back(std::make_pair(neighbor_cell_group.id, min_distance));
        }
        else
        {
            it->second.push_back(std::make_pair(neighbor_cell_group.id, min_distance));
        }
        if(min_distance <=1 )
        {
            neighbors_to_share_with[neighbor_cell_group.id].push_back(c.first);
        }
    }

    for(auto& innerCell : innerCells) {
        if(innerCell.second.size()>1) {
            if(innerCell.second[0].second > innerCell.second[1].second)
            {
                std::swap(innerCell.second[0], innerCell.second[1]);
            }
        }
    }
    for (auto &nc : neighbor_cell_group.cells)
    {
        int min_distance = INT_MAX;
        for(auto& c : cells)
        {
            int distance = std::max(std::max(std::abs(nc.second.x - c.second.x), std::abs(nc.second.y - c.second.y)), std::abs(nc.second.z - c.second.z));
            min_distance = std::min(min_distance, distance);
        }
        auto it = std::find_if(outerCells.begin(), outerCells.end(), [&](const pair<CellId, vector<pair<CellGroupId, Distance >>> el) {return el.first == nc.first;});

        if(min_distance <= 1)
        {
            if(it == outerCells.end())
            {
                outerCells.push_back(std::make_pair(nc.first, vector<pair<CellGroupId, Distance>>()));
                outerCells.back().second.push_back(std::make_pair(this->id, -min_distance));
            }
            else
            {
                it->second.push_back(std::make_pair(this->id, -min_distance));
            }
        }
    }
    for(auto& nc : neighbor_cell_group.cells)
    {
        if(std::find_if(outerCells.begin(), outerCells.end(), [&](const pair<CellId, vector<pair<CellGroupId, Distance >>>& el) {return el.first == nc.first;}) != outerCells.end())
        {
            cells[nc.first] = std::move(nc.second);
        }
    }
}

void CellGroup::sortCellsByDistanceToNeighbor() {

    std::sort(innerCells.begin(), innerCells.end(),
              [&](const pair<CellId, vector<pair<CellGroupId, Distance >>>& l, const pair<CellId, vector<pair<CellGroupId, Distance >>>& r)
              {
                  if(l.second.size() > 0 && r.second.size() > 0)
                  return l.second[0].second > r.second[0].second;
              });

    std::sort(outerCells.begin(), outerCells.end(),
              [&](const pair<CellId, vector<pair<CellGroupId, Distance >>>& l, const pair<CellId, vector<pair<CellGroupId, Distance >>>& r)
              {
                  if(l.second.size() > 0 && r.second.size() > 0)
                  return l.second[0].second > r.second[0].second;
              });

}

int CellGroup::getParticleCount() {
    int particles = 0;
    for(auto& i : innerCells) {
        particles += cells[i.first].particles.size();
    }
}
