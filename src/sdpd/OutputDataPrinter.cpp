//
// Created by dawid on 19/05/15.
//
#include "format.h"
#include <fstream>
#include <iostream>
#include "OutputDataPrinter.h"
using std::max;
using std::min;
void OutputDataPrinter::printData(vector<glm::vec4>& data, std::string fileName) {
    std::ofstream file;
    file.open(fileName);
    array<int, 3> color;
    array<int, 3> rgb1;
    array<int, 3> rgb2;
    float ratio;
    for(auto& d : data)
    {
        if(d.w <= cold) {
            color = cold_rgb;
        } else if(cold < d.w && d.w <= normal) {
            rgb1 = cold_rgb;
            rgb2 = normal_rgb;
            ratio = (d.w - cold) / (normal - cold);
            color = interpolateRGB(rgb1, rgb2, ratio);
        }
        else if(normal < d.w && d.w <= hot) {
            rgb1 = normal_rgb;
            rgb2 = hot_rgb;
            ratio = (d.w - normal) / (hot - normal);
            color = interpolateRGB(rgb1, rgb2, ratio);
        } else {
            color = hot_rgb;
        }
        file<<fmt::format("{0} {1} {2} {3}", d.x, d.y, d.z, 0x00000000 | (color[0] << 16) | (color[1] << 8) | (color[2] << 0))<<std::endl;
    }

}

auto OutputDataPrinter::interpolateRGB(array<int, 3> rgb1, array<int, 3> rgb2, float ratio) -> array<int, 3> {
    auto r = rgb1[0]*(1-ratio) + rgb2[0]*ratio;
    auto g = rgb1[1]*(1-ratio) + rgb2[1]*ratio;
    auto b = rgb1[2]*(1-ratio) + rgb2[2]*ratio;
    return {{(int)r, (int)g, (int)b}};
}
