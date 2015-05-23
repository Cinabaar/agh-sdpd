//
// Created by dawid on 19/05/15.
//
#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <array>

using std::vector;
using std::array;
class OutputDataPrinter {
private:
    int cold{0}, normal{20}, hot{40};
    array<int, 3> cold_rgb = {{0, 0, 255}};
    array<int, 3> normal_rgb = {{255, 255, 255}};
    array<int, 3> hot_rgb = {{255, 0, 0}};
public:
    void printData(vector<glm::vec4>& data, std::string fileName);

    array<int, 3> interpolateRGB(array<int, 3> rgb1, array<int, 3> rgb2, float ratio);
};