//
// Created by dawid on 19/05/15.
//
#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <array>

using std::vector;
using std::array;
using std::pair;
using std::string;
class OutputDataPrinter {
private:
    int cold{185}, normal{224}, hot{263};
    array<int, 3> cold_rgb = {{0, 0, 255}};
    array<int, 3> normal_rgb = {{255, 255, 255}};
    array<int, 3> hot_rgb = {{255, 0, 0}};
    vector<pair<string, vector<glm::vec4>>> data;
public:
    void printData();
    void printData(int num);
    void addData(vector<glm::vec4>& data, std::string fileName);

    array<int, 3> interpolateRGB(array<int, 3> rgb1, array<int, 3> rgb2, float ratio);
};
